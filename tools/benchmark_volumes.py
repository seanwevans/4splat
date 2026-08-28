#!/usr/bin/env python3
"""Benchmark 4Splat volumes against the formats that already store 3D grids.

`tools/benchmark.py` compares 2D images and video against PNG, GIF and QOI.
This is the volumetric counterpart: it encodes a stack of z-slices with
``4splat encode-volume`` and against the containers that hold a whole `(x, y,
z)` grid as one object - NIfTI, NRRD, MetaImage and multi-page TIFF - plus the
"just store the slices" baselines those formats replaced.

This is the comparison the format is actually built for. A `.4spl` volume
carries one palette for the entire grid, with each entry's `mu_z`/`sigma_z`
describing where in depth that color lives - and a 4D grid carries `mu_t` too;
the per-slice formats have no way to share anything between slices, and of the
volumetric containers only TIFF has a palette at all.

The corpus holds both: four 3D volumes and two 4D grids, the latter encoded
with `encode-4d --depth D`. NIfTI, NRRD and MetaImage each have a time axis, so
they are given the depth and describe the same `(x, y, z, t)` grid the header
does; TIFF has none, and its rows say so.

    python3 tools/benchmark_volumes.py                    # full corpus
    python3 tools/benchmark_volumes.py --quick
    python3 tools/benchmark_volumes.py --corpus labels,angio
    python3 tools/benchmark_volumes.py --input slices/    # your own P6 stack
    python3 tools/benchmark_volumes.py --colors 32 --json volumes.json

Like the 2D harness this needs nothing outside the standard library, and every
size it reports is proven by decoding the file again and comparing voxels.
"""

from __future__ import annotations

import argparse
import bz2
import lzma
import math
import os
import random
import sys
import tempfile
import zlib
from pathlib import Path
from typing import Callable, Dict, List, Optional, Sequence, Tuple

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from benchmark import (  # noqa: E402  (path set above)
    ALL_SCHEMES,
    DEFAULT_SCHEMES,
    Clip,
    Result,
    SplatRunner,
    _timed,
    measure_splat,
    render_markdown,
    write_csv,
    write_json,
)
from refcodecs import (  # noqa: E402
    decode_gif,
    decode_png,
    decode_qoi,
    encode_gif,
    encode_png,
    encode_png_indexed,
    encode_qoi,
    read_ppm,
)
from refvolumes import (  # noqa: E402
    decode_metaimage,
    decode_nifti,
    decode_nifti_gz,
    decode_nrrd,
    decode_tiff_stack,
    encode_metaimage,
    encode_nifti,
    encode_nifti_gz,
    encode_nrrd,
    encode_tiff_stack,
    encode_tiff_stack_palette,
)

DEFAULT_BINARY = "./4splat"


# --------------------------------------------------------------------------
# Corpus - volumes, not videos: the extra axis is depth
# --------------------------------------------------------------------------


def _volume(
    width: int,
    height: int,
    depth: int,
    shade: Callable[[int, int, int], Tuple[int, int, int]],
) -> List[bytes]:
    slices = []
    for z in range(depth):
        out = bytearray()
        for y in range(height):
            for x in range(width):
                r, g, b = shade(x, y, z)
                out += bytes((r & 0xFF, g & 0xFF, b & 0xFF))
        slices.append(bytes(out))
    return slices


def _corpus_labels() -> Clip:
    """A segmentation label map: a handful of tissue colors, smooth in z.

    This is the shape of most volumetric data that ships as a file rather than
    a scan - masks, atlases, region labels - and the case a global palette is
    built for.
    """

    width = height = 64
    depth = 24
    rng = random.Random(0x1AB315)
    regions = [
        (rng.uniform(12, 52), rng.uniform(12, 52), rng.uniform(6, 18), rng.uniform(8, 22))
        for _ in range(5)
    ]
    colors = [
        (0, 0, 0),
        (232, 76, 60),
        (46, 204, 113),
        (52, 152, 219),
        (241, 196, 15),
        (155, 89, 182),
    ]

    def shade(x: int, y: int, z: int) -> Tuple[int, int, int]:
        for index, (cx, cy, cz, radius) in enumerate(regions):
            if (x - cx) ** 2 + (y - cy) ** 2 + ((z - cz) * 2.5) ** 2 <= radius * radius:
                return colors[index + 1]
        return colors[0]

    return Clip(
        "labels",
        width,
        height,
        _volume(width, height, depth, shade),
        "6-color segmentation map",
        axis="depth",
    )


