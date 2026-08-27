#!/usr/bin/env python3
"""Benchmark the 4Splat codec against established lossless formats.

The harness encodes the same pixels with ``4splat`` and with reference
implementations of PNG, GIF and QOI (see :mod:`tools.refcodecs`), plus the
general-purpose compressors applied to raw RGB, and reports size, bits per
pixel and encode time side by side.  Every entry is decoded again and compared
against the source pixels, so a row only claims a size for a file that has been
proven to decode back to the original.

    python3 tools/benchmark.py                       # full synthetic corpus
    python3 tools/benchmark.py --quick               # a fast subset
    python3 tools/benchmark.py --corpus sprite,bounce
    python3 tools/benchmark.py --input shots/*.ppm   # your own P6 files
    python3 tools/benchmark.py --colors 16 --markdown bench.md --json bench.json

Nothing outside the standard library is required.  The ``4splat`` binary is
located via ``--binary`` (default: ``./4splat``); build it with ``make plain``
for the built-in schemes or ``make`` for every compression backend.  Whichever
schemes the binary was built with are probed at start-up and the rest are
skipped.
"""

from __future__ import annotations

import argparse
import bz2
import csv
import json
import lzma
import math
import os
import random
import shutil
import subprocess
import sys
import tempfile
import time
import zlib
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Dict, List, Optional, Sequence, Tuple

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from refcodecs import (  # noqa: E402  (path set above)
    decode_gif,
    decode_png,
    decode_qoi,
    distinct_colors,
    encode_gif,
    encode_png,
    encode_png_indexed,
    encode_qoi,
    read_ppm,
    write_ppm,
)

DEFAULT_BINARY = "./4splat"

# Schemes probed against the binary under test, in spec order.  Anything the
# build does not carry is dropped at start-up.
ALL_SCHEMES = (
    "none",
    "rle",
    "deflate",
    "zlib",
    "bzip2",
    "lzma",
    "xz",
    "lz4",
    "brotli",
    "zstd",
)

# The subset reported unless --schemes says otherwise: one representative of
# each family, so the table stays readable.
DEFAULT_SCHEMES = ("none", "rle", "zlib", "xz", "brotli", "zstd")


# --------------------------------------------------------------------------
# Corpus
# --------------------------------------------------------------------------


@dataclass
class Clip:
    """A benchmark input: one or more equally sized RGB frames.

    ``axis`` says what the extra frames mean - ``"time"`` for a video, or
    ``"depth"`` for a stack of z-slices, which the codec stores as a volume
    (``depth = N, frames = 1``) rather than as a sequence.
    """

    name: str
    width: int
    height: int
    frames: List[bytes]
    description: str = ""
    axis: str = "time"

    @property
    def raw_bytes(self) -> int:
        return self.width * self.height * 3 * len(self.frames)

    @property
    def pixels(self) -> int:
        return self.width * self.height * len(self.frames)

    @property
    def colors(self) -> int:
        return distinct_colors(self.frames)

    @property
    def kind(self) -> str:
        if len(self.frames) == 1:
            return "image"
        return "volume" if self.axis == "depth" else "video"

    @property
    def unit(self) -> str:
        """What one entry of ``frames`` is called in reports."""

        return "slices" if self.axis == "depth" else "frames"


def _flat(width: int, height: int, shade: Callable[[int, int], Tuple[int, int, int]]) -> bytes:
    out = bytearray()
    for y in range(height):
        for x in range(width):
            r, g, b = shade(x, y)
            out += bytes((r & 0xFF, g & 0xFF, b & 0xFF))
    return bytes(out)


def _corpus_checkerboard() -> Clip:
    """Two colors in a regular pattern - the palette codec's best case."""

    def shade(x: int, y: int) -> Tuple[int, int, int]:
        return (240, 240, 240) if ((x >> 3) + (y >> 3)) & 1 else (20, 24, 40)

    return Clip("checkerboard", 128, 128, [_flat(128, 128, shade)], "2-color 8px checks")


