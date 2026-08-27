"""Tests for the benchmark harness and its reference codecs.

The reference encoders in ``tools/refcodecs.py`` only make the benchmark
meaningful if the files they emit are real PNG/GIF/QOI files, so these tests
check both structure (magic numbers, chunk framing) and pixel-exact round
trips, including the awkward paths: a single-color image, a full 256-color
palette, an LZW string table that overflows and has to be cleared, and QOI's
index/diff/luma/run opcodes.
"""

import json
import os
import random
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import benchmark  # noqa: E402
import refcodecs as rc  # noqa: E402


# --------------------------------------------------------------------------
# helpers
# --------------------------------------------------------------------------


def make_palette(count, seed=1234):
    rng = random.Random(seed)
    return [bytes(rng.randrange(256) for _ in range(3)) for _ in range(count)]


def make_image(width, height, palette, seed=99):
    rng = random.Random(seed)
    return b"".join(rng.choice(palette) for _ in range(width * height))


def gradient(width, height):
    return b"".join(
        bytes(((x * 2) % 256, (y * 3) % 256, (x + y) % 256))
        for y in range(height)
        for x in range(width)
    )


# --------------------------------------------------------------------------
# PPM
# --------------------------------------------------------------------------


def test_ppm_round_trip():
    pixels = gradient(7, 5)
    data = rc.write_ppm(7, 5, pixels)
    assert data.startswith(b"P6\n7 5\n255\n")
    assert rc.read_ppm(data) == (7, 5, pixels)


def test_ppm_accepts_comments_in_header():
    pixels = bytes((1, 2, 3, 4, 5, 6))
    data = b"P6\n# written by a test\n2 1\n255\n" + pixels
    assert rc.read_ppm(data) == (2, 1, pixels)


@pytest.mark.parametrize(
    "data",
    [
        b"P3\n2 1\n255\n\x00\x00\x00\x00\x00\x00",  # ASCII PPM
        b"P6\n2 1\n65535\n" + b"\x00" * 12,  # 16-bit samples
        b"P6\n2 1\n255\n\x00\x00",  # truncated pixels
    ],
)
def test_ppm_rejects_unsupported_input(data):
    with pytest.raises(rc.PpmError):
        rc.read_ppm(data)


def test_write_ppm_rejects_mismatched_buffer():
    with pytest.raises(ValueError):
        rc.write_ppm(4, 4, b"\x00\x00\x00")


# --------------------------------------------------------------------------
# PNG
# --------------------------------------------------------------------------


@pytest.mark.parametrize(
    "width,height,colors",
    [(1, 1, 1), (17, 13, 2), (64, 64, 256), (33, 7, 900)],
)
def test_png_truecolor_round_trip(width, height, colors):
    pixels = make_image(width, height, make_palette(colors))
    data = rc.encode_png(width, height, pixels)
    assert data[:8] == b"\x89PNG\r\n\x1a\n"
    assert rc.decode_png(data) == (width, height, pixels)


def test_png_indexed_round_trip_and_chunk_order():
    pixels = make_image(40, 24, make_palette(16))
    data = rc.encode_png_indexed(40, 24, pixels)
    kinds = [kind for kind, _ in rc._png_chunks(data)]
    assert kinds == [b"IHDR", b"PLTE", b"IDAT", b"IEND"]
    assert rc.decode_png(data) == (40, 24, pixels)


def test_png_indexed_refuses_more_than_256_colors():
    pixels = make_image(32, 32, make_palette(400))
    with pytest.raises(ValueError):
        rc.encode_png_indexed(32, 32, pixels)


def test_png_gradient_exercises_every_filter():
    pixels = gradient(64, 64)
    data = rc.encode_png(64, 64, pixels)
    assert rc.decode_png(data) == (64, 64, pixels)


def test_png_decoder_rejects_corrupted_chunk():
    data = bytearray(rc.encode_png(8, 8, gradient(8, 8)))
    data[-5] ^= 0xFF  # flip a byte inside IEND's CRC coverage
    with pytest.raises(ValueError):
        rc.decode_png(bytes(data))


# --------------------------------------------------------------------------
# GIF
# --------------------------------------------------------------------------


@pytest.mark.parametrize("colors", [1, 2, 16, 256])
def test_gif_round_trip_single_frame(colors):
    pixels = make_image(48, 32, make_palette(colors))
    data = rc.encode_gif(48, 32, [pixels])
    assert data[:6] == b"GIF89a"
    assert data[-1:] == b"\x3b"
    assert rc.decode_gif(data) == (48, 32, [pixels])


def test_gif_round_trip_animated():
    palette = make_palette(64)
    frames = [make_image(32, 24, palette, seed=s) for s in range(5)]
    data = rc.encode_gif(32, 24, frames)
    assert b"NETSCAPE2.0" in data
    assert rc.decode_gif(data) == (32, 24, frames)