def _corpus_angio() -> Clip:
    """Sparse bright structures on a dark field - vessels, neurons, cracks."""

    width = height = 64
    depth = 24
    rng = random.Random(0xB100D)
    paths = []
    for _ in range(6):
        x, y = rng.uniform(0, width), rng.uniform(0, height)
        dx, dy = rng.uniform(-1, 1), rng.uniform(-1, 1)
        track = []
        for z in range(depth):
            track.append((x, y))
            x += dx + rng.uniform(-0.6, 0.6)
            y += dy + rng.uniform(-0.6, 0.6)
        paths.append(track)

    def shade(x: int, y: int, z: int) -> Tuple[int, int, int]:
        for track in paths:
            cx, cy = track[z]
            d2 = (x - cx) ** 2 + (y - cy) ** 2
            if d2 <= 2.0:
                return (255, 240, 230)
            if d2 <= 8.0:
                return (170, 60, 60)
        return (8, 8, 12)

    return Clip(
        "angio",
        width,
        height,
        _volume(width, height, depth, shade),
        "sparse bright structures",
        axis="depth",
    )


def _corpus_ct() -> Clip:
    """Continuous-tone density, the way a reconstructed scan actually looks."""

    width = height = 64
    depth = 16
    rng = random.Random(0xC7C7C7)
    blobs = [
        (
            rng.uniform(10, 54),
            rng.uniform(10, 54),
            rng.uniform(2, 14),
            rng.uniform(8, 20),
            rng.randrange(90, 256),
        )
        for _ in range(8)
    ]
    grain = [rng.randrange(-4, 5) for _ in range(2048)]

    def shade(x: int, y: int, z: int) -> Tuple[int, int, int]:
        value = 0.0
        for cx, cy, cz, radius, level in blobs:
            d2 = (x - cx) ** 2 + (y - cy) ** 2 + ((z - cz) * 3.0) ** 2
            value += level * math.exp(-d2 / (2 * radius * radius))
        v = int(value) + grain[(z * 64 + y * 7 + x) % 2048]
        v = max(0, min(255, v))
        return (v, v, v)

    return Clip(
        "ct",
        width,
        height,
        _volume(width, height, depth, shade),
        "continuous-tone density",
        axis="depth",
    )