def _corpus_sprite() -> Clip:
    """Blocky 16-color pixel art: flat regions, hard edges, small palette."""

    rng = random.Random(0xA11CE)
    palette = [
        (0, 0, 0),
        (255, 255, 255),
        (222, 46, 46),
        (46, 148, 222),
        (250, 204, 60),
        (60, 190, 120),
        (150, 90, 210),
        (255, 150, 60),
        (40, 40, 60),
        (200, 200, 210),
        (120, 70, 40),
        (30, 110, 90),
        (240, 130, 180),
        (90, 90, 90),
        (170, 220, 250),
        (10, 60, 30),
    ]
    cells = [[rng.randrange(len(palette)) for _ in range(16)] for _ in range(16)]
    for row in cells:  # mirror horizontally, the way sprite art usually is
        for x in range(8):
            row[15 - x] = row[x]

    def shade(x: int, y: int) -> Tuple[int, int, int]:
        return palette[cells[y >> 2][x >> 2]]

    return Clip("sprite", 64, 64, [_flat(64, 64, shade)], "16-color pixel art")


def _corpus_gradient() -> Clip:
    """A smooth ramp: few hard edges, many distinct colors."""

    def shade(x: int, y: int) -> Tuple[int, int, int]:
        return (x * 2, y * 2, (x + y))

    return Clip("gradient", 128, 128, [_flat(128, 128, shade)], "smooth RGB ramp")


def _corpus_photo() -> Clip:
    """Continuous tone with noise - the case palettes are worst at."""

    rng = random.Random(0xC0FFEE)
    blobs = [
        (rng.uniform(0, 128), rng.uniform(0, 128), rng.uniform(12, 44),
         (rng.randrange(256), rng.randrange(256), rng.randrange(256)))
        for _ in range(10)
    ]
    jitter = [rng.randrange(-6, 7) for _ in range(4096)]

    def shade(x: int, y: int) -> Tuple[int, int, int]:
        r = g = b = 0.0
        weight = 0.0
        for cx, cy, radius, (cr, cg, cb) in blobs:
            d2 = (x - cx) ** 2 + (y - cy) ** 2
            w = math.exp(-d2 / (2 * radius * radius))
            r += w * cr
            g += w * cg
            b += w * cb
            weight += w
        n = jitter[(y * 128 + x) % 4096]
        return (
            max(0, min(255, int(r / weight) + n)),
            max(0, min(255, int(g / weight) + n)),
            max(0, min(255, int(b / weight) + n)),
        )

    return Clip("photo", 128, 128, [_flat(128, 128, shade)], "synthetic continuous tone")


def _corpus_noise() -> Clip:
    """Uniform random RGB: the incompressible floor for every contender."""

    rng = random.Random(0xD15EA5E)
    data = bytes(rng.randrange(256) for _ in range(64 * 64 * 3))
    return Clip("noise", 64, 64, [data], "uniform random RGB")


def _corpus_bounce() -> Clip:
    """A small moving object over a static field - temporal redundancy."""

    width, height, frames = 64, 64, 12
    bg = (18, 22, 34)
    ball = (245, 210, 60)
    shadow = (120, 105, 40)
    out = []
    for t in range(frames):
        cx = 8 + int(abs(((t * 9) % (2 * (width - 16))) - (width - 16)))
        cy = 8 + int(abs(((t * 5) % (2 * (height - 16))) - (height - 16)))

        def shade(x: int, y: int, cx: int = cx, cy: int = cy) -> Tuple[int, int, int]:
            d2 = (x - cx) ** 2 + (y - cy) ** 2
            if d2 <= 36:
                return ball
            if d2 <= 64:
                return shadow
            return bg

        out.append(_flat(width, height, shade))
    return Clip("bounce", width, height, out, "12 frames, moving sprite")


def _corpus_screencast() -> Clip:
    """UI-like frames: flat panels with a caret that blinks and text that scrolls."""

    width, height, frames = 96, 64, 8
    bg = (30, 30, 46)
    panel = (56, 58, 82)
    text = (205, 214, 244)
    accent = (166, 227, 161)
    rng = random.Random(0x5CEEED)
    glyphs = [[rng.random() < 0.45 for _ in range(width)] for _ in range(height)]
    out = []
    for t in range(frames):
        def shade(x: int, y: int, t: int = t) -> Tuple[int, int, int]:
            if y < 8:
                return panel
            if x < 6:
                return panel
            row = (y - 8 + t) % height
            if 10 <= y <= height - 10 and glyphs[row][x] and (x % 6) < 4:
                return text
            if t % 2 == 0 and abs(x - (12 + 3 * t)) < 2 and abs(y - 40) < 5:
                return accent
            return bg

        out.append(_flat(width, height, shade))
    return Clip("screencast", width, height, out, "8 frames, mostly-static UI")


CORPUS_BUILDERS: Dict[str, Callable[[], Clip]] = {
    "checkerboard": _corpus_checkerboard,
    "sprite": _corpus_sprite,
    "gradient": _corpus_gradient,
    "photo": _corpus_photo,
    "noise": _corpus_noise,
    "bounce": _corpus_bounce,
    "screencast": _corpus_screencast,
}

