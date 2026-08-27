"""Reference encoders/decoders for the formats 4Splat is benchmarked against.

The benchmark harness (``tools/benchmark.py``) compares ``.4spl`` output with
what established lossless formats produce for the same pixels.  Rather than
shelling out to encoders that may or may not be installed, this module carries
small, dependency-free implementations of the three that matter most for a
palette-based codec:

* **PNG** (RFC 2083) - truecolor and indexed, DEFLATE via :mod:`zlib`, with the
  per-scanline adaptive filtering real encoders use.
* **GIF89a** - LZW over a global color table, animated when given several
  frames.  This is 4Splat's closest relative: an indexed format with one
  palette shared across every frame.
* **QOI** (the Quite OK Image format, v1.0) - a modern byte-oriented lossless
  codec that gets within range of PNG with none of the entropy coding.

Every encoder here has a matching decoder so the benchmark can prove the bytes
it counted are a real, decodable file of that format rather than plausible
noise.  All of them are pixel-exact: encode-then-decode returns the input.

Images are passed around as ``(width, height, rgb)`` where ``rgb`` is
``width * height * 3`` bytes of 8-bit RGB in row-major order - the same layout
as the binary PPM (``P6``) files the 4Splat CLI reads and writes.
"""

from __future__ import annotations

import struct
import zlib
from typing import Dict, List, Sequence, Tuple

__all__ = [
    "PpmError",
    "read_ppm",
    "write_ppm",
    "distinct_colors",
    "build_palette",
    "encode_png",
    "decode_png",
    "encode_gif",
    "decode_gif",
    "encode_qoi",
    "decode_qoi",
]


class PpmError(ValueError):
    """Raised for malformed PPM input."""


# --------------------------------------------------------------------------
# PPM (P6) - the interchange format shared with the 4Splat CLI
# --------------------------------------------------------------------------


def _ppm_token(data: bytes, pos: int) -> Tuple[bytes, int]:
    """Return the next whitespace-delimited token and the offset past it."""

    while pos < len(data):
        ch = data[pos : pos + 1]
        if ch == b"#":
            while pos < len(data) and data[pos : pos + 1] not in (b"\n", b"\r"):
                pos += 1
        elif ch.isspace():
            pos += 1
        else:
            break
    start = pos
    while pos < len(data) and not data[pos : pos + 1].isspace():
        pos += 1
    if start == pos:
        raise PpmError("unexpected end of PPM header")
    return data[start:pos], pos


def read_ppm(data: bytes) -> Tuple[int, int, bytes]:
    """Parse a binary PPM (``P6``, maxval 255) into ``(width, height, rgb)``."""

    magic, pos = _ppm_token(data, 0)
    if magic != b"P6":
        raise PpmError(f"not a binary PPM (magic {magic!r})")
    width_tok, pos = _ppm_token(data, pos)
    height_tok, pos = _ppm_token(data, pos)
    maxval_tok, pos = _ppm_token(data, pos)
    try:
        width = int(width_tok)
        height = int(height_tok)
        maxval = int(maxval_tok)
    except ValueError as exc:  # pragma: no cover - malformed header
        raise PpmError("non-numeric PPM header field") from exc
    if maxval != 255:
        raise PpmError(f"unsupported PPM maxval {maxval} (only 255)")
    if width <= 0 or height <= 0:
        raise PpmError(f"degenerate PPM dimensions {width}x{height}")
    pixels = data[pos + 1 : pos + 1 + width * height * 3]
    if len(pixels) != width * height * 3:
        raise PpmError("truncated PPM pixel data")
    return width, height, pixels


def write_ppm(width: int, height: int, rgb: bytes) -> bytes:
    """Serialize ``rgb`` as a binary PPM."""

    if len(rgb) != width * height * 3:
        raise ValueError("pixel buffer does not match dimensions")
    return b"P6\n%d %d\n255\n" % (width, height) + bytes(rgb)


# --------------------------------------------------------------------------
# Palette helpers
# --------------------------------------------------------------------------


def distinct_colors(frames: Sequence[bytes]) -> int:
    """Count distinct RGB triples across every frame."""

    return len(build_palette(frames)[0])


def build_palette(frames: Sequence[bytes]) -> Tuple[List[bytes], Dict[bytes, int]]:
    """Build a global palette over ``frames``, in first-appearance order."""

    palette: List[bytes] = []
    lookup: Dict[bytes, int] = {}
    for frame in frames:
        for off in range(0, len(frame), 3):
            color = frame[off : off + 3]
            if color not in lookup:
                lookup[color] = len(palette)
                palette.append(color)
    return palette, lookup


