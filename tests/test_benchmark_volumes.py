"""Tests for the volumetric benchmark and its reference formats.

The volumetric contenders only mean something if the bytes they emit are real
NIfTI/NRRD/MetaImage/TIFF files, so these tests check both the structure a
reader keys on (magic, header fields, IFD chaining) and pixel-exact round
trips, including the cases that are easy to get wrong: a single-slice volume,
a full 256-entry ColorMap, and a stack whose pages must all point at one
shared palette.
"""

import os
import random
import struct
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import benchmark  # noqa: E402
import benchmark_volumes as bv  # noqa: E402
import refvolumes as rv  # noqa: E402


def make_volume(width, height, depth, colors, seed=5):
    rng = random.Random(seed)
    palette = [bytes(rng.randrange(256) for _ in range(3)) for _ in range(colors)]
    return [
        b"".join(rng.choice(palette) for _ in range(width * height))
        for _ in range(depth)
    ]


VOLUMES = [(16, 12, 4, 3), (32, 32, 8, 64), (8, 6, 1, 2), (17, 5, 3, 900)]


# --------------------------------------------------------------------------
# NIfTI
# --------------------------------------------------------------------------


@pytest.mark.parametrize("width,height,depth,colors", VOLUMES)
def test_nifti_round_trip(width, height, depth, colors):
    slices = make_volume(width, height, depth, colors)
    data = rv.encode_nifti(width, height, slices)
    assert data[344:348] == b"n+1\x00"
    assert struct.unpack_from("<i", data, 0)[0] == 348
    assert rv.decode_nifti(data) == (width, height, slices)


def test_nifti_header_describes_the_grid():
    slices = make_volume(9, 7, 5, 4)
    data = rv.encode_nifti(9, 7, slices)
    dims = struct.unpack_from("<8h", data, 40)
    datatype, bitpix = struct.unpack_from("<2h", data, 70)
    assert dims[0] == 3 and dims[1:4] == (9, 7, 5)
    assert (datatype, bitpix) == (128, 24)  # DT_RGB24
    assert struct.unpack_from("<f", data, 108)[0] == 352.0  # vox_offset


def test_nifti_gz_round_trip_and_shrinks():
    slices = make_volume(32, 32, 8, 4)
    plain = rv.encode_nifti(32, 32, slices)
    packed = rv.encode_nifti_gz(32, 32, slices)
    assert packed[:2] == b"\x1f\x8b"
    assert len(packed) < len(plain)
    assert rv.decode_nifti_gz(packed) == (32, 32, slices)


def test_nifti_decoder_rejects_foreign_data():
    with pytest.raises(ValueError):
        rv.decode_nifti(b"\x00" * 400)


# --------------------------------------------------------------------------
# NRRD
# --------------------------------------------------------------------------


@pytest.mark.parametrize("encoding", ["raw", "gzip"])
@pytest.mark.parametrize("width,height,depth,colors", VOLUMES)
def test_nrrd_round_trip(encoding, width, height, depth, colors):
    slices = make_volume(width, height, depth, colors)
    data = rv.encode_nrrd(width, height, slices, encoding)
    assert data.startswith(b"NRRD0004\n")
    assert rv.decode_nrrd(data) == (width, height, slices)


def test_nrrd_header_declares_an_rgb_axis():
    slices = make_volume(6, 4, 3, 2)
    header = rv.encode_nrrd(6, 4, slices).split(b"\n\n")[0].decode()
    assert "sizes: 3 6 4 3" in header
    assert "kinds: RGB-color space space space" in header
    assert "dimension: 4" in header


def test_nrrd_rejects_unknown_encoding():
    with pytest.raises(ValueError):
        rv.encode_nrrd(4, 4, make_volume(4, 4, 2, 2), "ascii")


# --------------------------------------------------------------------------
# MetaImage
# --------------------------------------------------------------------------


@pytest.mark.parametrize("compress", [True, False])
@pytest.mark.parametrize("width,height,depth,colors", VOLUMES)
def test_metaimage_round_trip(compress, width, height, depth, colors):
    slices = make_volume(width, height, depth, colors)
    data = rv.encode_metaimage(width, height, slices, compress)
    assert data.startswith(b"ObjectType = Image")
    assert rv.decode_metaimage(data) == (width, height, slices)