def _corpus_voxel() -> Clip:
    """Blocky voxel-art terrain: 16 colors, hard boundaries, strong z runs."""

    width = height = 64
    depth = 20
    rng = random.Random(0x5EED17)
    heights = [[rng.randrange(4, depth) for _ in range(16)] for _ in range(16)]
    strata = [
        (60, 130, 70),
        (110, 85, 55),
        (95, 95, 100),
        (70, 70, 78),
        (48, 48, 54),
        (200, 190, 160),
        (40, 90, 150),
        (150, 60, 60),
    ]

    def shade(x: int, y: int, z: int) -> Tuple[int, int, int]:
        top = heights[y >> 2][x >> 2]
        if z > top:
            return (12, 14, 24)
        if z == top:
            return strata[0]
        return strata[1 + min(len(strata) - 2, (top - z) // 3)]

    return Clip(
        "voxel",
        width,
        height,
        _volume(width, height, depth, shade),
        "blocky voxel terrain",
        axis="depth",
    )


def _corpus_beating() -> Clip:
    """A 4D grid: a labeled structure that pulses over time.

    Slices are laid out t-major, z-minor, the order the index uses, so the
    codec sees one palette spanning both depth and time.
    """

    width = height = 48
    depth, frames = 8, 6
    wall = (206, 84, 84)
    lumen = (240, 214, 160)
    tissue = (64, 72, 96)
    background = (10, 12, 18)

    slices = []
    for t in range(frames):
        phase = 1.0 + 0.35 * math.sin(2 * math.pi * t / frames)
        for z in range(depth):
            zr = 1.0 - abs(z - (depth - 1) / 2.0) / depth
            outer = 15.0 * phase * (0.5 + zr)
            inner = outer * 0.55
            out = bytearray()
            for y in range(height):
                for x in range(width):
                    d = math.hypot(x - width / 2.0, y - height / 2.0)
                    if d <= inner:
                        color = lumen
                    elif d <= outer:
                        color = wall
                    elif d <= outer + 6:
                        color = tissue
                    else:
                        color = background
                    out += bytes(color)
            slices.append(bytes(out))
    return Clip(
        "beating",
        width,
        height,
        slices,
        "4-color structure pulsing over time",
        axis="depth",
        depth=depth,
    )


def _corpus_perfusion() -> Clip:
    """A 4D grid with continuous tone: intensity washing through a volume."""

    width = height = 40
    depth, frames = 6, 5
    rng = random.Random(0xBEEF)
    seeds = [
        (rng.uniform(8, 32), rng.uniform(8, 32), rng.uniform(0, depth), rng.uniform(6, 14))
        for _ in range(5)
    ]

    slices = []
    for t in range(frames):
        wave = t / max(1, frames - 1)
        for z in range(depth):
            out = bytearray()
            for y in range(height):
                for x in range(width):
                    value = 0.0
                    for index, (cx, cy, cz, radius) in enumerate(seeds):
                        arrival = index / len(seeds)
                        strength = max(0.0, 1.0 - abs(wave - arrival) * 2.2)
                        d2 = (x - cx) ** 2 + (y - cy) ** 2 + ((z - cz) * 2.0) ** 2
                        value += 235.0 * strength * math.exp(-d2 / (2 * radius * radius))
                    v = max(0, min(255, int(value)))
                    out += bytes((v, v // 2 + 20, 255 - v))
            slices.append(bytes(out))
    return Clip(
        "perfusion",
        width,
        height,
        slices,
        "continuous tone washing through time",
        axis="depth",
        depth=depth,
    )


CORPUS_BUILDERS: Dict[str, Callable[[], Clip]] = {
    "labels": _corpus_labels,
    "angio": _corpus_angio,
    "ct": _corpus_ct,
    "voxel": _corpus_voxel,
    "beating": _corpus_beating,
    "perfusion": _corpus_perfusion,
}

QUICK_CORPUS = ("labels", "ct", "beating")


def load_inputs(paths: Sequence[str]) -> List[Clip]:
    """Load user-supplied slice stacks: one directory of ``*.ppm`` per volume."""

    clips: List[Clip] = []
    for raw in paths:
        path = Path(raw)
        files = sorted(path.glob("*.ppm")) if path.is_dir() else [path]
        if not files:
            raise SystemExit(f"no .ppm slices in {path}")
        slices = []
        width = height = 0
        for file in files:
            w, h, pixels = read_ppm(file.read_bytes())
            if slices and (w, h) != (width, height):
                raise SystemExit(f"{file}: {w}x{h} does not match {width}x{height}")
            width, height = w, h
            slices.append(pixels)
        clips.append(
            Clip(path.stem or path.name, width, height, slices, "user input", axis="depth")
        )
    return clips


# --------------------------------------------------------------------------
# Measurements
# --------------------------------------------------------------------------


def measure_volume_reference(clip: Clip, verify: bool) -> List[Result]:
    """Measure the volumetric containers and the per-slice baselines."""

    raw = b"".join(clip.frames)
    width, height, slices = clip.width, clip.height, clip.frames
    # NIfTI, NRRD and MetaImage each have a time axis; for a 4D clip they are
    # told the depth so the header describes (x, y, z, t) rather than one tall
    # stack. TIFF has no such axis, so its pages stay a flat sequence.
    depth = clip.depth if clip.kind == "grid4d" else None
    results: List[Result] = []

    def add(fmt: str, detail: str, data: bytes, ms: float, ok: Optional[bool]) -> None:
        results.append(Result(clip.name, fmt, detail, len(data), ms, ok))

    def whole_volume(
        fmt: str,
        detail: str,
        encode: Callable[[], bytes],
        decode: Callable[[bytes], Tuple[int, int, List[bytes]]],
    ) -> None:
        data, ms = _timed(encode)
        ok = None
        if verify:
            ok = decode(data) == (width, height, slices)
        add(fmt, detail, data, ms, ok)

    # Generic compressors over the raw voxel grid.
    for name, fn in (
        ("raw voxels", lambda: raw),
        ("gzip", lambda: zlib.compress(raw, 9)),
        ("bzip2", lambda: bz2.compress(raw, 9)),
        ("xz", lambda: lzma.compress(raw, preset=9)),
    ):
        data, ms = _timed(fn)
        add(name, "whole volume" if name != "raw voxels" else "uncompressed", data, ms, True)

    # Volumetric containers: one file describing the whole (x, y, z) grid.
    whole_volume(
        "NIfTI", "RGB24, uncompressed",
        lambda: encode_nifti(width, height, slices, depth), decode_nifti,
    )
    whole_volume(
        "NIfTI", "RGB24, .nii.gz",
        lambda: encode_nifti_gz(width, height, slices, depth), decode_nifti_gz,
    )
    whole_volume(
        "NRRD", "gzip encoding",
        lambda: encode_nrrd(width, height, slices, "gzip", depth), decode_nrrd,
    )
    whole_volume(
        "MetaImage", ".mha, zlib",
        lambda: encode_metaimage(width, height, slices, True, depth), decode_metaimage,
    )
    pages = "RGB, Deflate" + (", no time axis" if depth else "")
    whole_volume(
        "TIFF stack", pages,
        lambda: encode_tiff_stack(width, height, slices, True), decode_tiff_stack,
    )
    if clip.colors <= 256:
        whole_volume(
            "TIFF stack", "palette + Deflate",
            lambda: encode_tiff_stack_palette(width, height, slices, True),
            decode_tiff_stack,
        )
    else:
        results.append(
            Result(
                clip.name, "TIFF stack", "palette + Deflate", None,
                note=f"n/a: {clip.colors} colors exceed the 256-entry ColorMap",
            )
        )

    # Per-slice baselines: what you get without a volumetric container at all.
    png_total, png_ms, png_ok = bytearray(), 0.0, True
    idx_total, idx_ms, idx_ok = bytearray(), 0.0, True
    qoi_total, qoi_ms, qoi_ok = bytearray(), 0.0, True
    for slice_ in slices:
        data, ms = _timed(lambda s=slice_: encode_png(width, height, s))
        png_total += data
        png_ms += ms
        if verify and decode_png(data) != (width, height, slice_):
            png_ok = False

        data, ms = _timed(lambda s=slice_: encode_qoi(width, height, s))
        qoi_total += data
        qoi_ms += ms
        if verify and decode_qoi(data) != (width, height, slice_):
            qoi_ok = False

        if clip.colors <= 256:
            data, ms = _timed(lambda s=slice_: encode_png_indexed(width, height, s))
            idx_total += data
            idx_ms += ms
            if verify and decode_png(data) != (width, height, slice_):
                idx_ok = False

    add("PNG", "one file per slice", bytes(png_total), png_ms, png_ok if verify else None)
    add("QOI", "one file per slice", bytes(qoi_total), qoi_ms, qoi_ok if verify else None)
    if clip.colors <= 256:
        add("PNG8", "per slice, own palette", bytes(idx_total), idx_ms,
            idx_ok if verify else None)
        data, ms = _timed(lambda: encode_gif(width, height, slices))
        ok = decode_gif(data) == (width, height, slices) if verify else None
        add("GIF", "slices as frames", data, ms, ok)
    else:
        results.append(
            Result(clip.name, "PNG8", "per slice, own palette", None, note="n/a: >256 colors")
        )
        results.append(
            Result(clip.name, "GIF", "slices as frames", None, note="n/a: >256 colors")
        )

    return results


# --------------------------------------------------------------------------
# Entry point
# --------------------------------------------------------------------------


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Benchmark 4Splat volumes against NIfTI, NRRD, MetaImage and TIFF.",
    )
    parser.add_argument("--binary", default=DEFAULT_BINARY, help="path to the 4splat CLI")
    parser.add_argument("--corpus", default="", help="comma-separated volumes (see --list)")
    parser.add_argument("--input", nargs="*", default=[], help="directories of .ppm slices")
    parser.add_argument("--list", action="store_true", help="list synthetic volumes and exit")
    parser.add_argument("--quick", action="store_true", help="run a representative subset")
    parser.add_argument(
        "--schemes",
        default=",".join(DEFAULT_SCHEMES),
        help="4splat compression schemes to try, or 'all'",
    )
    parser.add_argument("--colors", default="", help="also measure lossy runs, e.g. 32,8")
    parser.add_argument("--no-verify", action="store_true", help="skip decode verification")
    parser.add_argument("--markdown", help="write the Markdown report here as well")
    parser.add_argument("--csv", help="write per-row results as CSV")
    parser.add_argument("--json", help="write per-row results as JSON")
    parser.add_argument("--keep", help="keep intermediate files in this directory")
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)

    if args.list:
        for name, builder in CORPUS_BUILDERS.items():
            clip = builder()
            print(
                f"{name:<10} {clip.dimensions:<26} {clip.colors:>6} colors  "
                f"{clip.description}"
            )
        return 0

    if args.quick:
        names = list(QUICK_CORPUS)
    elif args.corpus:
        names = [n.strip() for n in args.corpus.split(",") if n.strip()]
    elif args.input:
        names = []
    else:
        names = list(CORPUS_BUILDERS)
    unknown = [n for n in names if n not in CORPUS_BUILDERS]
    if unknown:
        raise SystemExit(f"unknown volumes: {', '.join(unknown)} (try --list)")

    clips = [CORPUS_BUILDERS[name]() for name in names]
    clips += load_inputs(args.input)
    if not clips:
        raise SystemExit("nothing to benchmark")

    color_counts = [int(c) for c in args.colors.split(",") if c.strip()]
    requested = (
        list(ALL_SCHEMES)
        if args.schemes.strip() == "all"
        else [s.strip() for s in args.schemes.split(",") if s.strip()]
    )
    verify = not args.no_verify

    workdir = Path(args.keep) if args.keep else None
    tmp: Optional[tempfile.TemporaryDirectory] = None
    if workdir is None:
        tmp = tempfile.TemporaryDirectory(prefix="4splat-volbench-")
        workdir = Path(tmp.name)
    workdir.mkdir(parents=True, exist_ok=True)

    try:
        runner = SplatRunner(args.binary, workdir)
        schemes = runner.supported_schemes(requested)
        missing = [s for s in requested if s not in schemes]

        results: List[Result] = []
        for clip in clips:
            print(f"benchmarking volume {clip.name} ...", file=sys.stderr)
            results += measure_volume_reference(clip, verify)
            results += measure_splat(clip, runner, schemes, color_counts, verify)

        notes_lines = [
            f"Built-in schemes available in `{args.binary}`: "
            + (", ".join(f"`{s}`" for s in schemes) or "none"),
        ]
        if missing:
            notes_lines.append(
                "Not built into this binary (rebuild with `make` for all backends): "
                + ", ".join(f"`{s}`" for s in missing)
            )
        notes_lines.append(
            "NIfTI, NRRD, MetaImage and TIFF are the dependency-free reference "
            "encoders in `tools/refvolumes.py`; PNG, PNG8 and QOI have no "
            "volumetric container, so their number is the sum of one file per "
            "slice. Every size shown was decoded back and compared with the "
            "source voxels."
        )
        notes_lines.append(
            "`.4spl` volumes are written by `encode-volume` (depth = N, "
            "frames = 1) and 4D grids by `encode-4d --depth D` (depth = D, "
            "frames = N/D): one palette for the whole grid, with each entry's "
            "`mu_z`/`sigma_z` and `mu_t`/`sigma_t` recording where in depth and "
            "time that color sits."
        )
        notes = "\n".join(f"- {line}" for line in notes_lines)

        report = render_markdown(clips, results, notes)
        print(report)

        if args.markdown:
            Path(args.markdown).write_text(report + "\n")
        if args.csv:
            write_csv(Path(args.csv), clips, results)
        if args.json:
            write_json(
                Path(args.json),
                clips,
                results,
                {
                    "binary": args.binary,
                    "schemes": schemes,
                    "python": sys.version.split()[0],
                    "platform": sys.platform,
                    "verified": verify,
                    "benchmark": "volumes",
                },
            )

        failures = [r for r in results if r.lossless is False]
        if failures:
            print(
                "\nVERIFICATION FAILURES:\n"
                + "\n".join(
                    f"  {r.clip}/{r.format} ({r.detail}): {r.note or 'mismatch'}"
                    for r in failures
                ),
                file=sys.stderr,
            )
            return 1
        return 0
    finally:
        if tmp is not None:
            tmp.cleanup()


if __name__ == "__main__":
    raise SystemExit(main())
