"""Reference encoders/decoders for the volumetric formats 4Splat competes with.

`tools/benchmark_volumes.py` compares `.4spl` volumes against the formats that
already store a 3D grid of voxels as one object, rather than as a pile of
per-slice image files.  As in :mod:`refcodecs`, each format here has both an
encoder and a decoder so a reported size can be shown to belong to a file that
decodes back to the source voxels.

* **NIfTI-1** (`.nii`, `.nii.gz`) - the neuroimaging container, and the closest
  existing analogue to 4Splat's header: one fixed-size struct describing an
  `(x, y, z, t)` grid followed by the voxels.  Written here as `DT_RGB24`.
* **NRRD** - the ITK/3D Slicer format: a text header, then the voxels `raw` or
  `gzip`-encoded.  RGB is the usual "one axis of size 3, kind RGB-color".
* **MetaImage** (`.mha`) - ITK's other format: text header plus voxels, with
  `CompressedData` selecting zlib.
* **TIFF stack** - the microscopy standard (an ImageJ "stack"): one IFD per
  slice in a single file.  Three variants are offered - uncompressed RGB,
  Deflate RGB, and palette-color slices sharing one global `ColorMap`, which is
  the closest thing in a mainstream format to what 4Splat does.

Volumes are passed as ``(width, height, slices)`` where ``slices`` is a list of
``width * height * 3`` byte strings, ordered from z=0 up - the same slice order
the 4Splat CLI's ``encode-volume`` takes.
"""

from __future__ import annotations

import gzip
import struct
import zlib
from typing import Dict, List, Sequence, Tuple

from refcodecs import build_palette

__all__ = [
    "encode_nifti",
    "decode_nifti",
    "encode_nifti_gz",
    "decode_nifti_gz",
    "encode_nrrd",
    "decode_nrrd",
    "encode_metaimage",
    "decode_metaimage",
    "encode_tiff_stack",
    "encode_tiff_stack_palette",
    "decode_tiff_stack",
]

_DT_RGB24 = 128  # NIfTI datatype code for packed 8-bit RGB


def _voxels(slices: Sequence[bytes]) -> bytes:
    return b"".join(slices)


def _unslice(data: bytes, width: int, height: int, depth: int) -> List[bytes]:
    stride = width * height * 3
    if len(data) < stride * depth:
        raise ValueError("volume payload is shorter than its header claims")
    return [data[z * stride : (z + 1) * stride] for z in range(depth)]


# --------------------------------------------------------------------------
# NIfTI-1
# --------------------------------------------------------------------------


def encode_nifti(width: int, height: int, slices: Sequence[bytes]) -> bytes:
    """Write a single-file NIfTI-1 volume (`n+1`, RGB24, one 348-byte header)."""

    depth = len(slices)
    header = bytearray(348)
    struct.pack_into("<i", header, 0, 348)  # sizeof_hdr
    # dim[0] = number of used dimensions, then x, y, z, t, ...
    struct.pack_into("<8h", header, 40, 3, width, height, depth, 1, 1, 1, 1)
    struct.pack_into("<h", header, 70, _DT_RGB24)  # datatype
    struct.pack_into("<h", header, 72, 24)  # bitpix
    # pixdim[0..7]: qfac then the voxel size on each axis.
    struct.pack_into("<8f", header, 76, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0)
    struct.pack_into("<f", header, 108, 352.0)  # vox_offset
    struct.pack_into("<f", header, 112, 1.0)  # scl_slope
    struct.pack_into("<h", header, 252, 1)  # qform_code: scanner anatomical
    struct.pack_into("<h", header, 254, 1)  # sform_code
    header[344:348] = b"n+1\x00"  # magic for the single-file form
    return bytes(header) + b"\x00" * 4 + _voxels(slices)


def decode_nifti(data: bytes) -> Tuple[int, int, List[bytes]]:
    (sizeof_hdr,) = struct.unpack_from("<i", data, 0)
    if sizeof_hdr != 348 or data[344:348] not in (b"n+1\x00", b"ni1\x00"):
        raise ValueError("not a NIfTI-1 volume")
    dims = struct.unpack_from("<8h", data, 40)
    (datatype,) = struct.unpack_from("<h", data, 70)
    if datatype != _DT_RGB24:
        raise ValueError(f"unsupported NIfTI datatype {datatype}")
    (vox_offset,) = struct.unpack_from("<f", data, 108)
    width, height, depth = dims[1], dims[2], dims[3]
    return width, height, _unslice(data[int(vox_offset) :], width, height, depth)