def test_metaimage_records_its_compressed_size():
    slices = make_volume(16, 16, 4, 8)
    data = rv.encode_metaimage(16, 16, slices, True)
    header, _, _ = data.partition(b"ElementDataFile = LOCAL\n")
    fields = dict(
        line.split(" = ", 1) for line in header.decode().splitlines() if " = " in line
    )
    assert fields["CompressedData"] == "True"
    assert fields["ElementNumberOfChannels"] == "3"
    assert fields["DimSize"] == "16 16 4"
    assert int(fields["CompressedDataSize"]) == len(data) - len(header) - len(
        b"ElementDataFile = LOCAL\n"
    )


# --------------------------------------------------------------------------
# TIFF stacks
# --------------------------------------------------------------------------


@pytest.mark.parametrize("compress", [True, False])
@pytest.mark.parametrize("width,height,depth,colors", VOLUMES)
def test_tiff_rgb_stack_round_trip(compress, width, height, depth, colors):
    slices = make_volume(width, height, depth, colors)
    data = rv.encode_tiff_stack(width, height, slices, compress)
    assert data[:4] == b"II\x2a\x00"
    assert rv.decode_tiff_stack(data) == (width, height, slices)


@pytest.mark.parametrize("compress", [True, False])
@pytest.mark.parametrize("colors", [1, 2, 16, 256])
def test_tiff_palette_stack_round_trip(compress, colors):
    slices = make_volume(24, 20, 5, colors)
    data = rv.encode_tiff_stack_palette(24, 20, slices, compress)
    assert rv.decode_tiff_stack(data) == (24, 20, slices)


def test_tiff_palette_stack_refuses_more_than_256_colors():
    with pytest.raises(ValueError):
        rv.encode_tiff_stack_palette(32, 32, make_volume(32, 32, 4, 400))


def test_tiff_pages_share_one_color_map():
    """The palette is stored once for the stack, not copied into every page."""

    slices = make_volume(16, 16, 12, 16)
    data = rv.encode_tiff_stack_palette(16, 16, slices, True)

    offsets = []
    (first,) = struct.unpack_from("<I", data, 4)
    ifd = first
    while ifd:
        (count,) = struct.unpack_from("<H", data, ifd)
        for i in range(count):
            entry = ifd + 2 + 12 * i
            tag, _kind, _n = struct.unpack_from("<HHI", data, entry)
            if tag == rv._TIFF_TAGS["ColorMap"]:
                offsets.append(struct.unpack_from("<I", data, entry + 8)[0])
        (ifd,) = struct.unpack_from("<I", data, ifd + 2 + 12 * count)

    assert len(offsets) == 12, "every page should carry a ColorMap tag"
    assert len(set(offsets)) == 1, "pages must point at the same ColorMap block"
    # One 768-entry map (1536 bytes), not twelve.
    assert len(data) < sum(len(s) for s in slices) and len(data) < 12 * 1536


def test_tiff_decoder_rejects_big_endian():
    with pytest.raises(ValueError):
        rv.decode_tiff_stack(b"MM\x00\x2a" + b"\x00" * 16)


# --------------------------------------------------------------------------
# corpus and harness
# --------------------------------------------------------------------------


def test_volume_corpus_is_deterministic_and_depth_axis():
    for name, builder in bv.CORPUS_BUILDERS.items():
        first, second = builder(), builder()
        assert first.frames == second.frames, f"{name} is not reproducible"
        assert first.axis == "depth", f"{name} must be a volume, not a video"
        assert first.kind == "volume"
        assert first.unit == "slices"
        assert len(first.frames) > 1


def test_volume_corpus_spans_palette_sizes():
    clips = {name: builder() for name, builder in bv.CORPUS_BUILDERS.items()}
    assert clips["labels"].colors <= 8
    assert clips["ct"].colors >= 128
    assert clips["voxel"].raw_bytes == 64 * 64 * 3 * 20