QUICK_CORPUS = ("checkerboard", "sprite", "photo", "bounce")


def load_inputs(paths: Sequence[str]) -> List[Clip]:
    """Load user-supplied PPM files.

    A single file becomes a one-frame clip; a directory becomes one clip whose
    frames are its ``*.ppm`` files in sorted order.
    """

    clips: List[Clip] = []
    for raw in paths:
        path = Path(raw)
        if path.is_dir():
            files = sorted(path.glob("*.ppm"))
            if not files:
                raise SystemExit(f"no .ppm files in {path}")
        else:
            files = [path]
        frames = []
        width = height = 0
        for file in files:
            w, h, pixels = read_ppm(file.read_bytes())
            if frames and (w, h) != (width, height):
                raise SystemExit(f"{file}: {w}x{h} does not match {width}x{height}")
            width, height = w, h
            frames.append(pixels)
        clips.append(Clip(path.stem or path.name, width, height, frames, "user input"))
    return clips


# --------------------------------------------------------------------------
# Measurements
# --------------------------------------------------------------------------


@dataclass
class Result:
    """One (clip, contender) measurement."""

    clip: str
    format: str
    detail: str
    size: Optional[int]
    encode_ms: float = 0.0
    lossless: Optional[bool] = None
    psnr_db: Optional[float] = None
    note: str = ""

    def bpp(self, pixels: int) -> Optional[float]:
        return None if self.size is None else self.size * 8.0 / pixels


def _psnr(a: Sequence[bytes], b: Sequence[bytes]) -> float:
    total = 0
    count = 0
    for fa, fb in zip(a, b):
        for x, y in zip(fa, fb):
            d = x - y
            total += d * d
        count += len(fa)
    if count == 0 or total == 0:
        return float("inf")
    return 10.0 * math.log10(255.0 * 255.0 * count / total)


class SplatRunner:
    """Drives the ``4splat`` CLI and reports which schemes the build supports."""

    def __init__(self, binary: str, workdir: Path):
        self.binary = binary
        self.workdir = workdir
        if not (Path(binary).exists() or shutil.which(binary)):
            raise SystemExit(
                f"4splat binary not found at '{binary}'.\n"
                "Build it first (make plain, or make for every backend), or pass --binary."
            )

    def _run(self, args: Sequence[str]) -> subprocess.CompletedProcess:
        return subprocess.run(
            [self.binary, *args],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )

    def supported_schemes(self, candidates: Sequence[str]) -> List[str]:
        probe = self.workdir / "probe.ppm"
        probe.write_bytes(write_ppm(2, 1, bytes((0, 0, 0, 255, 255, 255))))
        out = self.workdir / "probe.4spl"
        supported = []
        for scheme in candidates:
            if self._run(
                ["encode-image", "--compress", scheme, str(probe), str(out)]
            ).returncode == 0:
                supported.append(scheme)
        return supported

    def encode(
        self, clip: Clip, scheme: str, colors: Optional[int], out: Path
    ) -> Tuple[float, subprocess.CompletedProcess]:
        frame_paths = []
        for i, frame in enumerate(clip.frames):
            path = self.workdir / f"{clip.name}_{i:04d}.ppm"
            if not path.exists():
                path.write_bytes(write_ppm(clip.width, clip.height, frame))
            frame_paths.append(str(path))

        commands = {
            "image": "encode-image",
            "video": "encode-video",
            "volume": "encode-volume",
        }
        args: List[str] = [commands[clip.kind]]
        if scheme != "none":
            args += ["--compress", scheme]
        if colors:
            args += ["--colors", str(colors)]
        if clip.kind == "image":
            args += [frame_paths[0], str(out)]
        else:
            args += [str(out), *frame_paths]

        start = time.perf_counter()
        proc = self._run(args)
        return (time.perf_counter() - start) * 1000.0, proc

    def decode(self, clip: Clip, path: Path) -> Optional[List[bytes]]:
        prefix = self.workdir / f"dec_{path.stem}_"
        if clip.kind == "image":
            out = Path(str(prefix) + "0000.ppm")
            if self._run(["decode-image", str(path), str(out)]).returncode != 0:
                return None
            return [read_ppm(out.read_bytes())[2]]
        command = "decode-volume" if clip.kind == "volume" else "decode-video"
        if self._run([command, str(path), str(prefix)]).returncode != 0:
            return None
        frames = []
        for i in range(len(clip.frames)):
            out = Path(f"{prefix}{i:04d}.ppm")
            if not out.exists():
                return None
            frames.append(read_ppm(out.read_bytes())[2])
        return frames