def encode_nifti_gz(width: int, height: int, slices: Sequence[bytes]) -> bytes:
    """The `.nii.gz` form nearly every NIfTI in the wild is actually stored as."""

    return gzip.compress(encode_nifti(width, height, slices), 9, mtime=0)


def decode_nifti_gz(data: bytes) -> Tuple[int, int, List[bytes]]:
    return decode_nifti(gzip.decompress(data))


# --------------------------------------------------------------------------
# NRRD
# --------------------------------------------------------------------------


def encode_nrrd(
    width: int, height: int, slices: Sequence[bytes], encoding: str = "gzip"
) -> bytes:
    """Write a detached-header-free NRRD, `raw` or `gzip` encoded."""

    if encoding not in ("raw", "gzip"):
        raise ValueError(f"unsupported NRRD encoding {encoding!r}")
    payload = _voxels(slices)
    if encoding == "gzip":
        payload = gzip.compress(payload, 9, mtime=0)
    header = (
        "NRRD0004\n"
        "# Complete NRRD file format specification at:\n"
        "# http://teem.sourceforge.net/nrrd/format.html\n"
        "type: unsigned char\n"
        "dimension: 4\n"
        f"sizes: 3 {width} {height} {len(slices)}\n"
        "kinds: RGB-color space space space\n"
        "endian: little\n"
        f"encoding: {encoding}\n"
        "space dimension: 3\n"
        "\n"
    )
    return header.encode("ascii") + payload


def decode_nrrd(data: bytes) -> Tuple[int, int, List[bytes]]:
    split = data.find(b"\n\n")
    if not data.startswith(b"NRRD") or split < 0:
        raise ValueError("not an NRRD file")
    fields: Dict[str, str] = {}
    for line in data[:split].decode("ascii").splitlines():
        if line.startswith("#") or ":" not in line:
            continue
        key, _, value = line.partition(":")
        fields[key.strip()] = value.strip()
    sizes = [int(v) for v in fields["sizes"].split()]
    if sizes[0] != 3:
        raise ValueError("expected an RGB-color axis of size 3")
    _, width, height, depth = sizes
    payload = data[split + 2 :]
    if fields.get("encoding") == "gzip":
        payload = gzip.decompress(payload)
    return width, height, _unslice(payload, width, height, depth)


# --------------------------------------------------------------------------
# MetaImage (.mha)
# --------------------------------------------------------------------------


def encode_metaimage(
    width: int, height: int, slices: Sequence[bytes], compress: bool = True
) -> bytes:
    """Write a single-file MetaImage volume, optionally zlib-compressed."""

    payload = _voxels(slices)
    body = zlib.compress(payload, 9) if compress else payload
    lines = [
        "ObjectType = Image",
        "NDims = 3",
        "BinaryData = True",
        "BinaryDataByteOrderMSB = False",
        f"CompressedData = {'True' if compress else 'False'}",
    ]
    if compress:
        lines.append(f"CompressedDataSize = {len(body)}")
    lines += [
        "TransformMatrix = 1 0 0 0 1 0 0 0 1",
        "Offset = 0 0 0",
        "ElementSpacing = 1 1 1",
        f"DimSize = {width} {height} {len(slices)}",
        "ElementNumberOfChannels = 3",
        "ElementType = MET_UCHAR",
        "ElementDataFile = LOCAL",
        "",
    ]
    return "\n".join(lines).encode("ascii") + body


def decode_metaimage(data: bytes) -> Tuple[int, int, List[bytes]]:
    marker = b"ElementDataFile = LOCAL\n"
    start = data.find(marker)
    if not data.startswith(b"ObjectType = Image") or start < 0:
        raise ValueError("not a local MetaImage volume")
    fields: Dict[str, str] = {}
    for line in data[:start].decode("ascii").splitlines():
        key, _, value = line.partition(" = ")
        fields[key.strip()] = value.strip()
    width, height, depth = (int(v) for v in fields["DimSize"].split())
    payload = data[start + len(marker) :]
    if fields.get("CompressedData") == "True":
        payload = zlib.decompress(payload)
    return width, height, _unslice(payload, width, height, depth)