def test_clip_kind_distinguishes_volume_from_video():
    frames = make_volume(8, 8, 3, 4)
    assert benchmark.Clip("v", 8, 8, frames).kind == "video"
    assert benchmark.Clip("v", 8, 8, frames, axis="depth").kind == "volume"
    assert benchmark.Clip("v", 8, 8, frames[:1], axis="depth").kind == "image"


def test_load_inputs_reads_a_slice_directory(tmp_path):
    from refcodecs import write_ppm

    slices = make_volume(8, 6, 4, 3)
    stack = tmp_path / "stack"
    stack.mkdir()
    for i, data in enumerate(slices):
        (stack / f"s{i:03d}.ppm").write_bytes(write_ppm(8, 6, data))

    clips = bv.load_inputs([str(stack)])
    assert len(clips) == 1
    assert clips[0].kind == "volume" and clips[0].frames == slices


def test_measure_volume_reference_covers_every_container():
    clip = bv.CORPUS_BUILDERS["labels"]()
    results = bv.measure_volume_reference(clip, verify=True)
    formats = {r.format for r in results}
    assert {"NIfTI", "NRRD", "MetaImage", "TIFF stack", "PNG", "PNG8", "GIF", "QOI"} <= formats
    assert all(r.lossless for r in results), "a volumetric codec failed to round-trip"

    variants = {(r.format, r.detail) for r in results}
    assert ("NIfTI", "RGB24, .nii.gz") in variants
    assert ("TIFF stack", "palette + Deflate") in variants

    plain = next(r for r in results if r.detail == "RGB24, uncompressed")
    assert plain.size == clip.raw_bytes + 352  # header, then the voxels


def test_measure_volume_reference_marks_palette_formats_unavailable():
    clip = benchmark.Clip(
        "many", 32, 32, make_volume(32, 32, 4, 400), "too many colors", axis="depth"
    )
    results = bv.measure_volume_reference(clip, verify=True)
    unavailable = {r.format for r in results if r.size is None}
    assert {"TIFF stack", "PNG8", "GIF"} <= unavailable
    assert all(r.size is not None for r in results if r.format == "NIfTI")


def test_unknown_volume_name_is_rejected():
    with pytest.raises(SystemExit):
        bv.main(["--corpus", "not-a-volume"])


# --------------------------------------------------------------------------
# end-to-end against the built CLI
# --------------------------------------------------------------------------


SPLAT_BINARY = os.environ.get("SPLAT_BINARY", str(ROOT / "4splat"))
needs_binary = pytest.mark.skipif(
    not Path(SPLAT_BINARY).exists(),
    reason=f"{SPLAT_BINARY} not built (run 'make plain')",
)


@needs_binary
def test_splat_encodes_a_volume_not_a_video(tmp_path):
    """A depth-axis clip must go through encode-volume and keep depth > 1."""

    clip = bv.CORPUS_BUILDERS["labels"]()
    runner = benchmark.SplatRunner(SPLAT_BINARY, tmp_path)
    out = tmp_path / "labels.4spl"
    _ms, proc = runner.encode(clip, "rle", None, out)
    assert proc.returncode == 0, proc.stdout.decode()

    header = out.read_bytes()[:32]
    width, height, depth, frames = struct.unpack_from("<4I", header, 8)
    assert (width, height) == (clip.width, clip.height)
    assert depth == len(clip.frames) and frames == 1
    assert runner.decode(clip, out) == clip.frames


@needs_binary
def test_end_to_end_volume_benchmark(tmp_path, capsys):
    csv_path = tmp_path / "volumes.csv"
    exit_code = bv.main(
        [
            "--binary", SPLAT_BINARY,
            "--corpus", "labels",
            "--schemes", "none,rle",
            "--csv", str(csv_path),
        ]
    )
    report = capsys.readouterr().out
    assert exit_code == 0, "a codec under test failed verification"
    assert "### `labels`" in report and "24 slices" in report
    assert "| NIfTI |" in report and "| 4splat |" in report
    rows = csv_path.read_text().splitlines()
    assert any(row.startswith("labels,NIfTI") for row in rows)
    assert any(row.startswith("labels,4splat") for row in rows)