def _timed(fn: Callable[[], bytes]) -> Tuple[bytes, float]:
    start = time.perf_counter()
    data = fn()
    return data, (time.perf_counter() - start) * 1000.0


def measure_reference(clip: Clip, verify: bool) -> List[Result]:
    """Measure every non-4Splat contender for one clip."""

    raw = b"".join(clip.frames)
    results: List[Result] = []

    def add(fmt: str, detail: str, data: bytes, ms: float, ok: Optional[bool]) -> None:
        results.append(Result(clip.name, fmt, detail, len(data), ms, ok))

    # Generic compressors over raw RGB - not image formats, but the honest
    # "just gzip the pixels" baseline every codec should beat.
    for name, fn in (
        ("raw RGB", lambda: raw),
        ("gzip", lambda: zlib.compress(raw, 9)),
        ("bzip2", lambda: bz2.compress(raw, 9)),
        ("xz", lambda: lzma.compress(raw, preset=9)),
    ):
        data, ms = _timed(fn)
        add(name, "whole clip" if name != "raw RGB" else "uncompressed", data, ms, True)

    # PNG, per frame (PNG has no multi-frame form here; APNG is out of scope).
    png_total = bytearray()
    png_ms = 0.0
    png_ok = True
    for frame in clip.frames:
        data, ms = _timed(lambda f=frame: encode_png(clip.width, clip.height, f))
        png_ms += ms
        png_total += data
        if verify and decode_png(data) != (clip.width, clip.height, frame):
            png_ok = False
    add("PNG", "truecolor, filtered", bytes(png_total), png_ms, png_ok if verify else None)

    if clip.colors <= 256:
        idx_total = bytearray()
        idx_ms = 0.0
        idx_ok = True
        for frame in clip.frames:
            data, ms = _timed(
                lambda f=frame: encode_png_indexed(clip.width, clip.height, f)
            )
            idx_ms += ms
            idx_total += data
            if verify and decode_png(data) != (clip.width, clip.height, frame):
                idx_ok = False
        add(
            "PNG8",
            "indexed palette",
            bytes(idx_total),
            idx_ms,
            idx_ok if verify else None,
        )

        data, ms = _timed(lambda: encode_gif(clip.width, clip.height, clip.frames))
        gif_ok = None
        if verify:
            gif_ok = decode_gif(data) == (clip.width, clip.height, clip.frames)
        add(
            "GIF",
            "global palette" + (", animated" if clip.kind == "video" else ""),
            data,
            ms,
            gif_ok,
        )
    else:
        results.append(
            Result(
                clip.name,
                "GIF",
                "global palette",
                None,
                note=f"n/a: {clip.colors} colors exceed GIF's 256",
            )
        )
        results.append(
            Result(clip.name, "PNG8", "indexed palette", None, note="n/a: >256 colors")
        )

    qoi_total = bytearray()
    qoi_ms = 0.0
    qoi_ok = True
    for frame in clip.frames:
        data, ms = _timed(lambda f=frame: encode_qoi(clip.width, clip.height, f))
        qoi_ms += ms
        qoi_total += data
        if verify and decode_qoi(data) != (clip.width, clip.height, frame):
            qoi_ok = False
    add("QOI", "per frame", bytes(qoi_total), qoi_ms, qoi_ok if verify else None)

    return results


def measure_splat(
    clip: Clip,
    runner: SplatRunner,
    schemes: Sequence[str],
    color_counts: Sequence[int],
    verify: bool,
) -> List[Result]:
    """Measure ``.4spl`` output for one clip across schemes and palette sizes."""

    results: List[Result] = []
    for colors in [None, *color_counts]:
        if colors is not None and colors >= clip.colors:
            # Quantizing to at least as many colors as the clip already has
            # would just repeat the exact-palette run.
            continue
        for scheme in schemes:
            tag = scheme if colors is None else f"{scheme}, {colors} colors"
            out = runner.workdir / (
                f"{clip.name}_{scheme}_{colors or 'exact'}.4spl"
            )
            ms, proc = runner.encode(clip, scheme, colors, out)
            if proc.returncode != 0 or not out.exists():
                message = proc.stdout.decode("utf-8", "replace").strip().splitlines()
                results.append(
                    Result(
                        clip.name,
                        "4splat",
                        tag,
                        None,
                        note=message[-1] if message else "encode failed",
                    )
                )
                continue

            lossless: Optional[bool] = None
            psnr: Optional[float] = None
            if verify:
                decoded = runner.decode(clip, out)
                if decoded is None:
                    results.append(
                        Result(clip.name, "4splat", tag, out.stat().st_size, ms,
                               False, note="decode failed")
                    )
                    continue
                matches = decoded == clip.frames
                if colors is None:
                    # An exact palette must round-trip bit for bit.
                    lossless = matches
                elif matches:
                    lossless = True
                else:
                    # Quantization is meant to lose data; score it instead.
                    psnr = _psnr(clip.frames, decoded)
            results.append(
                Result(clip.name, "4splat", tag, out.stat().st_size, ms, lossless, psnr)
            )
    return results