def _index_frame(frame: bytes, lookup: Dict[bytes, int]) -> bytes:
    return bytes(lookup[frame[off : off + 3]] for off in range(0, len(frame), 3))


# --------------------------------------------------------------------------
# PNG
# --------------------------------------------------------------------------


def _png_chunk(kind: bytes, payload: bytes) -> bytes:
    return (
        struct.pack(">I", len(payload))
        + kind
        + payload
        + struct.pack(">I", zlib.crc32(kind + payload) & 0xFFFFFFFF)
    )


def _paeth(a: int, b: int, c: int) -> int:
    p = a + b - c
    pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    if pb <= pc:
        return b
    return c


def _filter_scanlines(raw: bytes, stride: int, bpp: int) -> bytes:
    """Apply PNG's adaptive filtering, picking the minimum-sum filter per row."""

    out = bytearray()
    prior = bytes(stride)
    for start in range(0, len(raw), stride):
        line = raw[start : start + stride]
        candidates = []

        none = line
        candidates.append((0, none))

        sub = bytearray(stride)
        for i in range(stride):
            left = line[i - bpp] if i >= bpp else 0
            sub[i] = (line[i] - left) & 0xFF
        candidates.append((1, sub))

        up = bytearray(stride)
        for i in range(stride):
            up[i] = (line[i] - prior[i]) & 0xFF
        candidates.append((2, up))

        avg = bytearray(stride)
        for i in range(stride):
            left = line[i - bpp] if i >= bpp else 0
            avg[i] = (line[i] - ((left + prior[i]) >> 1)) & 0xFF
        candidates.append((3, avg))

        paeth = bytearray(stride)
        for i in range(stride):
            left = line[i - bpp] if i >= bpp else 0
            upleft = prior[i - bpp] if i >= bpp else 0
            paeth[i] = (line[i] - _paeth(left, prior[i], upleft)) & 0xFF
        candidates.append((4, paeth))

        # The heuristic from the PNG spec: minimum sum of absolute differences,
        # treating filtered bytes as signed.
        def cost(candidate: Tuple[int, bytes]) -> int:
            return sum(b if b < 128 else 256 - b for b in candidate[1])

        best_type, best = min(candidates, key=cost)
        out.append(best_type)
        out += best
        prior = line
    return bytes(out)


def encode_png(width: int, height: int, rgb: bytes, level: int = 9) -> bytes:
    """Encode 8-bit truecolor PNG (color type 2)."""

    raw = _filter_scanlines(rgb, width * 3, 3)
    return (
        b"\x89PNG\r\n\x1a\n"
        + _png_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
        + _png_chunk(b"IDAT", zlib.compress(raw, level))
        + _png_chunk(b"IEND", b"")
    )


def encode_png_indexed(
    width: int, height: int, rgb: bytes, level: int = 9
) -> bytes:
    """Encode an indexed PNG (color type 3).  Requires at most 256 colors."""

    palette, lookup = build_palette([rgb])
    if len(palette) > 256:
        raise ValueError("indexed PNG needs 256 colors or fewer")
    indices = _index_frame(rgb, lookup)
    raw = _filter_scanlines(indices, width, 1)
    return (
        b"\x89PNG\r\n\x1a\n"
        + _png_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 3, 0, 0, 0))
        + _png_chunk(b"PLTE", b"".join(palette))
        + _png_chunk(b"IDAT", zlib.compress(raw, level))
        + _png_chunk(b"IEND", b"")
    )


def _png_chunks(data: bytes):
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("not a PNG file")
    pos = 8
    while pos + 8 <= len(data):
        (length,) = struct.unpack(">I", data[pos : pos + 4])
        kind = data[pos + 4 : pos + 8]
        payload = data[pos + 8 : pos + 8 + length]
        (stored_crc,) = struct.unpack(">I", data[pos + 8 + length : pos + 12 + length])
        if stored_crc != (zlib.crc32(kind + payload) & 0xFFFFFFFF):
            raise ValueError(f"PNG chunk {kind!r} fails its CRC")
        yield kind, payload
        pos += 12 + length