def test_gif_lzw_handles_table_overflow():
    """A large, varied image overflows the 12-bit table and forces a clear."""

    pixels = make_image(256, 256, make_palette(200))
    data = rc.encode_gif(256, 256, [pixels])
    assert rc.decode_gif(data)[2] == [pixels]


def test_gif_lzw_handles_long_runs():
    pixels = bytes((7, 8, 9)) * (200 * 200)
    data = rc.encode_gif(200, 200, [pixels])
    assert rc.decode_gif(data)[2] == [pixels]
    assert len(data) < len(pixels) // 10


def test_gif_refuses_more_than_256_colors():
    pixels = make_image(32, 32, make_palette(500))
    with pytest.raises(ValueError):
        rc.encode_gif(32, 32, [pixels])


# --------------------------------------------------------------------------
# QOI
# --------------------------------------------------------------------------


@pytest.mark.parametrize(
    "width,height,pixels",
    [
        (1, 1, bytes((3, 4, 5))),
        (16, 16, bytes((200, 100, 50)) * 256),  # pure runs
        (64, 64, gradient(64, 64)),  # diff / luma
        (40, 40, make_image(40, 40, make_palette(8))),  # index hits
        (25, 9, make_image(25, 9, make_palette(600))),  # literal RGB
    ],
)
def test_qoi_round_trip(width, height, pixels):
    data = rc.encode_qoi(width, height, pixels)
    assert data[:4] == b"qoif"
    assert data[-8:] == b"\x00" * 7 + b"\x01"
    assert rc.decode_qoi(data) == (width, height, pixels)


def test_qoi_run_longer_than_one_opcode():
    """Runs cap at 62 pixels, so a flat image must emit several run opcodes."""

    pixels = bytes((1, 2, 3)) * 500
    data = rc.encode_qoi(500, 1, pixels)
    assert rc.decode_qoi(data) == (500, 1, pixels)


def test_qoi_rejects_foreign_data():
    with pytest.raises(ValueError):
        rc.decode_qoi(b"nope" + b"\x00" * 20)


# --------------------------------------------------------------------------
# palette helpers
# --------------------------------------------------------------------------


def test_build_palette_is_first_appearance_order():
    frame_a = bytes((1, 1, 1, 2, 2, 2))
    frame_b = bytes((2, 2, 2, 3, 3, 3))
    palette, lookup = rc.build_palette([frame_a, frame_b])
    assert palette == [bytes((1, 1, 1)), bytes((2, 2, 2)), bytes((3, 3, 3))]
    assert lookup[bytes((3, 3, 3))] == 2
    assert rc.distinct_colors([frame_a, frame_b]) == 3


# --------------------------------------------------------------------------
# corpus and reporting
# --------------------------------------------------------------------------


def test_corpus_clips_are_deterministic():
    for name, builder in benchmark.CORPUS_BUILDERS.items():
        first, second = builder(), builder()
        assert first.frames == second.frames, f"{name} is not reproducible"
        assert len(first.frames[0]) == first.width * first.height * 3
        assert all(len(f) == len(first.frames[0]) for f in first.frames)


def test_corpus_covers_both_kinds_and_palette_extremes():
    clips = {name: builder() for name, builder in benchmark.CORPUS_BUILDERS.items()}
    assert clips["checkerboard"].colors == 2
    assert clips["photo"].colors > 256
    assert clips["bounce"].kind == "video"
    assert clips["checkerboard"].kind == "image"
    assert clips["bounce"].raw_bytes == 64 * 64 * 3 * 12


def test_load_inputs_reads_file_and_directory(tmp_path):
    pixels = gradient(8, 6)
    single = tmp_path / "one.ppm"
    single.write_bytes(rc.write_ppm(8, 6, pixels))
    frames_dir = tmp_path / "clip"
    frames_dir.mkdir()
    for i in range(3):
        (frames_dir / f"f{i:02d}.ppm").write_bytes(rc.write_ppm(8, 6, pixels))

    clips = benchmark.load_inputs([str(single), str(frames_dir)])
    assert [c.name for c in clips] == ["one", "clip"]
    assert clips[0].kind == "image"
    assert clips[1].kind == "video" and len(clips[1].frames) == 3


def test_load_inputs_rejects_mismatched_frame_sizes(tmp_path):
    frames_dir = tmp_path / "clip"
    frames_dir.mkdir()
    (frames_dir / "a.ppm").write_bytes(rc.write_ppm(4, 4, gradient(4, 4)))
    (frames_dir / "b.ppm").write_bytes(rc.write_ppm(8, 4, gradient(8, 4)))
    with pytest.raises(SystemExit):
        benchmark.load_inputs([str(frames_dir)])