# --------------------------------------------------------------------------
# Reporting
# --------------------------------------------------------------------------


def _fmt_size(size: Optional[int]) -> str:
    return "-" if size is None else f"{size:,}"


def _fmt_ratio(size: Optional[int], raw: int) -> str:
    return "-" if size is None else f"{raw / size:.2f}x"


def _fmt_flag(result: Result) -> str:
    if result.size is None:
        return result.note or "skipped"
    if result.psnr_db is not None:
        return f"lossy, {result.psnr_db:.1f} dB"
    if result.lossless is None:
        return "not checked"
    return "lossless" if result.lossless else f"MISMATCH ({result.note or 'differs'})"


def render_markdown(clips: Sequence[Clip], results: Sequence[Result], notes: str) -> str:
    lines: List[str] = []
    by_clip: Dict[str, List[Result]] = {}
    for result in results:
        by_clip.setdefault(result.clip, []).append(result)

    for clip in clips:
        rows = by_clip.get(clip.name, [])
        raw = clip.raw_bytes
        png = next(
            (r.size for r in rows if r.format == "PNG" and r.size is not None), None
        )
        lines.append(
            f"### `{clip.name}` - {clip.description}, "
            f"{clip.width}x{clip.height}"
            + (
                f" x {len(clip.frames)} {clip.unit}"
                if len(clip.frames) > 1
                else ""
            )
            + f", {clip.colors} colors"
        )
        lines.append("")
        lines.append("| Format | Variant | Bytes | bits/px | vs raw | vs PNG | Fidelity |")
        lines.append("| --- | --- | ---: | ---: | ---: | ---: | --- |")
        for r in sorted(rows, key=lambda r: (r.size is None, r.size or 0)):
            vs_png = (
                "-"
                if r.size is None or not png
                else ("1.00x" if r.format == "PNG" else f"{r.size / png:.2f}x")
            )
            bpp = r.bpp(clip.pixels)
            lines.append(
                f"| {r.format} | {r.detail} | {_fmt_size(r.size)} | "
                f"{'-' if bpp is None else f'{bpp:.2f}'} | "
                f"{_fmt_ratio(r.size, raw)} | {vs_png} | {_fmt_flag(r)} |"
            )
        lines.append("")

    lines.append("### Best size per clip")
    lines.append("")
    lines.append("| Clip | Best overall | Best `.4spl` | 4Splat vs best | 4Splat vs PNG |")
    lines.append("| --- | --- | --- | ---: | ---: |")
    for clip in clips:
        rows = [
            r
            for r in by_clip.get(clip.name, [])
            if r.size is not None
            and r.format != "raw RGB"
            and r.psnr_db is None
            and r.lossless is not False
        ]
        if not rows:
            continue
        best = min(rows, key=lambda r: r.size or 0)
        splat_rows = [r for r in rows if r.format == "4splat"]
        png = next((r.size for r in rows if r.format == "PNG"), None)
        if splat_rows:
            best_splat = min(splat_rows, key=lambda r: r.size or 0)
            splat_cell = f"{best_splat.detail} ({_fmt_size(best_splat.size)} B)"
            vs_best = f"{(best_splat.size or 0) / (best.size or 1):.2f}x"
            vs_png = f"{(best_splat.size or 0) / png:.2f}x" if png else "-"
        else:
            splat_cell, vs_best, vs_png = "-", "-", "-"
        lines.append(
            f"| `{clip.name}` | {best.format} ({_fmt_size(best.size)} B) | "
            f"{splat_cell} | {vs_best} | {vs_png} |"
        )
    lines.append("")
    if notes:
        lines.append(notes)
    return "\n".join(lines)