def decode_png(data: bytes) -> Tuple[int, int, bytes]:
    """Decode the 8-bit PNG subset written above back to ``(w, h, rgb)``."""

    width = height = color_type = 0
    palette: List[bytes] = []
    idat = bytearray()
    for kind, payload in _png_chunks(data):
        if kind == b"IHDR":
            width, height, depth, color_type, comp, filt, interlace = struct.unpack(
                ">IIBBBBB", payload
            )
            if depth != 8 or comp != 0 or filt != 0 or interlace != 0:
                raise ValueError("unsupported PNG variant")
            if color_type not in (2, 3):
                raise ValueError(f"unsupported PNG color type {color_type}")
        elif kind == b"PLTE":
            palette = [payload[i : i + 3] for i in range(0, len(payload), 3)]
        elif kind == b"IDAT":
            idat += payload
        elif kind == b"IEND":
            break

    bpp = 3 if color_type == 2 else 1
    stride = width * bpp
    raw = zlib.decompress(bytes(idat))
    out = bytearray()
    prior = bytearray(stride)
    pos = 0
    for _ in range(height):
        filter_type = raw[pos]
        line = bytearray(raw[pos + 1 : pos + 1 + stride])
        pos += 1 + stride
        for i in range(stride):
            left = line[i - bpp] if i >= bpp else 0
            upleft = prior[i - bpp] if i >= bpp else 0
            if filter_type == 0:
                value = line[i]
            elif filter_type == 1:
                value = line[i] + left
            elif filter_type == 2:
                value = line[i] + prior[i]
            elif filter_type == 3:
                value = line[i] + ((left + prior[i]) >> 1)
            elif filter_type == 4:
                value = line[i] + _paeth(left, prior[i], upleft)
            else:
                raise ValueError(f"unknown PNG filter {filter_type}")
            line[i] = value & 0xFF
        out += line
        prior = line

    if color_type == 2:
        return width, height, bytes(out)
    return width, height, b"".join(palette[i] for i in out)


# --------------------------------------------------------------------------
# GIF89a
# --------------------------------------------------------------------------

_GIF_MAX_CODE_SIZE = 12


def _lzw_compress(indices: bytes, min_code_size: int) -> bytes:
    """GIF variable-width LZW, packed LSB-first.

    The width bookkeeping follows the order established encoders use: a code is
    emitted at the current width, the width is grown only once the next code to
    be assigned no longer fits, and the new string is added afterwards.  This
    keeps the emitted stream in lockstep with decoders, whose string table
    always lags one entry behind the encoder's.
    """

    clear_code = 1 << min_code_size
    end_code = clear_code + 1
    code_size = min_code_size + 1
    table: Dict[bytes, int] = {bytes([i]): i for i in range(clear_code)}
    next_code = clear_code + 2

    bits = 0
    nbits = 0
    out = bytearray()

    def emit(code: int, width: int) -> None:
        nonlocal bits, nbits
        bits |= code << nbits
        nbits += width
        while nbits >= 8:
            out.append(bits & 0xFF)
            bits >>= 8
            nbits -= 8

    emit(clear_code, code_size)
    prefix = b""
    for byte in indices:
        candidate = prefix + bytes([byte])
        if candidate in table:
            prefix = candidate
            continue
        emit(table[prefix], code_size)
        while next_code >= (1 << code_size) and code_size < _GIF_MAX_CODE_SIZE:
            code_size += 1
        if next_code >= (1 << _GIF_MAX_CODE_SIZE) - 1:
            emit(clear_code, code_size)
            table = {bytes([i]): i for i in range(clear_code)}
            next_code = clear_code + 2
            code_size = min_code_size + 1
        else:
            table[candidate] = next_code
            next_code += 1
        prefix = bytes([byte])
    if prefix:
        emit(table[prefix], code_size)
    while next_code >= (1 << code_size) and code_size < _GIF_MAX_CODE_SIZE:
        code_size += 1
    emit(end_code, code_size)
    if nbits:
        out.append(bits & 0xFF)
    return bytes(out)


def _lzw_decompress(data: bytes, min_code_size: int) -> bytes:
    clear_code = 1 << min_code_size
    end_code = clear_code + 1
    code_size = min_code_size + 1
    table: List[bytes] = [bytes([i]) for i in range(clear_code)] + [b"", b""]
    out = bytearray()
    previous: bytes = b""

    bits = 0
    nbits = 0
    pos = 0
    while True:
        while nbits < code_size and pos < len(data):
            bits |= data[pos] << nbits
            nbits += 8
            pos += 1
        if nbits < code_size:
            break
        code = bits & ((1 << code_size) - 1)
        bits >>= code_size
        nbits -= code_size

        if code == clear_code:
            table = [bytes([i]) for i in range(clear_code)] + [b"", b""]
            code_size = min_code_size + 1
            previous = b""
            continue
        if code == end_code:
            break
        if code < len(table):
            entry = table[code]
        elif code == len(table) and previous:
            entry = previous + previous[:1]
        else:
            raise ValueError("corrupt LZW stream")
        out += entry
        if previous:
            table.append(previous + entry[:1])
            if len(table) == (1 << code_size) and code_size < _GIF_MAX_CODE_SIZE:
                code_size += 1
        previous = entry
    return bytes(out)