# --------------------------------------------------------------------------
# TIFF stacks
# --------------------------------------------------------------------------

_TIFF_TAGS = {
    "ImageWidth": 256,
    "ImageLength": 257,
    "BitsPerSample": 258,
    "Compression": 259,
    "PhotometricInterpretation": 262,
    "StripOffsets": 273,
    "SamplesPerPixel": 277,
    "RowsPerStrip": 278,
    "StripByteCounts": 279,
    "ColorMap": 320,
}
_TIFF_SHORT = 3
_TIFF_LONG = 4


def _tiff_build(
    width: int,
    height: int,
    pages: Sequence[bytes],
    *,
    compression: int,
    photometric: int,
    samples: int,
    color_map: bytes = b"",
) -> bytes:
    """Assemble a multi-page TIFF from already-encoded strip payloads.

    Layout: header, then the blocks every page shares (BitsPerSample, and the
    ColorMap for palette-color stacks), then each page's IFD followed by its
    single strip.  Every IFD points its ColorMap tag at the one shared block -
    legal TIFF, and the point of the comparison: the palette is stored once for
    the whole stack, not copied into all N slices.
    """

    bits = struct.pack("<3H", 8, 8, 8) if samples == 3 else b""

    entries_per_page = 9 + (1 if color_map else 0)
    ifd_bytes = 2 + 12 * entries_per_page + 4

    out = bytearray(b"II" + struct.pack("<HI", 42, 0))
    bits_offset = len(out)
    out += bits
    map_offset = len(out)
    out += color_map

    # Where each page's IFD and strip will land, now that the shared blocks
    # have fixed the starting offset.
    offset = len(out)
    layout = []
    for page in pages:
        layout.append({"ifd": offset, "strip": offset + ifd_bytes, "size": len(page)})
        offset += ifd_bytes + len(page)
    struct.pack_into("<I", out, 4, layout[0]["ifd"])

    for index, page in enumerate(pages):
        spot = layout[index]
        entries: List[Tuple[int, int, int, int]] = [
            (_TIFF_TAGS["ImageWidth"], _TIFF_LONG, 1, width),
            (_TIFF_TAGS["ImageLength"], _TIFF_LONG, 1, height),
            (
                _TIFF_TAGS["BitsPerSample"],
                _TIFF_SHORT,
                samples,
                bits_offset if samples == 3 else 8,
            ),
            (_TIFF_TAGS["Compression"], _TIFF_SHORT, 1, compression),
            (_TIFF_TAGS["PhotometricInterpretation"], _TIFF_SHORT, 1, photometric),
            (_TIFF_TAGS["StripOffsets"], _TIFF_LONG, 1, spot["strip"]),
            (_TIFF_TAGS["SamplesPerPixel"], _TIFF_SHORT, 1, samples),
            (_TIFF_TAGS["RowsPerStrip"], _TIFF_LONG, 1, height),
            (_TIFF_TAGS["StripByteCounts"], _TIFF_LONG, 1, spot["size"]),
        ]
        if color_map:
            entries.append(
                (_TIFF_TAGS["ColorMap"], _TIFF_SHORT, len(color_map) // 2, map_offset)
            )
        entries.sort(key=lambda e: e[0])  # TIFF requires ascending tag order

        assert len(out) == spot["ifd"]
        out += struct.pack("<H", len(entries))
        for tag, kind, count, value in entries:
            payload = (
                struct.pack("<H2x", value)
                if kind == _TIFF_SHORT and count == 1
                else struct.pack("<I", value)
            )
            out += struct.pack("<HHI", tag, kind, count) + payload
        next_ifd = layout[index + 1]["ifd"] if index + 1 < len(layout) else 0
        out += struct.pack("<I", next_ifd)

        assert len(out) == spot["strip"]
        out += page
    return bytes(out)


def encode_tiff_stack(
    width: int, height: int, slices: Sequence[bytes], compress: bool = True
) -> bytes:
    """One RGB page per slice, uncompressed or Deflate (TIFF compression 8)."""

    pages = [zlib.compress(s, 9) if compress else bytes(s) for s in slices]
    return _tiff_build(
        width,
        height,
        pages,
        compression=8 if compress else 1,
        photometric=2,  # RGB
        samples=3,
    )


def encode_tiff_stack_palette(
    width: int, height: int, slices: Sequence[bytes], compress: bool = True
) -> bytes:
    """Palette-color pages sharing one ColorMap - TIFF's global-palette form."""

    palette, lookup = build_palette(slices)
    if len(palette) > 256:
        raise ValueError(f"palette TIFF needs 256 colors or fewer (got {len(palette)})")
    # ColorMap is 3 * 2^bits 16-bit values: all reds, then greens, then blues.
    channels = [[], [], []]
    for entry in palette:
        for c in range(3):
            channels[c].append(entry[c] * 257)
    for c in range(3):
        channels[c] += [0] * (256 - len(palette))
    color_map = struct.pack("<768H", *(channels[0] + channels[1] + channels[2]))

    pages = []
    for slice_ in slices:
        indexed = bytes(
            lookup[slice_[off : off + 3]] for off in range(0, len(slice_), 3)
        )
        pages.append(zlib.compress(indexed, 9) if compress else indexed)
    return _tiff_build(
        width,
        height,
        pages,
        compression=8 if compress else 1,
        photometric=3,  # palette color
        samples=1,
        color_map=color_map,
    )


def decode_tiff_stack(data: bytes) -> Tuple[int, int, List[bytes]]:
    """Decode the multi-page TIFFs written above back to RGB slices."""

    if data[:2] != b"II":
        raise ValueError("only little-endian TIFF is supported")
    magic, first_ifd = struct.unpack_from("<HI", data, 2)
    if magic != 42:
        raise ValueError("not a TIFF file")

    def read_values(kind: int, count: int, raw: bytes) -> List[int]:
        size = 2 if kind == _TIFF_SHORT else 4
        if count * size <= 4:
            payload = raw[: count * size]
        else:
            (offset,) = struct.unpack("<I", raw)
            payload = data[offset : offset + count * size]
        fmt = "<%d%s" % (count, "H" if kind == _TIFF_SHORT else "I")
        return list(struct.unpack(fmt, payload))

    width = height = 0
    slices: List[bytes] = []
    offset = first_ifd
    while offset:
        (count,) = struct.unpack_from("<H", data, offset)
        tags: Dict[int, List[int]] = {}
        for i in range(count):
            entry = offset + 2 + 12 * i
            tag, kind, n = struct.unpack_from("<HHI", data, entry)
            tags[tag] = read_values(kind, n, data[entry + 8 : entry + 12])
        (offset,) = struct.unpack_from("<I", data, offset + 2 + 12 * count)

        width = tags[_TIFF_TAGS["ImageWidth"]][0]
        height = tags[_TIFF_TAGS["ImageLength"]][0]
        compression = tags[_TIFF_TAGS["Compression"]][0]
        photometric = tags[_TIFF_TAGS["PhotometricInterpretation"]][0]
        start = tags[_TIFF_TAGS["StripOffsets"]][0]
        length = tags[_TIFF_TAGS["StripByteCounts"]][0]
        strip = data[start : start + length]
        if compression == 8:
            strip = zlib.decompress(strip)
        elif compression != 1:
            raise ValueError(f"unsupported TIFF compression {compression}")

        if photometric == 2:
            slices.append(strip)
        elif photometric == 3:
            color_map = tags[_TIFF_TAGS["ColorMap"]]
            third = len(color_map) // 3
            palette = [
                bytes(
                    (
                        color_map[i] // 257,
                        color_map[third + i] // 257,
                        color_map[2 * third + i] // 257,
                    )
                )
                for i in range(third)
            ]
            slices.append(b"".join(palette[i] for i in strip))
        else:
            raise ValueError(f"unsupported TIFF photometric {photometric}")
    return width, height, slices