def test_measure_reference_verifies_every_contender():
    clip = benchmark.CORPUS_BUILDERS["sprite"]()
    results = benchmark.measure_reference(clip, verify=True)
    formats = {r.format for r in results}
    assert {"raw RGB", "gzip", "bzip2", "xz", "PNG", "PNG8", "GIF", "QOI"} <= formats
    assert all(r.lossless for r in results), "a reference codec failed to round-trip"
    raw = next(r for r in results if r.format == "raw RGB")
    png = next(r for r in results if r.format == "PNG")
    assert raw.size == clip.raw_bytes
    assert png.size < raw.size


def test_measure_reference_marks_gif_unavailable_over_256_colors():
    clip = benchmark.CORPUS_BUILDERS["photo"]()
    results = benchmark.measure_reference(clip, verify=True)
    gif = next(r for r in results if r.format == "GIF")
    assert gif.size is None and "256" in gif.note


def test_psnr_is_infinite_for_identical_frames():
    frame = gradient(8, 8)
    assert benchmark._psnr([frame], [frame]) == float("inf")
    darker = bytes(max(0, b - 10) for b in frame)
    assert 20.0 < benchmark._psnr([frame], [darker]) < 60.0


def test_render_markdown_reports_sizes_and_notes():
    clip = benchmark.CORPUS_BUILDERS["checkerboard"]()
    results = benchmark.measure_reference(clip, verify=False)
    report = benchmark.render_markdown([clip], results, "- a note")
    assert "### `checkerboard`" in report
    assert "| PNG |" in report and "| GIF |" in report
    assert "Best size per clip" in report
    assert "- a note" in report


def test_csv_and_json_outputs(tmp_path):
    clip = benchmark.CORPUS_BUILDERS["sprite"]()
    results = benchmark.measure_reference(clip, verify=True)

    csv_path = tmp_path / "out.csv"
    benchmark.write_csv(csv_path, [clip], results)
    lines = csv_path.read_text().splitlines()
    assert lines[0].startswith("clip,format,variant,bytes")
    assert len(lines) == len(results) + 1

    json_path = tmp_path / "out.json"
    benchmark.write_json(json_path, [clip], results, {"binary": "./4splat"})
    payload = json.loads(json_path.read_text())
    assert payload["clips"][0]["name"] == "sprite"
    assert len(payload["results"]) == len(results)
    assert payload["environment"]["binary"] == "./4splat"


def test_unknown_corpus_name_is_rejected():
    with pytest.raises(SystemExit):
        benchmark.main(["--corpus", "does-not-exist"])


# --------------------------------------------------------------------------
# end-to-end against the built CLI
# --------------------------------------------------------------------------


SPLAT_BINARY = os.environ.get("SPLAT_BINARY", str(ROOT / "4splat"))
needs_binary = pytest.mark.skipif(
    not Path(SPLAT_BINARY).exists(),
    reason=f"{SPLAT_BINARY} not built (run 'make plain')",
)


@needs_binary
def test_end_to_end_benchmark_run(tmp_path, capsys):
    csv_path = tmp_path / "bench.csv"
    exit_code = benchmark.main(
        [
            "--binary", SPLAT_BINARY,
            "--corpus", "checkerboard,bounce",
            "--schemes", "none,rle",
            "--csv", str(csv_path),
        ]
    )
    report = capsys.readouterr().out
    assert exit_code == 0, "a codec under test failed verification"
    assert "### `checkerboard`" in report and "### `bounce`" in report
    assert "| 4splat |" in report
    rows = csv_path.read_text().splitlines()
    assert any(row.startswith("bounce,4splat") for row in rows)
    # RLE has to beat the uncompressed index on a flat, blocky clip.
    sizes = {}
    for row in rows[1:]:
        clip, fmt, variant, size = row.split(",")[:4]
        if fmt == "4splat" and clip == "checkerboard":
            sizes[variant] = int(size)
    assert sizes["rle"] < sizes["none"]


@needs_binary
def test_splat_runner_probes_schemes(tmp_path):
    runner = benchmark.SplatRunner(SPLAT_BINARY, tmp_path)
    supported = runner.supported_schemes(benchmark.ALL_SCHEMES)
    assert "none" in supported and "rle" in supported
    assert set(supported) <= set(benchmark.ALL_SCHEMES)


@needs_binary
def test_splat_round_trips_a_clip_losslessly(tmp_path):
    clip = benchmark.CORPUS_BUILDERS["screencast"]()
    runner = benchmark.SplatRunner(SPLAT_BINARY, tmp_path)
    out = tmp_path / "clip.4spl"
    _ms, proc = runner.encode(clip, "rle", None, out)
    assert proc.returncode == 0, proc.stdout.decode()
    assert runner.decode(clip, out) == clip.frames


def test_missing_binary_is_reported_clearly(tmp_path):
    with pytest.raises(SystemExit) as excinfo:
        benchmark.SplatRunner(str(tmp_path / "nope"), tmp_path)
    assert "not found" in str(excinfo.value)