def _sub_blocks(data: bytes) -> bytes:
    out = bytearray()
    for start in range(0, len(data), 255):
        chunk = data[start : start + 255]
        out.append(len(chunk))
        out += chunk
    out.append(0)
    return bytes(out)


def encode_gif(
    width: int, height: int, frames: Sequence[bytes], delay_cs: int = 4
) -> bytes:
    """Encode a GIF89a with a global color table shared by every frame.

    Raises :class:`ValueError` when the frames hold more than 256 distinct
    colors, which GIF simply cannot represent losslessly.
    """

    palette, lookup = build_palette(frames)
    if len(palette) > 256:
        raise ValueError(f"GIF needs 256 colors or fewer (got {len(palette)})")

    bits = max(1, (len(palette) - 1).bit_length())
    table_size = 1 << bits
    color_table = b"".join(palette) + bytes(3 * (table_size - len(palette)))
    min_code_size = max(2, bits)

    out = bytearray(b"GIF89a")
    out += struct.pack("<HH", width, height)
    out += bytes([0xF0 | (bits - 1), 0, 0])  # global table, 8-bit color res
    out += color_table

    if len(frames) > 1:
        out += b"\x21\xff\x0bNETSCAPE2.0\x03\x01\x00\x00\x00"  # loop forever

    for frame in frames:
        if len(frames) > 1:
            out += b"\x21\xf9\x04\x00" + struct.pack("<H", delay_cs) + b"\x00\x00"
        out += b"\x2c" + struct.pack("<HHHH", 0, 0, width, height) + b"\x00"
        out += bytes([min_code_size])
        out += _sub_blocks(_lzw_compress(_index_frame(frame, lookup), min_code_size))
    out += b"\x3b"
    return bytes(out)


def _read_sub_blocks(data: bytes, pos: int) -> Tuple[bytes, int]:
    chunks = bytearray()
    while True:
        size = data[pos]
        pos += 1
        if size == 0:
            return bytes(chunks), pos
        chunks += data[pos : pos + size]
        pos += size


def decode_gif(data: bytes) -> Tuple[int, int, List[bytes]]:
    """Decode a GIF written by :func:`encode_gif` back to RGB frames."""

    if data[:6] not in (b"GIF89a", b"GIF87a"):
        raise ValueError("not a GIF file")
    width, height, packed, _bg, _aspect = struct.unpack("<HHBBB", data[6:13])
    pos = 13
    palette: List[bytes] = []
    if packed & 0x80:
        count = 1 << ((packed & 0x07) + 1)
        palette = [data[pos + 3 * i : pos + 3 * i + 3] for i in range(count)]
        pos += 3 * count

    frames: List[bytes] = []
    while pos < len(data):
        marker = data[pos]
        pos += 1
        if marker == 0x3B:
            break
        if marker == 0x21:  # extension
            pos += 1  # label
            _payload, pos = _read_sub_blocks(data, pos)
            continue
        if marker != 0x2C:
            raise ValueError(f"unexpected GIF block 0x{marker:02x}")
        fx, fy, fw, fh, local = struct.unpack("<HHHHB", data[pos : pos + 9])
        pos += 9
        frame_palette = palette
        if local & 0x80:
            count = 1 << ((local & 0x07) + 1)
            frame_palette = [data[pos + 3 * i : pos + 3 * i + 3] for i in range(count)]
            pos += 3 * count
        min_code_size = data[pos]
        pos += 1
        payload, pos = _read_sub_blocks(data, pos)
        indices = _lzw_decompress(payload, min_code_size)
        if (fx, fy, fw, fh) != (0, 0, width, height):
            raise ValueError("only full-canvas GIF frames are supported")
        frames.append(b"".join(frame_palette[i] for i in indices[: fw * fh]))
    return width, height, frames


# --------------------------------------------------------------------------
# QOI (Quite OK Image, v1.0)
# --------------------------------------------------------------------------