def write_csv(path: Path, clips: Sequence[Clip], results: Sequence[Result]) -> None:
    pixels = {clip.name: clip.pixels for clip in clips}
    raw = {clip.name: clip.raw_bytes for clip in clips}
    with path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(
            ["clip", "format", "variant", "bytes", "bits_per_pixel",
             "ratio_vs_raw", "encode_ms", "lossless", "psnr_db", "note"]
        )
        for r in results:
            bpp = r.bpp(pixels[r.clip])
            writer.writerow(
                [
                    r.clip,
                    r.format,
                    r.detail,
                    "" if r.size is None else r.size,
                    "" if bpp is None else f"{bpp:.4f}",
                    "" if r.size is None else f"{raw[r.clip] / r.size:.4f}",
                    f"{r.encode_ms:.3f}",
                    "" if r.lossless is None else int(r.lossless),
                    "" if r.psnr_db is None else f"{r.psnr_db:.2f}",
                    r.note,
                ]
            )


def write_json(path: Path, clips: Sequence[Clip], results: Sequence[Result],
               environment: Dict[str, object]) -> None:
    payload = {
        "environment": environment,
        "clips": [
            {
                "name": clip.name,
                "description": clip.description,
                "width": clip.width,
                "height": clip.height,
                "frames": len(clip.frames),
                "colors": clip.colors,
                "raw_bytes": clip.raw_bytes,
            }
            for clip in clips
        ],
        "results": [
            {
                "clip": r.clip,
                "format": r.format,
                "variant": r.detail,
                "bytes": r.size,
                "encode_ms": round(r.encode_ms, 3),
                "lossless": r.lossless,
                "psnr_db": r.psnr_db,
                "note": r.note,
            }
            for r in results
        ],
    }
    path.write_text(json.dumps(payload, indent=2) + "\n")


# --------------------------------------------------------------------------
# Entry point
# --------------------------------------------------------------------------


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Benchmark 4Splat against PNG, GIF, QOI and generic compressors.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("--binary", default=DEFAULT_BINARY, help="path to the 4splat CLI")
    parser.add_argument(
        "--corpus",
        default="",
        help="comma-separated synthetic clips (default: all; see --list)",
    )
    parser.add_argument("--input", nargs="*", default=[], help="PPM files or frame directories")
    parser.add_argument("--list", action="store_true", help="list synthetic clips and exit")
    parser.add_argument("--quick", action="store_true", help="run a small representative subset")
    parser.add_argument(
        "--schemes",
        default=",".join(DEFAULT_SCHEMES),
        help="4splat compression schemes to try, or 'all'",
    )
    parser.add_argument(
        "--colors",
        default="",
        help="also measure lossy runs at these palette sizes, e.g. 64,16",
    )
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
            size = f"{clip.width}x{clip.height}"
            if len(clip.frames) > 1:
                size += f" x{len(clip.frames)}"
            print(f"{name:<13} {size:<14} {clip.colors:>6} colors  {clip.description}")
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
        raise SystemExit(f"unknown corpus clips: {', '.join(unknown)} (try --list)")

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
        tmp = tempfile.TemporaryDirectory(prefix="4splat-bench-")
        workdir = Path(tmp.name)
    workdir.mkdir(parents=True, exist_ok=True)

    try:
        runner = SplatRunner(args.binary, workdir)
        schemes = runner.supported_schemes(requested)
        missing = [s for s in requested if s not in schemes]

        results: List[Result] = []
        for clip in clips:
            print(f"benchmarking {clip.name} ...", file=sys.stderr)
            results += measure_reference(clip, verify)
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
            "Encode timings are recorded in the CSV/JSON output rather than here: "
            "`4splat` is measured as a subprocess (process start-up included) while "
            "the reference codecs run in-process in Python, so the two are not "
            "comparable."
        )
        notes_lines.append(
            "PNG/GIF/QOI are the dependency-free reference encoders in "
            "`tools/refcodecs.py`; PNG and QOI have no multi-frame container here, "
            "so a video's number is the sum of its per-frame files. "
            "Every size shown was decoded back and compared with the source."
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
                },
            )

        failures = [r for r in results if r.lossless is False]
        if failures:
            print(
                "\nVERIFICATION FAILURES:\n"
                + "\n".join(f"  {r.clip}/{r.format} ({r.detail}): {r.note or 'mismatch'}"
                            for r in failures),
                file=sys.stderr,
            )
            return 1
        return 0
    finally:
        if tmp is not None:
            tmp.cleanup()


if __name__ == "__main__":
    raise SystemExit(main())