_QOI_OP_INDEX = 0x00
_QOI_OP_DIFF = 0x40
_QOI_OP_LUMA = 0x80
_QOI_OP_RUN = 0xC0
_QOI_OP_RGB = 0xFE
_QOI_OP_RGBA = 0xFF


def _qoi_hash(r: int, g: int, b: int, a: int) -> int:
    return (r * 3 + g * 5 + b * 7 + a * 11) % 64


def encode_qoi(width: int, height: int, rgb: bytes) -> bytes:
    """Encode 8-bit RGB as QOI (3-channel, sRGB)."""

    out = bytearray(b"qoif")
    out += struct.pack(">IIBB", width, height, 3, 0)

    seen = [(0, 0, 0, 0)] * 64
    pr, pg, pb = 0, 0, 0  # QOI's initial previous pixel, alpha fixed at 255
    run = 0
    for off in range(0, width * height * 3, 3):
        r, g, b = rgb[off], rgb[off + 1], rgb[off + 2]
        a = 255
        if (r, g, b) == (pr, pg, pb):
            run += 1
            if run == 62:
                out.append(_QOI_OP_RUN | (run - 1))
                run = 0
            pr, pg, pb = r, g, b
            continue
        if run:
            out.append(_QOI_OP_RUN | (run - 1))
            run = 0
        slot = _qoi_hash(r, g, b, a)
        if seen[slot] == (r, g, b, a):
            out.append(_QOI_OP_INDEX | slot)
        else:
            seen[slot] = (r, g, b, a)
            dr, dg, db = r - pr, g - pg, b - pb
            if -2 <= dr <= 1 and -2 <= dg <= 1 and -2 <= db <= 1:
                out.append(_QOI_OP_DIFF | ((dr + 2) << 4) | ((dg + 2) << 2) | (db + 2))
            elif -32 <= dg <= 31 and -8 <= dr - dg <= 7 and -8 <= db - dg <= 7:
                out.append(_QOI_OP_LUMA | (dg + 32))
                out.append(((dr - dg + 8) << 4) | (db - dg + 8))
            else:
                out.append(_QOI_OP_RGB)
                out += bytes((r, g, b))
        pr, pg, pb = r, g, b
    if run:
        out.append(_QOI_OP_RUN | (run - 1))
    out += b"\x00" * 7 + b"\x01"
    return bytes(out)


def decode_qoi(data: bytes) -> Tuple[int, int, bytes]:
    """Decode a QOI stream back to ``(width, height, rgb)``."""

    if data[:4] != b"qoif":
        raise ValueError("not a QOI file")
    width, height, channels, _colorspace = struct.unpack(">IIBB", data[4:14])
    if channels not in (3, 4):
        raise ValueError(f"unsupported QOI channel count {channels}")

    seen = [(0, 0, 0, 0)] * 64
    r, g, b, a = 0, 0, 0, 255
    out = bytearray()
    pos = 14
    total = width * height
    while len(out) < total * 3:
        byte = data[pos]
        pos += 1
        if byte == _QOI_OP_RGB:
            r, g, b = data[pos], data[pos + 1], data[pos + 2]
            pos += 3
        elif byte == _QOI_OP_RGBA:
            r, g, b, a = data[pos], data[pos + 1], data[pos + 2], data[pos + 3]
            pos += 4
        elif byte & 0xC0 == _QOI_OP_INDEX:
            r, g, b, a = seen[byte & 0x3F]
        elif byte & 0xC0 == _QOI_OP_DIFF:
            r = (r + ((byte >> 4) & 0x03) - 2) & 0xFF
            g = (g + ((byte >> 2) & 0x03) - 2) & 0xFF
            b = (b + (byte & 0x03) - 2) & 0xFF
        elif byte & 0xC0 == _QOI_OP_LUMA:
            dg = (byte & 0x3F) - 32
            extra = data[pos]
            pos += 1
            r = (r + dg + ((extra >> 4) & 0x0F) - 8) & 0xFF
            g = (g + dg) & 0xFF
            b = (b + dg + (extra & 0x0F) - 8) & 0xFF
        else:  # QOI_OP_RUN
            run = (byte & 0x3F) + 1
            out += bytes((r, g, b)) * run
            seen[_qoi_hash(r, g, b, a)] = (r, g, b, a)
            continue
        seen[_qoi_hash(r, g, b, a)] = (r, g, b, a)
        out += bytes((r, g, b))
    return width, height, bytes(out[: total * 3])
