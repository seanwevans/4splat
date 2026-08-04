/*╭ .4spl File Format ╮
╭─╯                   ╰───────────────────────────────────────────────────╮
│ The 4Splat Codec is a palette-based, lossless video compression format. │
│ It generalizes indexed-color images to 3D and 4D spatial-temporal data: │
│ (x, y, z, frame). Pixels across all frames are compressed into the      │
│ global color palette, and each pixel stores an index into this          │
│ palette. All values are little-endian and indices are row-major order:  │
│ t-major -> z-major -> y-major -> x-major.                               │
╰─────────────────────────────────────────────────────────────────────────╯

  ╭ Binary Layout ╮
╭─╯▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒╰───────────────────────────────────────────────────────╮
│▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
│▒▒╭ Header ╮ ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
│▒╭╯        ╰───────────────────────────────────────────────────────────╮▒│
│▒│ name         size        encoding    value      hex                 │▒│
│▒├─────────────────────────────────────────────────────────────────────┤▒│
│▒│ magic        4 bytes     ASCII       "4SPL"     0x34 0x53 0x50 0x4C │▒│
│▒│ version      4 bytes     uint8[4]    {1,0,0,0}  0x01 0x00 0x00 0x00 │▒│
│▒│ width        4 bytes     uint32      Width                          │▒│
│▒│ height       4 bytes     uint32      Height                         │▒│
│▒│ depth        4 bytes     uint32      Depth                          │▒│
│▒│ frames       4 bytes     uint32      Frames                         │▒│
│▒│ paletteSize  4 bytes     uint32      PSize                          │▒│
│▒│ flags        4 bytes     bitmask     Flags                          │▒│
│▒╰─────────────────────────────────────────────────────────────────────╯▒│
│▒▒▒▒▒▒▒▒▒▒▒▒▒▒ 32 bytes ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
│▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
│▒▒╭ Palette ╮▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
│▒╭╯         ╰──────────────────────────────────────────────────────────╮▒│
│▒│              ╭ Splat4D PSize-1 ╮                                    │▒│
│▒│           ╭ ─ ... ─ ─ ╮        ╰────────────────────────────────────┤▒│
│▒│        ╭ Splat4D 2 ╮  ╰ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ╮ │▒│
│▒│     ╭ Splat4D 1 ╮  ╰───────────────────────────────────────────╮    │▒│
│▒│  ╭ Splat4D 0 ╮  ╰───────────────────────────────────────────╮  │  │ │▒│
│▒│ ╭╯           ╰───────────────────────────────────────────╮  │  │    │▒│
│▒│ │ name         size        encoding   purpose            │  │  │  │ │▒│
│▒│ ├────────────────────────────────────────────────────────┤  │  │    │▒│
│▒│ │ mu_x         4 bytes     float      mean position in x │  │  │  │ │▒│
│▒│ │ mu_y         4 bytes     float      mean position in y │  │  │    │▒│
│▒│ │ mu_z         4 bytes     float      mean position in z │  │  │  │ │▒│
│▒│ │ sigma_x      4 bytes     float      std dev in x       │  │  │    │▒│
│▒│ │ sigma_y      4 bytes     float      std dev in y       │  │  │  │ │▒│
│▒│ │ sigma_z      4 bytes     float      std dev in z       │  │  │    │▒│
│▒│ │ mu_t         4 bytes     float      mean position in t │  │  │  │ │▒│
│▒│ │ sigma_t      4 bytes     float      std dev in t       │  │  │    │▒│
│▒│ │ r            4 bytes     float      red intensity      │  │  │  ├─┤▒│
│▒│ │ g            4 bytes     float      green intensity    │  │  ├─ ╯ │▒│
│▒│ │ b            4 bytes     float      blue intensity     │  ├──╯    │▒│
│▒│ │ alpha        4 bytes     float  ▲   opacity            ├──╯       │▒│
│▒│ ╰─────────────────────────────────╂──────────────────────╯          │▒│
│▒│               48 bytes        references                            │▒│
│▒╰───────────────────────────────────╂─────────────────────────────────╯▒│
│▒▒▒▒▒▒▒▒▒▒ 48*PSize bytes ▒▒▒▒▒▒▒▒▒▒▒┃▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
│▒▒╭ Index ╮▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒┃▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
│▒╭╯       ╰──────────────────────────╂─────────────────────────────────╮▒│
│▒│  ╭───╮╭───╮╭───╮╭───╮╭───╮╭───╮╭──╂╮╭───╮╭───╮╭───╮     ╭───╮╭───╮  │▒│
│▒│  │ 0 ││ 1 ││ 2 ││ 3 ││ 4 ││ 5 ││ 6┛││ 7 ││ 8 ││ 9 │ ... │I-2││I-1│  │▒│
│▒│  ╰───╯╰───╯╰───╯╰───╯╰───╯╰───╯╰───╯╰───╯╰───╯╰───╯     ╰───╯╰───╯  │▒│
│▒╰─────────────────────────────────────────────────────────────────────╯▒│
│▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ 8*I bytes ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
│▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
│▒▒╭ Footer ╮▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
│▒╭╯        ╰───────────────────────────────────────────────────────────╮▒│
│▒│ name        size        encoding    value      hex                  │▒│
│▒├─────────────────────────────────────────────────────────────────────┤▒│
│▒│ idxoffset   8 bytes     uint64                                      │▒│
│▒│ checksum    4 bytes     CRC32                                       │▒│
│▒│ end         4 bytes     ASCII       "LPS4"     0x4C 0x50 0x53 0x34  │▒│
│▒╰─────────────────────────────────────────────────────────────────────╯▒│
│▒▒▒▒▒▒▒▒▒▒▒▒▒ 16 bytes ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
│▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
╰─────────────────────────────────────────────────────────────────────────╯

  ╭ flags ╮
 ╭╯▒▒▒▒▒▒▒╰──────────────────────────────────────────────────────────────╮
 │▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
 │▒▒▒▒▒ 00 ▒▒▒▒ 01 ▒▒▒▒ 02 ▒▒▒▒ 03 ▒▒▒▒ 04 ▒▒▒▒ 05 ▒▒▒▒ 06 ▒▒▒▒ 07 ▒▒▒▒▒▒│
 │▒▒▒▒╭───────┬───────┬───────────────┬───────────────────────────────╮▒▒│
 │ 00 │ ENDIAN│ SORTED│ PRECISION     │ COMPRESSION SCHEME            │▒▒│
 │▒▒▒▒├───────┴───────┼───────────────┼───────────────────────────────┤▒▒│
 │ 01 │ INDEX WIDTH   │ SPLAT SHAPE   │ COLOR SPACE                   │▒▒│
 │▒▒▒▒├───────────────┴───────────────┼───────────────────────────────┤▒▒│
 │ 02 │ INTERPOLATION TYPE            │ ENCRYPTION TYPE               │▒▒│
 │▒▒▒▒├───────────────────────────────┴───────────────────────────────┤▒▒│
 │ 03 │ METADATA                                                      │▒▒│
 │▒▒▒▒╰───────────────────────────────────────────────────────────────╯▒▒│
 │▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
 │▒ Offset   Size    Flag            Bin Description                    ▒│
 │▒ ──────────────────────────────────────────────────────────────────  ▒│
 │▒ 0x00     1       ENDIAN            0 Little-Endan                   ▒│
 │▒                                    1 Big-Endian                     ▒│
 │▒ 0x01     1       SORTED            0 No                             ▒│
 │▒                                    1 Yes                            ▒│
 │▒ 0x02     2       PRECISION        00 Float16                        ▒│
 │▒                                   01 Float32                        ▒│
 │▒                                   10 Float64                        ▒│
 │▒                                   11 Float128                       ▒│
 │▒ 0x04     4       COMPRESSION    0000 None                           ▒│
 │▒                                 0001 Run Length Encoding            ▒│
 │▒                                 0010 DEFLATE           (Katz, 1993) ▒│
 │▒                                 0011 RAR             (Roshal, 1993) ▒│
 │▒                                 0100 LZO          (Oberhumer, 1994) ▒│
 │▒                                 0101 Zlib      Gaily & Adler, 1995) ▒│
 │▒                                 0110 bzip2           (Seward, 1996) ▒│
 │▒                                 0111 LZMA            (Pavlov, 1998) ▒│
 │▒                                 1000 ZPAQ           (Mahoney, 2008) ▒│
 │▒                                 1001 XZ              (Pavlov, 2009) ▒│
 │▒                                 1010 LZ4             (Collet, 2011) ▒│
 │▒                                 1011 Snappy          (Google, 2011) ▒│
 │▒                                 1100 LZHAM        (Geldreich, 2013) ▒│
 │▒                                 1101 Brotli          (Google, 2015) ▒│
 │▒                                 1110 LZFSE            (Apple, 2015) ▒│
 │▒                                 1111 Zstd          (Facebook, 2016) ▒│
 │▒ 0x08     2       INDEX WIDTH      00 1-byte                         ▒│
 │▒                                   01 2-byte                         ▒│
 │▒                                   10 4-byte                         ▒│
 │▒                                   11 8-byte                         ▒│
 │▒ 0x0A     2       SPLAT SHAPE      00 Isotropic (1 σ)                ▒│
 │▒                                   01 Axis-Aligned (3 σ)             ▒│
 │▒                                   10 Full Covariance (6 vals)       ▒│
 │▒                                   11 RESERVED                       ▒│
 │▒ 0x0C     4       COLOR SPACE    0000 sRGB          (IEC 61966-2-1)  ▒│
 │▒                                 0001 Linear sRGB   (IEC 61966-2-1)  ▒│
 │▒                                 0010 OKLab         (Ottosson 2020)  ▒│
 │▒                                 0011 Display P3  (DCI-P3 / P3-D65)  ▒│
 │▒                                 0100 Rec.709        (ITU-R BT.709)  ▒│
 │▒                                 0101 Rec.2020      (ITU-R BT.2020)  ▒│
 │▒                                 0110 DCI-P3       (SMPTE RP 431-2)  ▒│
 │▒                                 0111 ACES-AP0    (SMPTE ST 2065-1)  ▒│
 │▒                                 1000 ProPhoto RGB (Kodak ROMM RGB)  ▒│
 │▒                                 1001 Rec.2100      (ITU-R BT.2100)  ▒│
 │▒                                 1010 CIE Lab     (CIE 1976 L*a*b*)  ▒│
 │▒                                 1011 CIE XYZ D65     (ISO 11664-1)  ▒│
 │▒                                 1100 ACEScg-AP1  (SMPTE ST 2065-1)  ▒│
 │▒                                 1101 Rec.601      (ITU-R BT.601-7)  ▒│
 │▒                                 1110 XYZ D50         (ISO 11664-1)  ▒│
 │▒                                 1111 XYZ D65         (ISO 11664-1)  ▒│
 │▒ 0x20     4       INTERPOLATION  0000 None                           ▒│
 │▒                                 0001 Nearest Neighbor               ▒│
 │▒                                 0010 Axis-align (lin/bi/tri/quad)   ▒│
 │▒                                 0011 Smooth (cubic/bi/tri/quad)     ▒│
 │▒                                 0100 Lanczos                        ▒│
 │▒                                 0101 Gaussian                       ▒│
 │▒                                 0110 Catmull-Rom                    ▒│
 │▒                                 0111 NURBS                          ▒│
 │▒                                 1000 Radial Basis Functions         ▒│
 │▒                                 1001 Optical Flow                   ▒│
 │▒                                 1010 Neural                         ▒│
 │▒                                 1011 Akima Splines                  ▒│
 │▒                                 1100 Inverse Distace                ▒│
 │▒                                 1101 Fourier                        ▒│
 │▒                                 1110 Moving Least Squares           ▒│
 │▒                                 1111 Cubic Hermite                  ▒│
 │▒ 0x24     8       METADATA   ???????? Anything                       ▒│
 │▒▒▒▒▒▒▒▒▒ 4 bytes ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
 │▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
 ╰───────────────────────────────────────────────────────────────────────╯*/

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Optional compression backends.
 *
 * The reference codec is self-contained and builds with a bare `gcc 4splat.c`;
 * in that configuration only the None and RLE schemes are available. Mature
 * third-party libraries provide the remaining compression schemes from the
 * format spec and are enabled at compile time (see the Makefile). Defining
 * SPLAT_WITH_ALL turns on every backend the toolchain can link. */
#ifdef SPLAT_WITH_ALL
#define SPLAT_WITH_ZLIB
#define SPLAT_WITH_BZIP2
#define SPLAT_WITH_LZMA
#define SPLAT_WITH_BROTLI
#define SPLAT_WITH_ZSTD
#define SPLAT_WITH_LZ4
#define SPLAT_WITH_LCMS2
#endif

#ifdef SPLAT_WITH_ZLIB
#include <zlib.h>
#endif
#ifdef SPLAT_WITH_BZIP2
#include <bzlib.h>
#endif
#ifdef SPLAT_WITH_LZMA
#include <lzma.h>
#endif
#ifdef SPLAT_WITH_BROTLI
#include <brotli/decode.h>
#include <brotli/encode.h>
#endif
#ifdef SPLAT_WITH_ZSTD
#include <zstd.h>
#endif
#ifdef SPLAT_WITH_LZ4
#include <lz4.h>
#endif
#ifdef SPLAT_WITH_LCMS2
#include <lcms2.h>
#include <math.h> // OKLab transform (powf/cbrtf)
#endif

#define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
#define SAFE_SNPRINTF(...) snprintf(__VA_ARGS__)

/* Header flag-word field layout (little-endian, see the spec at the top of this
 * file). Defined as macros rather than an enum because the metadata mask
 * (0xFF000000) does not fit the int range that ISO C requires for enumerators. */
#define SPLAT_FLAG_ENDIAN_BIG (1u << 0)

#define SPLAT_FLAG_SORTED (1u << 1)

#define SPLAT_FLAG_PRECISION_SHIFT 2
#define SPLAT_FLAG_PRECISION_MASK (0x3u << SPLAT_FLAG_PRECISION_SHIFT)
#define SPLAT_FLAG_PRECISION_FLOAT16 (0x0u << SPLAT_FLAG_PRECISION_SHIFT)
#define SPLAT_FLAG_PRECISION_FLOAT32 (0x1u << SPLAT_FLAG_PRECISION_SHIFT)
#define SPLAT_FLAG_PRECISION_FLOAT64 (0x2u << SPLAT_FLAG_PRECISION_SHIFT)
#define SPLAT_FLAG_PRECISION_FLOAT128 (0x3u << SPLAT_FLAG_PRECISION_SHIFT)

#define SPLAT_FLAG_COMPRESSION_SHIFT 4
#define SPLAT_FLAG_COMPRESSION_MASK (0xFu << SPLAT_FLAG_COMPRESSION_SHIFT)

#define SPLAT_FLAG_INDEX_WIDTH_SHIFT 8
#define SPLAT_FLAG_INDEX_WIDTH_MASK (0x3u << SPLAT_FLAG_INDEX_WIDTH_SHIFT)

#define SPLAT_FLAG_SPLAT_SHAPE_SHIFT 10
#define SPLAT_FLAG_SPLAT_SHAPE_MASK (0x3u << SPLAT_FLAG_SPLAT_SHAPE_SHIFT)

#define SPLAT_FLAG_COLOR_SPACE_SHIFT 12
#define SPLAT_FLAG_COLOR_SPACE_MASK (0xFu << SPLAT_FLAG_COLOR_SPACE_SHIFT)

#define SPLAT_FLAG_INTERP_SHIFT 16
#define SPLAT_FLAG_INTERP_MASK (0xFu << SPLAT_FLAG_INTERP_SHIFT)

#define SPLAT_FLAG_ENCRYPTION_SHIFT 20
#define SPLAT_FLAG_ENCRYPTION_MASK (0xFu << SPLAT_FLAG_ENCRYPTION_SHIFT)

#define SPLAT_FLAG_METADATA_SHIFT 24
#define SPLAT_FLAG_METADATA_MASK (0xFFu << SPLAT_FLAG_METADATA_SHIFT)

static uint32_t sanitize_flags(uint32_t flags) {
  // 4Splat files are always little-endian.
  flags &= ~SPLAT_FLAG_ENDIAN_BIG;

  if ((flags & SPLAT_FLAG_PRECISION_MASK) == SPLAT_FLAG_PRECISION_FLOAT16)
    flags = (flags & ~SPLAT_FLAG_PRECISION_MASK) | SPLAT_FLAG_PRECISION_FLOAT32;

  return flags;
}

// Whether this build can (de)compress the index payload with the given codec,
// and a human-readable name for it. Defined with the compression backends below.
static bool splat_compression_available(uint32_t codec);
static const char *splat_compression_display_name(uint32_t codec);

// Validate that a flag word describes a file this build can decode. Fields that
// only describe the payload (index width, splat shape, color space,
// interpolation, sort order and the metadata byte) accept every value and are
// preserved verbatim across a read/write round-trip; fields that change how the
// bytes are interpreted are checked against what the codec implements.
static bool flags_supported(uint32_t flags) {
  if (flags & SPLAT_FLAG_ENDIAN_BIG) {
    LOG_ERROR("❌ Big-endian 4Splat files are unsupported\n");
    return false;
  }

  uint32_t precision = flags & SPLAT_FLAG_PRECISION_MASK;
  if (precision == SPLAT_FLAG_PRECISION_FLOAT128) {
    LOG_ERROR("❌ Unsupported precision (Float128)\n");
    return false;
  }

  uint32_t compression = (flags & SPLAT_FLAG_COMPRESSION_MASK) >> SPLAT_FLAG_COMPRESSION_SHIFT;
  if (!splat_compression_available(compression)) {
    LOG_ERROR("❌ Compression scheme not available in this build: %s\n",
              splat_compression_display_name(compression));
    return false;
  }

  uint32_t splat_shape = (flags & SPLAT_FLAG_SPLAT_SHAPE_MASK) >> SPLAT_FLAG_SPLAT_SHAPE_SHIFT;
  if (splat_shape == 3) { // reserved
    LOG_ERROR("❌ Reserved splat shape value\n");
    return false;
  }

  uint32_t encryption = (flags & SPLAT_FLAG_ENCRYPTION_MASK) >> SPLAT_FLAG_ENCRYPTION_SHIFT;
  if (encryption != 0) {
    LOG_ERROR("❌ Encrypted 4Splat files are unsupported\n");
    return false;
  }

  return true;
}
enum { SPLAT4D_STREAM_CHUNK_SIZE = 1 << 15 };

typedef struct {
  float mu_x, sigma_x, mu_y, sigma_y, mu_z, sigma_z, mu_t, sigma_t, r, g, b, alpha;
  // Off-diagonal spatial covariance terms, used only by the full-covariance
  // splat shape; zero for isotropic and axis-aligned splats.
  float sigma_xy, sigma_xz, sigma_yz;
} Splat4D;

typedef enum {
  SPLAT_ENDIAN_LITTLE = 0,
  SPLAT_ENDIAN_BIG = 1,
} SplatEndian;

typedef enum {
  SPLAT_SORT_UNSORTED = 0,
  SPLAT_SORT_SORTED = 1,
} SplatSortOrder;

typedef enum {
  SPLAT_PRECISION_FLOAT16 = 0,
  SPLAT_PRECISION_FLOAT32 = 1,
  SPLAT_PRECISION_FLOAT64 = 2,
  SPLAT_PRECISION_FLOAT128 = 3,
} SplatPrecision;

typedef enum {
  SPLAT_COMPRESSION_NONE = 0,
  SPLAT_COMPRESSION_RUN_LENGTH = 1,
  SPLAT_COMPRESSION_DEFLATE = 2,
  SPLAT_COMPRESSION_RAR = 3,
  SPLAT_COMPRESSION_LZO = 4,
  SPLAT_COMPRESSION_ZLIB = 5,
  SPLAT_COMPRESSION_BZIP2 = 6,
  SPLAT_COMPRESSION_LZMA = 7,
  SPLAT_COMPRESSION_ZPAQ = 8,
  SPLAT_COMPRESSION_XZ = 9,
  SPLAT_COMPRESSION_LZ4 = 10,
  SPLAT_COMPRESSION_SNAPPY = 11,
  SPLAT_COMPRESSION_LZHAM = 12,
  SPLAT_COMPRESSION_BROTLI = 13,
  SPLAT_COMPRESSION_LZFSE = 14,
  SPLAT_COMPRESSION_ZSTD = 15,
} SplatCompression;

typedef enum {
  SPLAT_INDEX_WIDTH_8 = 0,
  SPLAT_INDEX_WIDTH_16 = 1,
  SPLAT_INDEX_WIDTH_32 = 2,
  SPLAT_INDEX_WIDTH_64 = 3,
} SplatIndexWidth;

typedef enum {
  SPLAT_SHAPE_ISOTROPIC = 0,
  SPLAT_SHAPE_AXIS_ALIGNED = 1,
  SPLAT_SHAPE_FULL_COVARIANCE = 2,
  SPLAT_SHAPE_RESERVED = 3,
} SplatShape;

typedef enum {
  SPLAT_COLOR_SRGB = 0,
  SPLAT_COLOR_LINEAR_SRGB = 1,
  SPLAT_COLOR_OKLAB = 2,
  SPLAT_COLOR_DISPLAY_P3 = 3,
  SPLAT_COLOR_REC709 = 4,
  SPLAT_COLOR_REC2020 = 5,
  SPLAT_COLOR_DCI_P3 = 6,
  SPLAT_COLOR_ACES_AP0 = 7,
  SPLAT_COLOR_PROPHOTO_RGB = 8,
  SPLAT_COLOR_REC2100 = 9,
  SPLAT_COLOR_CIE_LAB = 10,
  SPLAT_COLOR_CIE_XYZ_D65 = 11,
  SPLAT_COLOR_ACESCG_AP1 = 12,
  SPLAT_COLOR_REC601 = 13,
  SPLAT_COLOR_CIE_XYZ_D50 = 14,
  SPLAT_COLOR_CIE_XYZ_D65_ALT = 15,
} SplatColorSpace;

typedef enum {
  SPLAT_INTERP_NONE = 0,
  SPLAT_INTERP_NEAREST = 1,
  SPLAT_INTERP_AXIS_ALIGNED = 2,
  SPLAT_INTERP_SMOOTH = 3,
  SPLAT_INTERP_LANCZOS = 4,
  SPLAT_INTERP_GAUSSIAN = 5,
  SPLAT_INTERP_CATMULL_ROM = 6,
  SPLAT_INTERP_NURBS = 7,
  SPLAT_INTERP_RBF = 8,
  SPLAT_INTERP_OPTICAL_FLOW = 9,
  SPLAT_INTERP_NEURAL = 10,
  SPLAT_INTERP_AKIMA = 11,
  SPLAT_INTERP_INVERSE_DISTANCE = 12,
  SPLAT_INTERP_FOURIER = 13,
  SPLAT_INTERP_MOVING_LEAST_SQUARES = 14,
  SPLAT_INTERP_CUBIC_HERMITE = 15,
} SplatInterpolation;

typedef union {
  uint32_t raw;
  struct {
    uint32_t endian : 1;
    uint32_t sorted : 1;
    uint32_t precision : 2;
    uint32_t compression : 4;
    uint32_t index_width : 2;
    uint32_t splat_shape : 2;
    uint32_t color_space : 4;
    uint32_t interpolation : 4;
    uint32_t reserved : 12;
  } bits;
} Splat4DFlags;

_Static_assert(sizeof(Splat4DFlags) == sizeof(uint32_t),
               "Splat4DFlags must occupy exactly four bytes");

static inline Splat4DFlags splat4d_flags_from_raw(uint32_t raw) {
  return (Splat4DFlags){.raw = raw};
}

static const char *splat_endian_name(SplatEndian e) {
  switch (e) {
  case SPLAT_ENDIAN_LITTLE:
    return "Little-Endian";
  case SPLAT_ENDIAN_BIG:
    return "Big-Endian";
  default:
    return "Unknown";
  }
}

static const char *splat_sort_order_name(SplatSortOrder order) {
  switch (order) {
  case SPLAT_SORT_UNSORTED:
    return "Unsorted";
  case SPLAT_SORT_SORTED:
    return "Sorted";
  default:
    return "Unknown";
  }
}

static const char *splat_precision_name(SplatPrecision precision) {
  switch (precision) {
  case SPLAT_PRECISION_FLOAT16:
    return "Float16";
  case SPLAT_PRECISION_FLOAT32:
    return "Float32";
  case SPLAT_PRECISION_FLOAT64:
    return "Float64";
  case SPLAT_PRECISION_FLOAT128:
    return "Float128";
  default:
    return "Unknown";
  }
}

static const char *splat_compression_name(SplatCompression compression) {
  switch (compression) {
  case SPLAT_COMPRESSION_NONE:
    return "None";
  case SPLAT_COMPRESSION_RUN_LENGTH:
    return "Run Length";
  case SPLAT_COMPRESSION_DEFLATE:
    return "DEFLATE";
  case SPLAT_COMPRESSION_RAR:
    return "RAR";
  case SPLAT_COMPRESSION_LZO:
    return "LZO";
  case SPLAT_COMPRESSION_ZLIB:
    return "zlib";
  case SPLAT_COMPRESSION_BZIP2:
    return "bzip2";
  case SPLAT_COMPRESSION_LZMA:
    return "LZMA";
  case SPLAT_COMPRESSION_ZPAQ:
    return "ZPAQ";
  case SPLAT_COMPRESSION_XZ:
    return "XZ";
  case SPLAT_COMPRESSION_LZ4:
    return "LZ4";
  case SPLAT_COMPRESSION_SNAPPY:
    return "Snappy";
  case SPLAT_COMPRESSION_LZHAM:
    return "LZHAM";
  case SPLAT_COMPRESSION_BROTLI:
    return "Brotli";
  case SPLAT_COMPRESSION_LZFSE:
    return "LZFSE";
  case SPLAT_COMPRESSION_ZSTD:
    return "Zstd";
  default:
    return "Unknown";
  }
}

static const char *splat_index_width_name(SplatIndexWidth width) {
  switch (width) {
  case SPLAT_INDEX_WIDTH_8:
    return "1-byte";
  case SPLAT_INDEX_WIDTH_16:
    return "2-byte";
  case SPLAT_INDEX_WIDTH_32:
    return "4-byte";
  case SPLAT_INDEX_WIDTH_64:
    return "8-byte";
  default:
    return "Unknown";
  }
}

static const char *splat_shape_name(SplatShape shape) {
  switch (shape) {
  case SPLAT_SHAPE_ISOTROPIC:
    return "Isotropic";
  case SPLAT_SHAPE_AXIS_ALIGNED:
    return "Axis-Aligned";
  case SPLAT_SHAPE_FULL_COVARIANCE:
    return "Full Cov";
  case SPLAT_SHAPE_RESERVED:
    return "Reserved";
  default:
    return "Unknown";
  }
}

static const char *splat_color_space_name(SplatColorSpace color_space) {
  switch (color_space) {
  case SPLAT_COLOR_SRGB:
    return "sRGB";
  case SPLAT_COLOR_LINEAR_SRGB:
    return "Linear sRGB";
  case SPLAT_COLOR_OKLAB:
    return "OKLab";
  case SPLAT_COLOR_DISPLAY_P3:
    return "Display P3";
  case SPLAT_COLOR_REC709:
    return "Rec.709";
  case SPLAT_COLOR_REC2020:
    return "Rec.2020";
  case SPLAT_COLOR_DCI_P3:
    return "DCI-P3";
  case SPLAT_COLOR_ACES_AP0:
    return "ACES AP0";
  case SPLAT_COLOR_PROPHOTO_RGB:
    return "ProPhoto";
  case SPLAT_COLOR_REC2100:
    return "Rec.2100";
  case SPLAT_COLOR_CIE_LAB:
    return "CIE Lab";
  case SPLAT_COLOR_CIE_XYZ_D65:
    return "CIE XYZ D65";
  case SPLAT_COLOR_ACESCG_AP1:
    return "ACEScg AP1";
  case SPLAT_COLOR_REC601:
    return "Rec.601";
  case SPLAT_COLOR_CIE_XYZ_D50:
    return "CIE XYZ D50";
  case SPLAT_COLOR_CIE_XYZ_D65_ALT:
    return "CIE XYZ D65";
  default:
    return "Unknown";
  }
}

static const char *splat_interpolation_name(SplatInterpolation interpolation) {
  switch (interpolation) {
  case SPLAT_INTERP_NONE:
    return "None";
  case SPLAT_INTERP_NEAREST:
    return "Nearest";
  case SPLAT_INTERP_AXIS_ALIGNED:
    return "Axis-Aligned";
  case SPLAT_INTERP_SMOOTH:
    return "Smooth";
  case SPLAT_INTERP_LANCZOS:
    return "Lanczos";
  case SPLAT_INTERP_GAUSSIAN:
    return "Gaussian";
  case SPLAT_INTERP_CATMULL_ROM:
    return "Catmull-Rom";
  case SPLAT_INTERP_NURBS:
    return "NURBS";
  case SPLAT_INTERP_RBF:
    return "RBF";
  case SPLAT_INTERP_OPTICAL_FLOW:
    return "Optical Flow";
  case SPLAT_INTERP_NEURAL:
    return "Neural";
  case SPLAT_INTERP_AKIMA:
    return "Akima";
  case SPLAT_INTERP_INVERSE_DISTANCE:
    return "Inverse Distance";
  case SPLAT_INTERP_FOURIER:
    return "Fourier";
  case SPLAT_INTERP_MOVING_LEAST_SQUARES:
    return "Moving LS";
  case SPLAT_INTERP_CUBIC_HERMITE:
    return "Cubic Hermite";
  default:
    return "Unknown";
  }
}

typedef struct {
  uint32_t magic;
  uint8_t version[4];
  uint32_t width, height, depth, frames;
  uint32_t pSize;
  uint32_t flags;
} Splat4DHeader;

typedef struct {
  Splat4D *palette;
} Splat4DPalette;

typedef struct {
  uint64_t *index;
} Splat4DIndex;

typedef struct {
  uint32_t checksum;
  uint64_t idxoffset;
  uint32_t end;
} Splat4DFooter;

typedef struct {
  Splat4DHeader header;
  Splat4DPalette palette;
  Splat4DIndex index;
  Splat4DFooter footer;
} Splat4DVideo;

uint64_t header_total_indices(const Splat4DHeader *h);

// --- fixed on-disk container layout -----------------------------------------
//
// The header and footer are serialized field-by-field to the exact byte layout
// the format spec documents, independent of the host's struct padding and
// endianness. Numeric fields are little-endian; the "4SPL"/"LPS4" fourcc tags
// are written most-significant-byte first so the file reads as ASCII in order.
#define SPLAT_HEADER_DISK_BYTES 32
#define SPLAT_FOOTER_DISK_BYTES 16

// Upper bound on the decompressed index size accepted from a compressed file,
// so a tiny "decompression bomb" header cannot force a huge allocation.
#define SPLAT_MAX_COMPRESSED_INDEX_BYTES ((uint64_t)1 << 31) // 2 GiB

static void store_u32le(uint8_t *p, uint32_t v) {
  p[0] = (uint8_t)v;
  p[1] = (uint8_t)(v >> 8);
  p[2] = (uint8_t)(v >> 16);
  p[3] = (uint8_t)(v >> 24);
}

static void store_u64le(uint8_t *p, uint64_t v) {
  for (int i = 0; i < 8; ++i)
    p[i] = (uint8_t)(v >> (8 * i));
}

static void store_u32be(uint8_t *p, uint32_t v) {
  p[0] = (uint8_t)(v >> 24);
  p[1] = (uint8_t)(v >> 16);
  p[2] = (uint8_t)(v >> 8);
  p[3] = (uint8_t)v;
}

static uint32_t load_u32le(const uint8_t *p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint64_t load_u64le(const uint8_t *p) {
  uint64_t v = 0;
  for (int i = 0; i < 8; ++i)
    v |= (uint64_t)p[i] << (8 * i);
  return v;
}

static uint32_t load_u32be(const uint8_t *p) {
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static void serialize_header(const Splat4DHeader *h, uint8_t out[SPLAT_HEADER_DISK_BYTES]) {
  store_u32be(out + 0, h->magic); // "4SPL"
  out[4] = h->version[0];
  out[5] = h->version[1];
  out[6] = h->version[2];
  out[7] = h->version[3];
  store_u32le(out + 8, h->width);
  store_u32le(out + 12, h->height);
  store_u32le(out + 16, h->depth);
  store_u32le(out + 20, h->frames);
  store_u32le(out + 24, h->pSize);
  store_u32le(out + 28, h->flags);
}

static void deserialize_header(const uint8_t in[SPLAT_HEADER_DISK_BYTES], Splat4DHeader *h) {
  h->magic = load_u32be(in + 0);
  h->version[0] = in[4];
  h->version[1] = in[5];
  h->version[2] = in[6];
  h->version[3] = in[7];
  h->width = load_u32le(in + 8);
  h->height = load_u32le(in + 12);
  h->depth = load_u32le(in + 16);
  h->frames = load_u32le(in + 20);
  h->pSize = load_u32le(in + 24);
  h->flags = load_u32le(in + 28);
}

static void serialize_footer(const Splat4DFooter *f, uint8_t out[SPLAT_FOOTER_DISK_BYTES]) {
  store_u64le(out + 0, f->idxoffset);
  store_u32le(out + 8, f->checksum);
  store_u32be(out + 12, f->end); // "LPS4"
}

static void deserialize_footer(const uint8_t in[SPLAT_FOOTER_DISK_BYTES], Splat4DFooter *f) {
  f->idxoffset = load_u64le(in + 0);
  f->checksum = load_u32le(in + 8);
  f->end = load_u32be(in + 12);
}

// utils //
typedef struct {
  uint32_t v;
} crc32_t;

static const uint32_t crc32_table[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

// Named splat_crc32 rather than crc32 to avoid colliding with zlib's crc32()
// when SPLAT_WITH_ZLIB is enabled.
uint32_t splat_crc32(const void *data, size_t len) {
  const uint8_t *p = data;
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < len; i++) {
    crc = (crc >> 8) ^ crc32_table[(crc ^ p[i]) & 0xFF];
  }
  return ~crc;
}

static inline void crc32_init(crc32_t *c) { c->v = 0xFFFFFFFFu; }

static inline void crc32_update(crc32_t *c, const void *p, size_t n) {
  const uint8_t *b = p;
  // ⚡ Bolt Optimization: Extract hash accumulator to avoid defensive
  // memory reloads in this tight bitwise loop.
  uint32_t cv = c->v;
  for (size_t i = 0; i < n; i++) {
    cv = (cv >> 8) ^ crc32_table[(cv ^ b[i]) & 0xFF];
  }
  c->v = cv;
}

static inline uint32_t crc32_final(crc32_t *c) { return ~c->v; }

// --- compression backends ---------------------------------------------------
//
// The index payload (the packed run of palette references) can be stored using
// any of the compression schemes the format defines. None and RLE are always
// available; the rest are provided by third-party libraries compiled in via the
// SPLAT_WITH_* macros. Each backend exposes the same shape: compress() returns a
// freshly malloc'd buffer, decompress() fills a caller-provided buffer whose
// size is known from the header (total indices * index width).

// Hand-rolled byte-oriented run-length encoding: a stream of (count, value)
// pairs with 1 <= count <= 255. Always available, no dependencies.
static uint8_t *rle_compress(const uint8_t *in, size_t n, size_t *out_len) {
  if (n > SIZE_MAX / 2)
    return NULL;
  size_t cap = n ? n * 2 : 2;
  uint8_t *out = malloc(cap);
  if (!out)
    return NULL;
  size_t o = 0, i = 0;
  while (i < n) {
    uint8_t v = in[i];
    size_t run = 1;
    while (i + run < n && in[i + run] == v && run < 255)
      run++;
    out[o++] = (uint8_t)run;
    out[o++] = v;
    i += run;
  }
  *out_len = o;
  return out;
}

static bool rle_decompress(const uint8_t *in, size_t n, uint8_t *out, size_t expected) {
  size_t o = 0, i = 0;
  while (i + 1 < n) {
    uint8_t run = in[i];
    uint8_t v = in[i + 1];
    i += 2;
    if (run == 0 || o + run > expected)
      return false;
    memset(out + o, v, run);
    o += run;
  }
  return i == n && o == expected;
}

#ifdef SPLAT_WITH_ZLIB
// windowBits selects the wrapper: -15 = raw DEFLATE, 15 = zlib.
static uint8_t *zlib_do_compress(const uint8_t *in, size_t n, size_t *out_len, int windowBits) {
  if (n > UINT_MAX)
    return NULL;
  z_stream zs;
  memset(&zs, 0, sizeof zs);
  if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY) != Z_OK)
    return NULL;
  uLong bound = deflateBound(&zs, (uLong)n);
  uint8_t *out = malloc(bound ? bound : 1);
  if (!out) {
    deflateEnd(&zs);
    return NULL;
  }
  zs.next_in = (Bytef *)in;
  zs.avail_in = (uInt)n;
  zs.next_out = out;
  zs.avail_out = (uInt)bound;
  int r = deflate(&zs, Z_FINISH);
  if (r != Z_STREAM_END) {
    free(out);
    deflateEnd(&zs);
    return NULL;
  }
  *out_len = zs.total_out;
  deflateEnd(&zs);
  return out;
}

static bool zlib_do_decompress(const uint8_t *in, size_t n, uint8_t *out, size_t expected,
                               int windowBits) {
  if (n > UINT_MAX || expected > UINT_MAX)
    return false;
  z_stream zs;
  memset(&zs, 0, sizeof zs);
  if (inflateInit2(&zs, windowBits) != Z_OK)
    return false;
  zs.next_in = (Bytef *)in;
  zs.avail_in = (uInt)n;
  zs.next_out = out;
  zs.avail_out = (uInt)expected;
  int r = inflate(&zs, Z_FINISH);
  bool ok = (r == Z_STREAM_END) && zs.total_out == expected;
  inflateEnd(&zs);
  return ok;
}
#endif

#ifdef SPLAT_WITH_LZMA
// Drive an already-initialized encoder or decoder stream to completion.
static uint8_t *lzma_stream_run(lzma_stream *strm, const uint8_t *in, size_t in_len,
                                size_t *out_len) {
  size_t cap = in_len / 2 + 4096;
  uint8_t *out = malloc(cap);
  if (!out) {
    lzma_end(strm);
    return NULL;
  }
  strm->next_in = in;
  strm->avail_in = in_len;
  strm->next_out = out;
  strm->avail_out = cap;
  for (;;) {
    lzma_ret r = lzma_code(strm, LZMA_FINISH);
    if (r == LZMA_STREAM_END)
      break;
    if (r != LZMA_OK) {
      free(out);
      lzma_end(strm);
      return NULL;
    }
    if (strm->avail_out == 0) {
      size_t used = cap;
      if (cap > SIZE_MAX / 2) {
        free(out);
        lzma_end(strm);
        return NULL;
      }
      size_t ncap = cap * 2;
      uint8_t *grown = realloc(out, ncap);
      if (!grown) {
        free(out);
        lzma_end(strm);
        return NULL;
      }
      out = grown;
      strm->next_out = out + used;
      strm->avail_out = ncap - used;
      cap = ncap;
    }
  }
  *out_len = cap - strm->avail_out;
  lzma_end(strm);
  return out;
}

static uint8_t *lzma_do_compress(const uint8_t *in, size_t n, size_t *out_len, bool xz) {
  lzma_stream strm = LZMA_STREAM_INIT;
  if (xz) {
    if (lzma_easy_encoder(&strm, LZMA_PRESET_DEFAULT, LZMA_CHECK_CRC64) != LZMA_OK)
      return NULL;
  } else {
    lzma_options_lzma opt;
    if (lzma_lzma_preset(&opt, LZMA_PRESET_DEFAULT))
      return NULL;
    if (lzma_alone_encoder(&strm, &opt) != LZMA_OK)
      return NULL;
  }
  return lzma_stream_run(&strm, in, n, out_len);
}

static bool lzma_do_decompress(const uint8_t *in, size_t n, uint8_t *out, size_t expected,
                               bool xz) {
  lzma_stream strm = LZMA_STREAM_INIT;
  if (xz) {
    if (lzma_stream_decoder(&strm, UINT64_MAX, 0) != LZMA_OK)
      return false;
  } else {
    if (lzma_alone_decoder(&strm, UINT64_MAX) != LZMA_OK)
      return false;
  }
  size_t got = 0;
  uint8_t *buf = lzma_stream_run(&strm, in, n, &got);
  if (!buf)
    return false;
  bool ok = (got == expected);
  if (ok)
    memcpy(out, buf, expected);
  free(buf);
  return ok;
}
#endif

// Is the given compression codec (a 4-bit flag value) usable in this build?
static bool splat_compression_available(uint32_t codec) {
  switch (codec) {
  case SPLAT_COMPRESSION_NONE:
  case SPLAT_COMPRESSION_RUN_LENGTH:
    return true;
#ifdef SPLAT_WITH_ZLIB
  case SPLAT_COMPRESSION_DEFLATE:
  case SPLAT_COMPRESSION_ZLIB:
    return true;
#endif
#ifdef SPLAT_WITH_BZIP2
  case SPLAT_COMPRESSION_BZIP2:
    return true;
#endif
#ifdef SPLAT_WITH_LZMA
  case SPLAT_COMPRESSION_LZMA:
  case SPLAT_COMPRESSION_XZ:
    return true;
#endif
#ifdef SPLAT_WITH_LZ4
  case SPLAT_COMPRESSION_LZ4:
    return true;
#endif
#ifdef SPLAT_WITH_BROTLI
  case SPLAT_COMPRESSION_BROTLI:
    return true;
#endif
#ifdef SPLAT_WITH_ZSTD
  case SPLAT_COMPRESSION_ZSTD:
    return true;
#endif
  default:
    return false;
  }
}

static const char *splat_compression_display_name(uint32_t codec) {
  static const char *names[16] = {"None",  "RLE",    "DEFLATE", "RAR", "LZO", "zlib",
                                  "bzip2", "LZMA",   "ZPAQ",    "XZ",  "LZ4", "Snappy",
                                  "LZHAM", "Brotli", "LZFSE",   "Zstd"};
  return codec < 16 ? names[codec] : "Unknown";
}

// Compress in[0..in_len) with `codec`; returns a malloc'd buffer (caller frees)
// and stores its length in *out_len, or NULL on failure / unavailable codec.
static uint8_t *splat_compress(uint32_t codec, const uint8_t *in, size_t in_len, size_t *out_len) {
  switch (codec) {
  case SPLAT_COMPRESSION_RUN_LENGTH:
    return rle_compress(in, in_len, out_len);
#ifdef SPLAT_WITH_ZLIB
  case SPLAT_COMPRESSION_DEFLATE:
    return zlib_do_compress(in, in_len, out_len, -15);
  case SPLAT_COMPRESSION_ZLIB:
    return zlib_do_compress(in, in_len, out_len, 15);
#endif
#ifdef SPLAT_WITH_BZIP2
  case SPLAT_COMPRESSION_BZIP2: {
    if (in_len > UINT_MAX)
      return NULL;
    unsigned int bound = (unsigned int)in_len + in_len / 100 + 600;
    uint8_t *out = malloc(bound ? bound : 1);
    if (!out)
      return NULL;
    unsigned int dst_len = bound;
    int r = BZ2_bzBuffToBuffCompress((char *)out, &dst_len, (char *)(uintptr_t)in,
                                     (unsigned int)in_len, 9, 0, 0);
    if (r != BZ_OK) {
      free(out);
      return NULL;
    }
    *out_len = dst_len;
    return out;
  }
#endif
#ifdef SPLAT_WITH_LZMA
  case SPLAT_COMPRESSION_LZMA:
    return lzma_do_compress(in, in_len, out_len, false);
  case SPLAT_COMPRESSION_XZ:
    return lzma_do_compress(in, in_len, out_len, true);
#endif
#ifdef SPLAT_WITH_LZ4
  case SPLAT_COMPRESSION_LZ4: {
    if (in_len > (size_t)LZ4_MAX_INPUT_SIZE)
      return NULL;
    int bound = LZ4_compressBound((int)in_len);
    if (bound <= 0)
      return NULL;
    uint8_t *out = malloc((size_t)bound);
    if (!out)
      return NULL;
    int wrote = LZ4_compress_default((const char *)in, (char *)out, (int)in_len, bound);
    if (wrote <= 0) {
      free(out);
      return NULL;
    }
    *out_len = (size_t)wrote;
    return out;
  }
#endif
#ifdef SPLAT_WITH_BROTLI
  case SPLAT_COMPRESSION_BROTLI: {
    size_t bound = BrotliEncoderMaxCompressedSize(in_len);
    if (bound == 0)
      bound = in_len + 1024;
    uint8_t *out = malloc(bound);
    if (!out)
      return NULL;
    size_t dst_len = bound;
    if (!BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW, BROTLI_MODE_GENERIC,
                               in_len, in, &dst_len, out)) {
      free(out);
      return NULL;
    }
    *out_len = dst_len;
    return out;
  }
#endif
#ifdef SPLAT_WITH_ZSTD
  case SPLAT_COMPRESSION_ZSTD: {
    size_t bound = ZSTD_compressBound(in_len);
    uint8_t *out = malloc(bound ? bound : 1);
    if (!out)
      return NULL;
    size_t wrote = ZSTD_compress(out, bound, in, in_len, ZSTD_CLEVEL_DEFAULT);
    if (ZSTD_isError(wrote)) {
      free(out);
      return NULL;
    }
    *out_len = wrote;
    return out;
  }
#endif
  default:
    return NULL;
  }
}

// Decompress in[0..in_len) into out[0..expected_len) using `codec`. The output
// size is exact and known from the header, so any mismatch is a hard failure.
static bool splat_decompress(uint32_t codec, const uint8_t *in, size_t in_len, uint8_t *out,
                             size_t expected_len) {
  switch (codec) {
  case SPLAT_COMPRESSION_RUN_LENGTH:
    return rle_decompress(in, in_len, out, expected_len);
#ifdef SPLAT_WITH_ZLIB
  case SPLAT_COMPRESSION_DEFLATE:
    return zlib_do_decompress(in, in_len, out, expected_len, -15);
  case SPLAT_COMPRESSION_ZLIB:
    return zlib_do_decompress(in, in_len, out, expected_len, 15);
#endif
#ifdef SPLAT_WITH_BZIP2
  case SPLAT_COMPRESSION_BZIP2: {
    if (in_len > UINT_MAX || expected_len > UINT_MAX)
      return false;
    unsigned int dst_len = (unsigned int)expected_len;
    int r = BZ2_bzBuffToBuffDecompress((char *)out, &dst_len, (char *)(uintptr_t)in,
                                       (unsigned int)in_len, 0, 0);
    return r == BZ_OK && dst_len == expected_len;
  }
#endif
#ifdef SPLAT_WITH_LZMA
  case SPLAT_COMPRESSION_LZMA:
    return lzma_do_decompress(in, in_len, out, expected_len, false);
  case SPLAT_COMPRESSION_XZ:
    return lzma_do_decompress(in, in_len, out, expected_len, true);
#endif
#ifdef SPLAT_WITH_LZ4
  case SPLAT_COMPRESSION_LZ4: {
    if (in_len > INT_MAX || expected_len > INT_MAX)
      return false;
    int got = LZ4_decompress_safe((const char *)in, (char *)out, (int)in_len, (int)expected_len);
    return got >= 0 && (size_t)got == expected_len;
  }
#endif
#ifdef SPLAT_WITH_BROTLI
  case SPLAT_COMPRESSION_BROTLI: {
    size_t dst_len = expected_len;
    BrotliDecoderResult r = BrotliDecoderDecompress(in_len, in, &dst_len, out);
    return r == BROTLI_DECODER_RESULT_SUCCESS && dst_len == expected_len;
  }
#endif
#ifdef SPLAT_WITH_ZSTD
  case SPLAT_COMPRESSION_ZSTD: {
    size_t got = ZSTD_decompress(out, expected_len, in, in_len);
    return !ZSTD_isError(got) && got == expected_len;
  }
#endif
  default:
    return false;
  }
}

// --- color-space conversion (LittleCMS) -------------------------------------
//
// The header names the color space the palette RGB values live in. When the
// lcms2 backend is compiled in, the palette colors can be converted between a
// documented subset of those spaces; the color-space field is updated to match.
// Spaces outside the subset (OKLab, the ACES and Rec.2100 variants, DCI-P3,
// Rec.601) are rejected with a diagnostic rather than silently mishandled.
#ifdef SPLAT_WITH_LCMS2
static cmsToneCurve *splat_srgb_curve(void) {
  // IEC 61966-2-1 sRGB transfer function as an ICC parametric type-4 curve.
  cmsFloat64Number params[5] = {2.4, 1.0 / 1.055, 0.055 / 1.055, 1.0 / 12.92, 0.04045};
  return cmsBuildParametricToneCurve(NULL, 4, params);
}

static cmsHPROFILE splat_rgb_profile(cmsCIExyY white, cmsCIExyYTRIPLE primaries, double gamma) {
  cmsToneCurve *tc = (gamma <= 0.0) ? splat_srgb_curve() : cmsBuildGamma(NULL, gamma);
  if (!tc)
    return NULL;
  cmsToneCurve *curves[3] = {tc, tc, tc};
  cmsHPROFILE profile = cmsCreateRGBProfile(&white, &primaries, curves);
  cmsFreeToneCurve(tc);
  return profile;
}

// Resolve a color space to an lcms profile and matching float pixel format.
// Returns false for spaces this backend does not model.
static bool splat_space_profile(uint32_t space, cmsHPROFILE *profile, cmsUInt32Number *format) {
  const cmsCIExyY d65 = {0.3127, 0.3290, 1.0};
  const cmsCIExyY d50 = {0.3457, 0.3585, 1.0};
  switch (space) {
  case SPLAT_COLOR_SRGB:
  case SPLAT_COLOR_REC709: { // Rec.709 shares the sRGB primaries/white point
    *profile = cmsCreate_sRGBProfile();
    *format = TYPE_RGB_FLT;
    return *profile != NULL;
  }
  case SPLAT_COLOR_LINEAR_SRGB: {
    cmsCIExyYTRIPLE p = {{0.640, 0.330, 1}, {0.300, 0.600, 1}, {0.150, 0.060, 1}};
    *profile = splat_rgb_profile(d65, p, 1.0);
    *format = TYPE_RGB_FLT;
    return *profile != NULL;
  }
  case SPLAT_COLOR_DISPLAY_P3: {
    cmsCIExyYTRIPLE p = {{0.680, 0.320, 1}, {0.265, 0.690, 1}, {0.150, 0.060, 1}};
    *profile = splat_rgb_profile(d65, p, 0.0); // sRGB transfer
    *format = TYPE_RGB_FLT;
    return *profile != NULL;
  }
  case SPLAT_COLOR_REC2020: {
    cmsCIExyYTRIPLE p = {{0.708, 0.292, 1}, {0.170, 0.797, 1}, {0.131, 0.046, 1}};
    *profile = splat_rgb_profile(d65, p, 0.0);
    *format = TYPE_RGB_FLT;
    return *profile != NULL;
  }
  case SPLAT_COLOR_PROPHOTO_RGB: {
    cmsCIExyYTRIPLE p = {{0.7347, 0.2653, 1}, {0.1596, 0.8404, 1}, {0.0366, 0.0001, 1}};
    *profile = splat_rgb_profile(d50, p, 1.8);
    *format = TYPE_RGB_FLT;
    return *profile != NULL;
  }
  case SPLAT_COLOR_CIE_LAB: {
    *profile = cmsCreateLab4Profile(NULL);
    *format = TYPE_Lab_FLT;
    return *profile != NULL;
  }
  case SPLAT_COLOR_CIE_XYZ_D65:
  case SPLAT_COLOR_CIE_XYZ_D50:
  case SPLAT_COLOR_CIE_XYZ_D65_ALT: {
    *profile = cmsCreateXYZProfile();
    *format = TYPE_XYZ_FLT;
    return *profile != NULL;
  }
  default:
    return false;
  }
}

// Convert every palette entry's (r, g, b) from `from` to `to`. Non-RGB targets
// (Lab, XYZ) leave their channel values in the r/g/b slots.
// Convert the palette between two lcms-modeled spaces (neither OKLab).
static bool lcms_convert_palette(Splat4D *palette, uint32_t count, uint32_t from, uint32_t to) {
  if (from == to)
    return true;
  cmsHPROFILE in_profile = NULL, out_profile = NULL;
  cmsUInt32Number in_format = 0, out_format = 0;
  if (!splat_space_profile(from, &in_profile, &in_format)) {
    LOG_ERROR("❌ Color-space conversion from %s is not supported\n",
              splat_color_space_name((SplatColorSpace)from));
    return false;
  }
  if (!splat_space_profile(to, &out_profile, &out_format)) {
    LOG_ERROR("❌ Color-space conversion to %s is not supported\n",
              splat_color_space_name((SplatColorSpace)to));
    cmsCloseProfile(in_profile);
    return false;
  }

  cmsHTRANSFORM xform = cmsCreateTransform(in_profile, in_format, out_profile, out_format,
                                           INTENT_RELATIVE_COLORIMETRIC,
                                           cmsFLAGS_BLACKPOINTCOMPENSATION | cmsFLAGS_NOOPTIMIZE);
  cmsCloseProfile(in_profile);
  cmsCloseProfile(out_profile);
  if (!xform)
    return false;

  for (uint32_t i = 0; i < count; ++i) {
    float in[3] = {palette[i].r, palette[i].g, palette[i].b};
    float out[3];
    cmsDoTransform(xform, in, out, 1);
    palette[i].r = out[0];
    palette[i].g = out[1];
    palette[i].b = out[2];
  }
  cmsDeleteTransform(xform);
  return true;
}

// OKLab (Björn Ottosson, 2020) <-> sRGB, per the reference formulae.
static float srgb_to_linear(float c) {
  return c <= 0.04045f ? c / 12.92f : powf((c + 0.055f) / 1.055f, 2.4f);
}
static float linear_to_srgb(float c) {
  return c <= 0.0031308f ? 12.92f * c : 1.055f * powf(c, 1.0f / 2.4f) - 0.055f;
}

static void oklab_from_srgb(Splat4D *p) {
  float r = srgb_to_linear(p->r), g = srgb_to_linear(p->g), b = srgb_to_linear(p->b);
  float l = 0.4122214708f * r + 0.5363325363f * g + 0.0514459929f * b;
  float m = 0.2119034982f * r + 0.6806995451f * g + 0.1073969566f * b;
  float s = 0.0883024619f * r + 0.2817188376f * g + 0.6299787005f * b;
  float l_ = cbrtf(l), m_ = cbrtf(m), s_ = cbrtf(s);
  p->r = 0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_; // L
  p->g = 1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_; // a
  p->b = 0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_; // b
}

static void srgb_from_oklab(Splat4D *p) {
  float L = p->r, A = p->g, B = p->b;
  float l_ = L + 0.3963377774f * A + 0.2158037573f * B;
  float m_ = L - 0.1055613458f * A - 0.0638541728f * B;
  float s_ = L - 0.0894841775f * A - 1.2914855480f * B;
  float l = l_ * l_ * l_, m = m_ * m_ * m_, s = s_ * s_ * s_;
  float r = +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
  float g = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
  float b = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;
  p->r = linear_to_srgb(r);
  p->g = linear_to_srgb(g);
  p->b = linear_to_srgb(b);
}

// Convert the palette between color spaces. OKLab is handled directly (it is not
// an ICC space); any other pair goes through LittleCMS. OKLab conversions pivot
// through sRGB, so OKLab <-> any lcms-modeled space works too.
static bool splat_convert_palette_colors(Splat4D *palette, uint32_t count, uint32_t from,
                                         uint32_t to) {
  if (from == to)
    return true;

  bool from_oklab = (from == SPLAT_COLOR_OKLAB), to_oklab = (to == SPLAT_COLOR_OKLAB);
  if (!from_oklab && !to_oklab)
    return lcms_convert_palette(palette, count, from, to);

  // Step 1: bring the palette to sRGB.
  if (from_oklab) {
    for (uint32_t i = 0; i < count; ++i)
      srgb_from_oklab(&palette[i]);
  } else if (from != SPLAT_COLOR_SRGB) {
    if (!lcms_convert_palette(palette, count, from, SPLAT_COLOR_SRGB))
      return false;
  }

  // Step 2: sRGB to the target.
  if (to_oklab) {
    for (uint32_t i = 0; i < count; ++i)
      oklab_from_srgb(&palette[i]);
  } else if (to != SPLAT_COLOR_SRGB) {
    if (!lcms_convert_palette(palette, count, SPLAT_COLOR_SRGB, to))
      return false;
  }
  return true;
}
#endif // SPLAT_WITH_LCMS2

// streaming helpers //
typedef bool (*Splat4DChunkFn)(const uint8_t *chunk, size_t n, void *ctx);

static bool splat4d_stream_block(const uint8_t *data, size_t len, size_t chunk, Splat4DChunkFn fn,
                                 void *ctx) {
  if (!data || len == 0)
    return true;

  if (chunk == 0)
    chunk = SPLAT4D_STREAM_CHUNK_SIZE;

  while (len > 0) {
    size_t step = len < chunk ? len : chunk;
    if (!fn(data, step, ctx))
      return false;
    data += step;
    len -= step;
  }
  return true;
}

static bool checked_mul_u64(uint64_t a, uint64_t b, uint64_t *out) {
  if (!out)
    return false;
  if (b != 0 && a > UINT64_MAX / b)
    return false;
  *out = a * b;
  return true;
}

bool header_total_indices_checked(const Splat4DHeader *h, uint64_t *total) {
  if (!h || !total)
    return false;

  uint64_t dims[4] = {h->width, h->height, h->depth, h->frames};
  uint64_t acc = 1;
  for (size_t i = 0; i < 4; ++i) {
    if (dims[i] == 0)
      return false;
    if (!checked_mul_u64(acc, dims[i], &acc))
      return false;
  }

  *total = acc;
  return true;
}

uint64_t header_total_indices(const Splat4DHeader *h) {
  return (uint64_t)h->width * (uint64_t)h->height * (uint64_t)h->depth * (uint64_t)h->frames;
}

static uint8_t get_index_width_bytes(uint32_t flags) {
  uint32_t w = (flags >> 8) & 0x3u;
  if (w == SPLAT_INDEX_WIDTH_8)
    return 1;
  if (w == SPLAT_INDEX_WIDTH_16)
    return 2;
  if (w == SPLAT_INDEX_WIDTH_32)
    return 4;
  return 8; // SPLAT_INDEX_WIDTH_64
}

// Largest index value representable in idx_width bytes.
static uint64_t index_width_max_value(uint8_t idx_width) {
  if (idx_width >= 8)
    return UINT64_MAX;
  return ((uint64_t)1 << (8u * idx_width)) - 1u;
}

// --- palette precision ---
//
// A Splat4D is twelve packed IEEE-754 floats in memory. On disk the palette is
// stored at the precision selected by the header's precision field: float16
// (2 bytes/component), float32 (4) or float64 (8). float32 is the in-memory
// representation, so that path is a byte-for-byte copy; float64 widens exactly;
// float16 is a lossy narrowing.
_Static_assert(sizeof(Splat4D) == 15 * sizeof(float), "Splat4D must be 15 packed floats");

// Largest palette entry (full covariance): mu_{x,y,z} + 6 covariance values +
// mu_t + sigma_t + r,g,b,alpha.
#define SPLAT_PALETTE_MAX_COMPONENTS 15

static uint32_t splat_precision_bits(uint32_t flags) {
  return (flags & SPLAT_FLAG_PRECISION_MASK) >> SPLAT_FLAG_PRECISION_SHIFT;
}

static size_t splat_precision_component_bytes(uint32_t flags) {
  switch (splat_precision_bits(flags)) {
  case 0:
    return 2; // Float16
  case 2:
    return 8; // Float64
  default:
    return 4; // Float32 (the only other value that reaches disk)
  }
}

static uint32_t splat_shape_bits(uint32_t flags) {
  return (flags & SPLAT_FLAG_SPLAT_SHAPE_MASK) >> SPLAT_FLAG_SPLAT_SHAPE_SHIFT;
}

// Number of spatial spread values stored per palette entry for the splat shape:
// isotropic = 1 (a shared sigma), axis-aligned = 3 (sigma_x/y/z), full
// covariance = 6 (the symmetric 3x3 upper triangle). The reserved value (3) is
// rejected by flags_supported() and never reaches here.
static uint32_t splat_shape_cov_count(uint32_t flags) {
  switch (splat_shape_bits(flags)) {
  case 0:
    return 1; // Isotropic
  case 2:
    return 6; // Full covariance
  default:
    return 3; // Axis-aligned
  }
}

// Total floats in a palette entry: mu_{x,y,z} (3) + covariance block + mu_t +
// sigma_t (2) + r,g,b,alpha (4).
static uint32_t splat_palette_component_count(uint32_t flags) {
  return 3 + splat_shape_cov_count(flags) + 2 + 4;
}

static size_t palette_entry_disk_bytes(uint32_t flags) {
  return (size_t)splat_palette_component_count(flags) * splat_precision_component_bytes(flags);
}

// IEEE-754 binary16 <-> binary32 with round-to-nearest-even.
static uint16_t float_to_half(float value) {
  uint32_t x;
  memcpy(&x, &value, sizeof x);
  uint32_t sign = (x >> 16) & 0x8000u;
  uint32_t exp = (x >> 23) & 0xFFu;
  uint32_t mant = x & 0x7FFFFFu;

  if (exp == 0xFF) // Inf / NaN
    return (uint16_t)(sign | 0x7C00u | (mant ? 0x200u : 0u));

  int32_t e = (int32_t)exp - 127 + 15;
  if (e >= 0x1F) // overflow -> Inf
    return (uint16_t)(sign | 0x7C00u);

  if (e <= 0) {
    if (e < -10) // too small -> signed zero
      return (uint16_t)sign;
    mant |= 0x800000u; // restore implicit leading bit
    uint32_t shift = (uint32_t)(14 - e);
    uint32_t half_mant = mant >> shift;
    uint32_t remainder = mant & ((1u << shift) - 1u);
    uint32_t halfway = 1u << (shift - 1);
    if (remainder > halfway || (remainder == halfway && (half_mant & 1u)))
      half_mant++;
    return (uint16_t)(sign | half_mant);
  }

  uint16_t half = (uint16_t)(sign | ((uint32_t)e << 10) | (mant >> 13));
  uint32_t remainder = mant & 0x1FFFu;
  if (remainder > 0x1000u || (remainder == 0x1000u && (half & 1u)))
    half++; // carry propagates into the exponent correctly
  return half;
}

static float half_to_float(uint16_t h) {
  uint32_t sign = (uint32_t)(h & 0x8000u) << 16;
  uint32_t exp = (h >> 10) & 0x1Fu;
  uint32_t mant = h & 0x3FFu;
  uint32_t f;

  if (exp == 0) {
    if (mant == 0) {
      f = sign;
    } else {
      exp = 127 - 15 + 1;
      while (!(mant & 0x400u)) {
        mant <<= 1;
        exp--;
      }
      mant &= 0x3FFu;
      f = sign | (exp << 23) | (mant << 13);
    }
  } else if (exp == 0x1F) {
    f = sign | 0x7F800000u | (mant << 13);
  } else {
    exp = exp - 15 + 127;
    f = sign | (exp << 23) | (mant << 13);
  }

  float out;
  memcpy(&out, &f, sizeof out);
  return out;
}

// Write/read a single float component at the given precision.
static void store_component(uint8_t *p, float v, uint32_t precision) {
  if (precision == 0) { // Float16
    uint16_t h = float_to_half(v);
    memcpy(p, &h, 2);
  } else if (precision == 2) { // Float64
    double d = (double)v;
    memcpy(p, &d, 8);
  } else { // Float32
    memcpy(p, &v, 4);
  }
}

static float load_component(const uint8_t *p, uint32_t precision) {
  if (precision == 0) { // Float16
    uint16_t h;
    memcpy(&h, p, 2);
    return half_to_float(h);
  }
  if (precision == 2) { // Float64
    double d;
    memcpy(&d, p, 8);
    return (float)d;
  }
  float f; // Float32
  memcpy(&f, p, 4);
  return f;
}

// Serialize a palette entry in the spec's field order (grouped means, then the
// shape-dependent covariance block, then temporal and color), each component at
// the selected precision.
static void serialize_palette_entry(const Splat4D *s, uint32_t flags, uint8_t *out) {
  uint32_t precision = splat_precision_bits(flags);
  uint32_t cov = splat_shape_cov_count(flags);
  size_t cb = splat_precision_component_bytes(flags);

  float comps[SPLAT_PALETTE_MAX_COMPONENTS];
  int n = 0;
  comps[n++] = s->mu_x;
  comps[n++] = s->mu_y;
  comps[n++] = s->mu_z;
  comps[n++] = s->sigma_x; // isotropic stores this single shared sigma
  if (cov >= 3) {
    comps[n++] = s->sigma_y;
    comps[n++] = s->sigma_z;
  }
  if (cov >= 6) {
    comps[n++] = s->sigma_xy;
    comps[n++] = s->sigma_xz;
    comps[n++] = s->sigma_yz;
  }
  comps[n++] = s->mu_t;
  comps[n++] = s->sigma_t;
  comps[n++] = s->r;
  comps[n++] = s->g;
  comps[n++] = s->b;
  comps[n++] = s->alpha;

  for (int i = 0; i < n; ++i)
    store_component(out + (size_t)i * cb, comps[i], precision);
}

static void deserialize_palette_entry(const uint8_t *in, uint32_t flags, Splat4D *s) {
  uint32_t precision = splat_precision_bits(flags);
  uint32_t cov = splat_shape_cov_count(flags);
  size_t cb = splat_precision_component_bytes(flags);
  uint32_t n = splat_palette_component_count(flags);

  float comps[SPLAT_PALETTE_MAX_COMPONENTS];
  for (uint32_t i = 0; i < n; ++i)
    comps[i] = load_component(in + (size_t)i * cb, precision);

  int k = 0;
  s->mu_x = comps[k++];
  s->mu_y = comps[k++];
  s->mu_z = comps[k++];
  if (cov == 1) {
    // Isotropic: one shared spatial sigma expands to all three axes.
    s->sigma_x = s->sigma_y = s->sigma_z = comps[k++];
    s->sigma_xy = s->sigma_xz = s->sigma_yz = 0.0f;
  } else {
    s->sigma_x = comps[k++];
    s->sigma_y = comps[k++];
    s->sigma_z = comps[k++];
    if (cov == 6) {
      s->sigma_xy = comps[k++];
      s->sigma_xz = comps[k++];
      s->sigma_yz = comps[k++];
    } else {
      s->sigma_xy = s->sigma_xz = s->sigma_yz = 0.0f;
    }
  }
  s->mu_t = comps[k++];
  s->sigma_t = comps[k++];
  s->r = comps[k++];
  s->g = comps[k++];
  s->b = comps[k++];
  s->alpha = comps[k++];
}

typedef enum {
  SPLAT_INDEX_OK = 0,
  SPLAT_INDEX_OUT_OF_RANGE = 1, // references a palette slot that does not exist
  SPLAT_INDEX_TOO_WIDE = 2,     // does not fit the selected index width
} SplatIndexCheck;

// Verify that every entry is a valid palette reference (< pSize) and fits the
// selected index width, so that packing to that width will not silently
// truncate the value. On failure *bad_pos (when non-NULL) receives the offset
// of the first offending entry.
static SplatIndexCheck check_index_values(const uint64_t *indices, uint64_t count, uint32_t pSize,
                                          uint8_t idx_width, uint64_t *bad_pos) {
  if (!indices)
    return SPLAT_INDEX_OUT_OF_RANGE;

  uint64_t max_repr = index_width_max_value(idx_width);
  for (uint64_t i = 0; i < count; ++i) {
    if (indices[i] >= pSize) {
      if (bad_pos)
        *bad_pos = i;
      return SPLAT_INDEX_OUT_OF_RANGE;
    }
    if (indices[i] > max_repr) {
      if (bad_pos)
        *bad_pos = i;
      return SPLAT_INDEX_TOO_WIDE;
    }
  }
  return SPLAT_INDEX_OK;
}

static bool splat4d_stream_video_payload(const Splat4DVideo *v, size_t chunk, Splat4DChunkFn fn,
                                         void *ctx) {
  if (!v || !fn)
    return false;

  // Stream the header in its on-disk form so the checksum covers exactly the
  // bytes written to the file.
  uint8_t header_bytes[SPLAT_HEADER_DISK_BYTES];
  serialize_header(&v->header, header_bytes);
  if (!splat4d_stream_block(header_bytes, sizeof header_bytes, chunk, fn, ctx))
    return false;

  size_t entry_bytes = palette_entry_disk_bytes(v->header.flags);
  uint64_t palette_bytes_u64;
  if (!checked_mul_u64((uint64_t)v->header.pSize, entry_bytes, &palette_bytes_u64))
    return false;
  if (palette_bytes_u64 > SIZE_MAX)
    return false;

  size_t palette_bytes = (size_t)palette_bytes_u64;
  if (palette_bytes > 0) {
    if (!v->palette.palette)
      return false;
    // The on-disk palette is always serialized (spec field order + per-shape
    // covariance + precision), so build it into a buffer and stream that.
    uint8_t *packed = malloc(palette_bytes);
    if (!packed)
      return false;
    for (uint32_t i = 0; i < v->header.pSize; ++i)
      serialize_palette_entry(&v->palette.palette[i], v->header.flags,
                              packed + (size_t)i * entry_bytes);
    bool ok = splat4d_stream_block(packed, palette_bytes, chunk, fn, ctx);
    free(packed);
    if (!ok)
      return false;
  }

  uint64_t total = header_total_indices(&v->header);
  uint8_t idx_width = get_index_width_bytes(v->header.flags);
  uint64_t index_bytes_u64;

  if (!checked_mul_u64(total, idx_width, &index_bytes_u64))
    return false;
  if (index_bytes_u64 > SIZE_MAX)
    return false;

  size_t index_bytes = (size_t)index_bytes_u64;
  if (index_bytes > 0) {
    if (!v->index.index)
      return false;

    if (idx_width == 8) {
      if (!splat4d_stream_block((const uint8_t *)v->index.index, index_bytes, chunk, fn, ctx))
        return false;
    } else {
      // Pack the 64-bit array down to the requested width dynamically in chunks
      uint8_t pack_buf[SPLAT4D_STREAM_CHUNK_SIZE];
      uint64_t items_per_chunk = SPLAT4D_STREAM_CHUNK_SIZE / idx_width;
      uint64_t items_streamed = 0;

      while (items_streamed < total) {
        uint64_t to_pack = total - items_streamed;
        if (to_pack > items_per_chunk)
          to_pack = items_per_chunk;

        // ⚡ Bolt Optimization: Use restrict pointers to guarantee no aliasing,
        // allowing better vectorization of these packing loops.
        const uint64_t *restrict src = v->index.index + items_streamed;
        if (idx_width == 1) {
          uint8_t *restrict p = (uint8_t *)pack_buf;
          for (uint64_t i = 0; i < to_pack; i++)
            p[i] = (uint8_t)src[i];
        } else if (idx_width == 2) {
          uint16_t *restrict p = (uint16_t *)pack_buf;
          for (uint64_t i = 0; i < to_pack; i++)
            p[i] = (uint16_t)src[i];
        } else if (idx_width == 4) {
          uint32_t *restrict p = (uint32_t *)pack_buf;
          for (uint64_t i = 0; i < to_pack; i++)
            p[i] = (uint32_t)src[i];
        }

        uint64_t packed_bytes;
        if (!checked_mul_u64(to_pack, idx_width, &packed_bytes))
          return false;
        if (packed_bytes > SIZE_MAX)
          return false;

        if (!fn(pack_buf, (size_t)packed_bytes, ctx))
          return false;
        items_streamed += to_pack;
      }
    }
  }

  return true;
}

bool stream_splat4DVideo(const Splat4DVideo *v, size_t chunk, Splat4DChunkFn fn, void *ctx) {
  return splat4d_stream_video_payload(v, chunk, fn, ctx);
}

static bool splat4d_crc32_consumer(const uint8_t *chunk, size_t n, void *ctx) {
  crc32_t *c = ctx;
  crc32_update(c, chunk, n);
  return true;
}

typedef struct {
  FILE *fp;
  crc32_t *crc;
} Splat4DStreamFileCtx;

static bool splat4d_stream_file_consumer(const uint8_t *chunk, size_t n, void *ctx) {
  Splat4DStreamFileCtx *state = ctx;
  if (fwrite(chunk, 1, n, state->fp) != n)
    return false;
  if (state->crc)
    crc32_update(state->crc, chunk, n);
  return true;
}

uint32_t compute_video_checksum(const Splat4DVideo *v) {
  if (!v)
    return 0;

  crc32_t c;
  crc32_init(&c);
  if (!stream_splat4DVideo(v, SPLAT4D_STREAM_CHUNK_SIZE, splat4d_crc32_consumer, &c))
    return 0;
  return crc32_final(&c);
}

uint64_t compute_idxoffset_forward(const Splat4DHeader *h) {
  return (uint64_t)sizeof(Splat4DHeader) +
         (uint64_t)h->pSize * (uint64_t)palette_entry_disk_bytes(h->flags);
}

uint64_t compute_idxoffset_reverse(const Splat4DHeader *h) {
  uint64_t total = header_total_indices(h);
  uint64_t header_bytes = sizeof(Splat4DHeader);
  uint64_t palette_bytes = (uint64_t)h->pSize * (uint64_t)palette_entry_disk_bytes(h->flags);
  uint8_t idx_width = get_index_width_bytes(h->flags);
  uint64_t index_bytes = total * idx_width;
  uint64_t footer_bytes = SPLAT_FOOTER_DISK_BYTES;
  uint64_t filesize = header_bytes + palette_bytes + index_bytes + footer_bytes;
  uint64_t offset = filesize - (footer_bytes + index_bytes);
  return offset;
}

bool sanity_check_idxoffset_file(FILE *fp, const Splat4DHeader *h, const Splat4DFooter *f) {
  (void)fp;
  uint64_t expect = (uint64_t)sizeof(Splat4DHeader) +
                    (uint64_t)h->pSize * (uint64_t)palette_entry_disk_bytes(h->flags);
  return f->idxoffset == expect;
}

bool check_idxoffset_file(FILE *fp, const Splat4DHeader *h, const Splat4DFooter *f) {
  (void)fp;
  uint64_t after_header = (uint64_t)sizeof(Splat4DHeader) +
                          (uint64_t)h->pSize * (uint64_t)palette_entry_disk_bytes(h->flags);
  return after_header == (uint64_t)f->idxoffset;
}

// splat
Splat4D create_splat4D(float mu_x, float sigma_x, float mu_y, float sigma_y, float mu_z,
                       float sigma_z, float mu_t, float sigma_t, float r, float g, float b,
                       float alpha) {
  return (Splat4D){.mu_x = mu_x,
                   .sigma_x = sigma_x,
                   .mu_y = mu_y,
                   .sigma_y = sigma_y,
                   .mu_z = mu_z,
                   .sigma_z = sigma_z,
                   .mu_t = mu_t,
                   .sigma_t = sigma_t,
                   .r = r,
                   .g = g,
                   .b = b,
                   .alpha = alpha};
}

void print_splat4D(const Splat4D *s, const uint32_t count) {
  printf("│ ╭ Splat %010u ──────╮ │\n", count);
  printf("│ │         mu    sigma    │ │\n");
  printf("│ │ x %8.2f %8.2f    │ │\n", s->mu_x, s->sigma_x);
  printf("│ │ y %8.2f %8.2f    │ │\n", s->mu_y, s->sigma_y);
  printf("│ │ z %8.2f %8.2f    │ │\n", s->mu_z, s->sigma_z);
  printf("│ │ t %8.2f %8.2f    │ │\n", s->mu_t, s->sigma_t);
  if (s->sigma_xy != 0.0f || s->sigma_xz != 0.0f || s->sigma_yz != 0.0f)
    printf("│ │ cov %6.2f %6.2f %6.2f│ │\n", s->sigma_xy, s->sigma_xz, s->sigma_yz);
  printf("│ │ r %-18.2f   │ │\n", s->r);
  printf("│ │ g %-18.2f   │ │\n", s->g);
  printf("│ │ b %-18.2f   │ │\n", s->b);
  printf("│ │ α %-18.2f   │ │\n", s->alpha);
  printf("│ ╰────────────────────────╯ │\n");
}

// header
Splat4DHeader create_splat4DHeader(uint32_t width, uint32_t height, uint32_t depth, uint32_t frames,
                                   uint32_t pSize, uint32_t flags) {
  uint32_t sanitized = sanitize_flags(flags);
  return (Splat4DHeader){.magic = 0x3453504C,
                         .version = {1, 1, 0, 0}, // v0.2 palette layout
                         .width = width,
                         .height = height,
                         .depth = depth,
                         .frames = frames,
                         .pSize = pSize,
                         .flags = sanitized};
}

static const char *precision_name(uint32_t precision) {
  static const char *names[] = {"Float16", "Float32", "Float64", "Float128"};
  if (precision < (sizeof names / sizeof names[0]))
    return names[precision];
  return "Reserved";
}

static const char *compression_name(uint32_t compression) {
  static const char *names[] = {"None",    "Run Length Encoding",
                                "DEFLATE", "RAR",
                                "LZO",     "Zlib",
                                "bzip2",   "LZMA",
                                "ZPAQ",    "XZ",
                                "LZ4",     "Snappy",
                                "LZHAM",   "Brotli",
                                "LZFSE",   "Zstd"};
  if (compression < (sizeof names / sizeof names[0]))
    return names[compression];
  return "Reserved";
}

static const char *index_width_name(uint32_t width) {
  static const char *names[] = {"1 byte", "2 bytes", "4 bytes", "8 bytes"};
  if (width < (sizeof names / sizeof names[0]))
    return names[width];
  return "Reserved";
}

static const char *splat_shape_label(uint32_t shape) {
  static const char *names[] = {"Isotropic (1σ)", "Axis-Aligned", "Full Covariance", "Reserved"};
  if (shape < (sizeof names / sizeof names[0]))
    return names[shape];
  return "Reserved";
}

static const char *color_space_name(uint32_t color_space) {
  static const char *names[] = {"sRGB",         "Linear sRGB", "OKLab",   "Display P3",
                                "Rec.709",      "Rec.2020",    "DCI-P3",  "ACES-AP0",
                                "ProPhoto RGB", "Rec.2100",    "CIE Lab", "CIE XYZ D65",
                                "ACEScg-AP1",   "Rec.601",     "XYZ D50", "XYZ D65"};
  if (color_space < (sizeof names / sizeof names[0]))
    return names[color_space];
  return "Reserved";
}

static const char *interpolation_name(uint32_t interpolation) {
  static const char *names[] = {"None",
                                "Nearest Neighbor",
                                "Axis-Aligned",
                                "Smooth",
                                "Lanczos",
                                "Gaussian",
                                "Catmull-Rom",
                                "NURBS",
                                "Radial Basis Fn",
                                "Optical Flow",
                                "Neural",
                                "Akima Splines",
                                "Inverse Distance",
                                "Fourier",
                                "Moving Least Sq",
                                "Cubic Hermite"};
  if (interpolation < (sizeof names / sizeof names[0]))
    return names[interpolation];
  return "Reserved";
}

static void print_flag_line(FILE *out, const char *label, const char *value) {
  if (!out || !label || !value)
    return;

  char buffer[128];
  int written = SAFE_SNPRINTF(buffer, sizeof buffer, "  %-12s %s", label, value);
  if (written < 0)
    return;

  size_t len = (size_t)written;
  const size_t width = 27; // width of the content between the │ characters

  fputs("│", out);
  fputs(buffer, out);
  if (len < width) {
    static const char spaces[] = "                           "; // 27 spaces
    size_t pad = width - len;
    if (pad <= 27)
      fputs(spaces + 27 - pad, out);
    else
      for (size_t i = len; i < width; ++i)
        fputc(' ', out);
  }
  fputs(" │\n", out);
}

void print_flags(uint32_t flags) {
  const char *endian = (flags & 0x1u) ? "Big-Endian" : "Little-Endian";
  const char *sorted = (flags & 0x2u) ? "Yes" : "No";
  uint32_t precision_bits = (flags >> 2) & 0x3u;
  uint32_t compression_bits = (flags >> 4) & 0xFu;
  uint32_t index_width_bits = (flags >> 8) & 0x3u;
  uint32_t splat_shape_bits = (flags >> 10) & 0x3u;
  uint32_t color_space_bits = (flags >> 12) & 0xFu;
  uint32_t interpolation_bits = (flags >> 16) & 0xFu;
  uint32_t encryption_bits = (flags >> 20) & 0xFu;
  uint32_t metadata_bits = (flags >> 24) & 0xFFu;

  char buffer[32];

  print_flag_line(stdout, "endian", endian);
  print_flag_line(stdout, "sorted", sorted);

  print_flag_line(stdout, "precision", precision_name(precision_bits));
  print_flag_line(stdout, "compression", compression_name(compression_bits));
  print_flag_line(stdout, "index width", index_width_name(index_width_bits));
  print_flag_line(stdout, "splat shape", splat_shape_label(splat_shape_bits));
  print_flag_line(stdout, "color space", color_space_name(color_space_bits));
  print_flag_line(stdout, "interp", interpolation_name(interpolation_bits));

  SAFE_SNPRINTF(buffer, sizeof buffer, "0x%X", encryption_bits);
  print_flag_line(stdout, "encryption", buffer);

  SAFE_SNPRINTF(buffer, sizeof buffer, "0x%02X", metadata_bits);
  print_flag_line(stdout, "metadata", buffer);
}

void print_splat4DHeader(const Splat4DHeader *h) {
  printf("│  magic        0X%-10X │\n", h->magic);
  printf("│  version      %02d.%02d.%02d-%02d  │\n", h->version[0], h->version[1], h->version[2],
         h->version[3]);
  printf("│  width        0X%-10X │\n", h->width);
  printf("│  height       0X%-10X │\n", h->height);
  printf("│  depth        0X%-10X │\n", h->depth);
  printf("│  frames       0X%-10X │\n", h->frames);
  printf("│  palette size 0X%-10X │\n", h->pSize);
  printf("│  flags        0X%-10X │\n", h->flags);
  Splat4DFlags decoded = splat4d_flags_from_raw(h->flags);
  printf("│    endian     %-14s │\n", splat_endian_name((SplatEndian)decoded.bits.endian));
  printf("│    sort       %-14s │\n", splat_sort_order_name((SplatSortOrder)decoded.bits.sorted));
  printf("│    precision  %-14s │\n", splat_precision_name((SplatPrecision)decoded.bits.precision));
  printf("│    compress   %-14s │\n",
         splat_compression_name((SplatCompression)decoded.bits.compression));
  printf("│    index      %-14s │\n",
         splat_index_width_name((SplatIndexWidth)decoded.bits.index_width));
  printf("│    shape      %-14s │\n", splat_shape_name((SplatShape)decoded.bits.splat_shape));
  printf("│    color      %-14s │\n",
         splat_color_space_name((SplatColorSpace)decoded.bits.color_space));
  printf("│    interp     %-14s │\n",
         splat_interpolation_name((SplatInterpolation)decoded.bits.interpolation));
  print_flags(h->flags);
  printf("│  idxs %-20" PRIu64 " │\n", header_total_indices(h));
  printf("│                            │\n");
}

bool write_splat4DHeader(FILE *fp, const Splat4DHeader *h) {
  if (!fp || !h)
    return false;
  uint8_t buf[SPLAT_HEADER_DISK_BYTES];
  serialize_header(h, buf);
  return fwrite(buf, 1, sizeof buf, fp) == sizeof buf;
}

bool read_splat4DHeader(FILE *fp, Splat4DHeader *h) {
  if (!fp || !h)
    return false;
  uint8_t buf[SPLAT_HEADER_DISK_BYTES];
  if (fread(buf, 1, sizeof buf, fp) != sizeof buf)
    return false;
  deserialize_header(buf, h);
  return true;
}

// palette
Splat4DPalette create_splat4DPalette(Splat4D *p) { return (Splat4DPalette){.palette = p}; }

void print_splat4DPalette(const Splat4DVideo *v) {
  if (!v->palette.palette)
    return;
  printf("├ Palette (%8u) ──      │\n", v->header.pSize);
  for (uint32_t i = 0; i < v->header.pSize; i++)
    print_splat4D(&v->palette.palette[i], i);
}

bool write_splat4DPalette(FILE *fp, const Splat4DPalette *p, uint32_t count, uint32_t flags) {
  if (!fp || !p || !p->palette || count == 0)
    return false;

  size_t entry_bytes = palette_entry_disk_bytes(flags);
  uint64_t bytes64 = 0;
  if (!checked_mul_u64((uint64_t)count, (uint64_t)entry_bytes, &bytes64) || bytes64 > SIZE_MAX)
    return false;
  uint8_t *packed = malloc((size_t)bytes64);
  if (!packed)
    return false;
  for (uint32_t i = 0; i < count; ++i)
    serialize_palette_entry(&p->palette[i], flags, packed + (size_t)i * entry_bytes);
  bool ok = fwrite(packed, 1, (size_t)bytes64, fp) == (size_t)bytes64;
  free(packed);
  return ok;
}

bool read_splat4DPalette(FILE *fp, Splat4DPalette *p, uint32_t count, uint32_t flags) {
  if (!fp || !p || count == 0)
    return false;

  // The in-memory palette is always full-precision Splat4D; the on-disk entries
  // may be narrower, so allocate for the former and read/convert the latter.
  uint64_t mem_bytes = 0;
  if (!checked_mul_u64((uint64_t)count, (uint64_t)sizeof(Splat4D), &mem_bytes) ||
      mem_bytes > SIZE_MAX)
    return false;
  p->palette = malloc((size_t)mem_bytes);
  if (!p->palette)
    return false;

  size_t entry_bytes = palette_entry_disk_bytes(flags);
  uint64_t disk_bytes = 0;
  if (!checked_mul_u64((uint64_t)count, (uint64_t)entry_bytes, &disk_bytes) ||
      disk_bytes > SIZE_MAX) {
    free(p->palette);
    p->palette = NULL;
    return false;
  }
  uint8_t *packed = malloc((size_t)disk_bytes);
  if (!packed) {
    free(p->palette);
    p->palette = NULL;
    return false;
  }
  if (fread(packed, 1, (size_t)disk_bytes, fp) != (size_t)disk_bytes) {
    free(packed);
    free(p->palette);
    p->palette = NULL;
    return false;
  }
  for (uint32_t i = 0; i < count; ++i)
    deserialize_palette_entry(packed + (size_t)i * entry_bytes, flags, &p->palette[i]);
  free(packed);
  return true;
}

// index
Splat4DIndex create_splat4DIndex(uint64_t *i) { return (Splat4DIndex){.index = i}; }

void print_splat4DIndex(const Splat4DVideo *v) {
  if (!v->index.index)
    return;
  uint64_t total = header_total_indices(&v->header);
  printf("├ Index (%8" PRIu64 ") ────      │\n", total);
  uint64_t n = total < 8 ? total : 8;
  for (uint64_t i = 0; i < n; i++) {
    printf("│   [%" PRIu64 "] %-20" PRIu64 " │\n", i, v->index.index[i]);
  }
  if (total > n) {
    printf("│   ... (%" PRIu64 " more)          │\n", total - n);
  }
}

// Pack the 64-bit index array down to idx_width bytes/entry into `out`
// (malloc'd, so suitably aligned for the wider element writes).
static void pack_index_to_buffer(const uint64_t *src, uint64_t total, uint8_t idx_width,
                                 uint8_t *out) {
  if (idx_width == 8) {
    memcpy(out, src, (size_t)total * 8);
  } else if (idx_width == 1) {
    for (uint64_t k = 0; k < total; k++)
      out[k] = (uint8_t)src[k];
  } else if (idx_width == 2) {
    uint16_t *p = (uint16_t *)out;
    for (uint64_t k = 0; k < total; k++)
      p[k] = (uint16_t)src[k];
  } else {
    uint32_t *p = (uint32_t *)out;
    for (uint64_t k = 0; k < total; k++)
      p[k] = (uint32_t)src[k];
  }
}

static void unpack_index_from_buffer(const uint8_t *in, uint64_t total, uint8_t idx_width,
                                     uint64_t *out) {
  if (idx_width == 8) {
    memcpy(out, in, (size_t)total * 8);
  } else if (idx_width == 1) {
    for (uint64_t k = 0; k < total; k++)
      out[k] = in[k];
  } else if (idx_width == 2) {
    const uint16_t *p = (const uint16_t *)in;
    for (uint64_t k = 0; k < total; k++)
      out[k] = p[k];
  } else {
    const uint32_t *p = (const uint32_t *)in;
    for (uint64_t k = 0; k < total; k++)
      out[k] = p[k];
  }
}

bool write_splat4DIndex(FILE *fp, const Splat4DIndex *i, uint64_t total, uint32_t flags) {
  if (!fp || !i || !i->index || total == 0)
    return false;

  uint8_t idx_width = get_index_width_bytes(flags);
  if (idx_width == 8) {
    return fwrite(i->index, sizeof(uint64_t), total, fp) == total;
  } else {
    uint8_t pack_buf[SPLAT4D_STREAM_CHUNK_SIZE];
    uint64_t items_per_chunk = SPLAT4D_STREAM_CHUNK_SIZE / idx_width;
    uint64_t items_written = 0;
    while (items_written < total) {
      uint64_t to_pack = total - items_written;
      if (to_pack > items_per_chunk)
        to_pack = items_per_chunk;

      // ⚡ Bolt Optimization: Use restrict pointers to guarantee no aliasing,
      // allowing better vectorization of these packing loops.
      const uint64_t *restrict src = i->index + items_written;
      if (idx_width == 1) {
        uint8_t *restrict p = (uint8_t *)pack_buf;
        for (uint64_t k = 0; k < to_pack; k++)
          p[k] = (uint8_t)src[k];
      } else if (idx_width == 2) {
        uint16_t *restrict p = (uint16_t *)pack_buf;
        for (uint64_t k = 0; k < to_pack; k++)
          p[k] = (uint16_t)src[k];
      } else if (idx_width == 4) {
        uint32_t *restrict p = (uint32_t *)pack_buf;
        for (uint64_t k = 0; k < to_pack; k++)
          p[k] = (uint32_t)src[k];
      }

      if (fwrite(pack_buf, idx_width, (size_t)to_pack, fp) != (size_t)to_pack)
        return false;
      items_written += to_pack;
    }
  }
  return true;
}

bool read_splat4DIndex(FILE *fp, Splat4DIndex *i, uint64_t total, uint32_t flags) {
  if (!fp || !i || total == 0)
    return false;

  uint64_t bytes64 = 0;
  if (!checked_mul_u64(total, (uint64_t)sizeof(uint64_t), &bytes64) || bytes64 > SIZE_MAX)
    return false;
  size_t bytes = (size_t)bytes64;
  i->index = malloc(bytes);
  if (!i->index)
    return false;

  uint8_t idx_width = get_index_width_bytes(flags);
  if (idx_width == 8) {
    size_t total_count = (size_t)total;
    if (fread(i->index, sizeof(uint64_t), total_count, fp) != total_count) {
      free(i->index);
      i->index = NULL;
      return false;
    }
  } else {
    uint8_t pack_buf[SPLAT4D_STREAM_CHUNK_SIZE];
    uint64_t items_per_chunk = SPLAT4D_STREAM_CHUNK_SIZE / idx_width;
    uint64_t items_read = 0;
    while (items_read < total) {
      uint64_t to_read = total - items_read;
      if (to_read > items_per_chunk)
        to_read = items_per_chunk;

      if (fread(pack_buf, idx_width, (size_t)to_read, fp) != (size_t)to_read) {
        free(i->index);
        i->index = NULL;
        return false;
      }

      // ⚡ Bolt Optimization: Use restrict pointers to guarantee no aliasing,
      // allowing better vectorization of these unpacking loops.
      uint64_t *restrict dst = i->index + items_read;
      if (idx_width == 1) {
        uint8_t *restrict p = (uint8_t *)pack_buf;
        for (uint64_t k = 0; k < to_read; k++)
          dst[k] = p[k];
      } else if (idx_width == 2) {
        uint16_t *restrict p = (uint16_t *)pack_buf;
        for (uint64_t k = 0; k < to_read; k++)
          dst[k] = p[k];
      } else if (idx_width == 4) {
        uint32_t *restrict p = (uint32_t *)pack_buf;
        for (uint64_t k = 0; k < to_read; k++)
          dst[k] = p[k];
      }
      items_read += to_read;
    }
  }
  return true;
}

// footer
Splat4DFooter create_splat4DFooter(const Splat4DHeader *h) {
  return (Splat4DFooter){
      .checksum = 0,
      .idxoffset = (uint64_t)sizeof(Splat4DHeader) +
                   (uint64_t)h->pSize * (uint64_t)palette_entry_disk_bytes(h->flags),
      .end = 0x4C505334 // "LPS4"
  };
}

void print_splat4DFooter(const Splat4DFooter *f) {
  printf("├ Footer ──────────────      │\n");
  printf("│    checksum  0x%08X    │\n", f->checksum);
  printf("│    idxoffset 0x%08" PRIX64 "   │\n", f->idxoffset);
  printf("│    end       0x%08X    │\n", f->end);
}

bool write_splat4DFooter(FILE *fp, const Splat4DFooter *f) {
  if (!fp || !f)
    return false;
  uint8_t buf[SPLAT_FOOTER_DISK_BYTES];
  serialize_footer(f, buf);
  return fwrite(buf, 1, sizeof buf, fp) == sizeof buf;
}

bool read_splat4DFooter(FILE *fp, Splat4DFooter *f) {
  if (!fp || !f)
    return false;
  uint8_t buf[SPLAT_FOOTER_DISK_BYTES];
  if (fread(buf, 1, sizeof buf, fp) != sizeof buf)
    return false;
  deserialize_footer(buf, f);
  return true;
}

// video
Splat4DVideo create_splat4DVideo(const Splat4DHeader header, Splat4D *splats, uint64_t *idxs) {
  Splat4DVideo v = {.header = header,
                    .palette = create_splat4DPalette(splats),
                    .index = create_splat4DIndex(idxs),
                    .footer = create_splat4DFooter(&header)};

  v.footer.checksum = compute_video_checksum(&v);
  return v;
}

void print_splat4DVideo(const Splat4DVideo *v) {
  printf("╭─────── 4Splat Video ───────╮\n");
  print_splat4DHeader(&v->header);
  print_splat4DPalette(v);
  print_splat4DIndex(v);
  print_splat4DFooter(&v->footer);
  printf("╰────────────────────────────╯\n");
}

// Pack the index to the header's index width, compress it with `codec` and
// write the compressed bytes. Used for the on-disk index section whenever the
// compression field is not None.
static bool write_index_compressed(FILE *fp, const Splat4DVideo *v, uint32_t codec) {
  uint64_t total = header_total_indices(&v->header);
  uint8_t idx_width = get_index_width_bytes(v->header.flags);
  uint64_t packed64;
  if (!checked_mul_u64(total, idx_width, &packed64) || packed64 > SIZE_MAX)
    return false;
  size_t packed_len = (size_t)packed64;

  uint8_t *packed = malloc(packed_len ? packed_len : 1);
  if (!packed)
    return false;
  pack_index_to_buffer(v->index.index, total, idx_width, packed);

  size_t clen = 0;
  uint8_t *comp = splat_compress(codec, packed, packed_len, &clen);
  free(packed);
  if (!comp)
    return false;

  bool ok = fwrite(comp, 1, clen, fp) == clen;
  free(comp);
  return ok;
}

// Read `comp_len` compressed bytes, decompress to the exact index-payload size
// and unpack into a freshly allocated 64-bit index array.
static bool read_index_compressed(FILE *fp, Splat4DIndex *idx, uint64_t total, uint32_t flags,
                                  size_t comp_len, uint32_t codec) {
  uint8_t idx_width = get_index_width_bytes(flags);
  uint64_t packed64;
  if (!checked_mul_u64(total, idx_width, &packed64) || packed64 > SIZE_MAX)
    return false;
  size_t packed_len = (size_t)packed64;

  uint8_t *cbuf = malloc(comp_len ? comp_len : 1);
  if (!cbuf)
    return false;
  if (fread(cbuf, 1, comp_len, fp) != comp_len) {
    free(cbuf);
    return false;
  }

  uint8_t *packed = malloc(packed_len ? packed_len : 1);
  if (!packed) {
    free(cbuf);
    return false;
  }
  bool ok = splat_decompress(codec, cbuf, comp_len, packed, packed_len);
  free(cbuf);
  if (!ok) {
    free(packed);
    return false;
  }

  uint64_t mem64;
  if (!checked_mul_u64(total, (uint64_t)sizeof(uint64_t), &mem64) || mem64 > SIZE_MAX) {
    free(packed);
    return false;
  }
  idx->index = malloc((size_t)mem64);
  if (!idx->index) {
    free(packed);
    return false;
  }
  unpack_index_from_buffer(packed, total, idx_width, idx->index);
  free(packed);
  return true;
}

bool write_splat4DVideo(FILE *fp, Splat4DVideo *v) {
  if (!fp || !v)
    return false;

  // Compute header-derived values
  v->footer.idxoffset =
      (uint64_t)sizeof(Splat4DHeader) +
      (uint64_t)v->header.pSize * (uint64_t)palette_entry_disk_bytes(v->header.flags);

  uint32_t codec = (v->header.flags & SPLAT_FLAG_COMPRESSION_MASK) >> SPLAT_FLAG_COMPRESSION_SHIFT;

  if (codec == SPLAT_COMPRESSION_NONE) {
    // Uncompressed: the on-disk bytes equal the logical payload, so stream them
    // straight to the file while accumulating the checksum.
    crc32_t c;
    crc32_init(&c);
    Splat4DStreamFileCtx ctx = {.fp = fp, .crc = &c};
    if (!stream_splat4DVideo(v, SPLAT4D_STREAM_CHUNK_SIZE, splat4d_stream_file_consumer, &ctx))
      return false;
    v->footer.checksum = crc32_final(&c);
  } else {
    // Compressed: the checksum covers the logical (uncompressed) payload so it
    // is independent of the codec's byte output, while only the index section is
    // physically compressed on disk.
    v->footer.checksum = compute_video_checksum(v);
    if (!write_splat4DHeader(fp, &v->header))
      return false;
    if (v->header.pSize > 0 &&
        !write_splat4DPalette(fp, &v->palette, v->header.pSize, v->header.flags))
      return false;
    if (!write_index_compressed(fp, v, codec))
      return false;
  }

  // Return to end of file and write footer
  fseek(fp, 0, SEEK_END);
  if (!write_splat4DFooter(fp, &v->footer))
    return false;

  return true;
}

bool read_splat4DVideo(FILE *fp, Splat4DVideo *v) {
  if (!fp || !v)
    return false;

  // Null the owned pointers up front so every early-return path leaves the
  // caller's struct in a consistent (freeable) state.
  v->palette.palette = NULL;
  v->index.index = NULL;

  // Read header
  if (!read_splat4DHeader(fp, &v->header))
    return false;

  // Reject files that are not 4Splat containers before sizing any allocation
  // from attacker-controlled header dimensions.
  if (v->header.magic != 0x3453504C) {
    LOG_ERROR("❌ Unsupported format\n");
    return false;
  }
  if (v->header.version[0] != 1) {
    LOG_ERROR("❌ Unsupported version\n");
    return false;
  }

  if (!flags_supported(v->header.flags))
    return false;
  if (v->header.pSize == 0) {
    LOG_ERROR("❌ Invalid palette size\n");
    return false;
  }

  uint64_t palette_bytes = 0;
  if (!checked_mul_u64((uint64_t)v->header.pSize,
                       (uint64_t)palette_entry_disk_bytes(v->header.flags), &palette_bytes) ||
      palette_bytes > SIZE_MAX) {
    LOG_ERROR("❌ Invalid palette size\n");
    return false;
  }

  uint64_t total = 0;
  if (!header_total_indices_checked(&v->header, &total)) {
    LOG_ERROR("❌ Invalid index count\n");
    return false;
  }

  uint64_t index_bytes = 0;
  if (!checked_mul_u64(total, (uint64_t)sizeof(uint64_t), &index_bytes) || index_bytes > SIZE_MAX) {
    LOG_ERROR("❌ Invalid index count\n");
    return false;
  }

  uint32_t codec = (v->header.flags & SPLAT_FLAG_COMPRESSION_MASK) >> SPLAT_FLAG_COMPRESSION_SHIFT;

  // Reject headers whose declared sections cannot fit the actual file, before
  // allocating anything sized from those (attacker-controlled) dimensions.
  long after_header = ftell(fp);
  if (after_header < 0 || fseek(fp, 0, SEEK_END) != 0)
    return false;
  long file_end = ftell(fp);
  if (file_end < 0 || fseek(fp, after_header, SEEK_SET) != 0)
    return false;
  uint64_t filesize = (uint64_t)file_end;

  uint64_t base = (uint64_t)SPLAT_HEADER_DISK_BYTES + SPLAT_FOOTER_DISK_BYTES;
  if (palette_bytes > filesize || filesize - palette_bytes < base) {
    LOG_ERROR("❌ Palette does not fit the file\n");
    return false;
  }
  uint64_t index_room = filesize - palette_bytes - base; // on-disk bytes for the index
  uint64_t ondisk_index = 0;
  if (!checked_mul_u64(total, (uint64_t)get_index_width_bytes(v->header.flags), &ondisk_index)) {
    LOG_ERROR("❌ Invalid index count\n");
    return false;
  }
  if (codec == SPLAT_COMPRESSION_NONE) {
    if (ondisk_index > index_room) {
      LOG_ERROR("❌ Index does not fit the file\n");
      return false;
    }
  } else if (ondisk_index > SPLAT_MAX_COMPRESSED_INDEX_BYTES) {
    LOG_ERROR("❌ Compressed index would decompress beyond the size limit\n");
    return false;
  }

  // Read palette
  if (!read_splat4DPalette(fp, &v->palette, v->header.pSize, v->header.flags))
    return false;

  // Read index
  if (codec == SPLAT_COMPRESSION_NONE) {
    if (!read_splat4DIndex(fp, &v->index, total, v->header.flags)) {
      free(v->palette.palette);
      v->palette.palette = NULL;
      return false;
    }
  } else {
    // The compressed index section runs from the current position (the index
    // offset) up to the fixed-size footer at end of file.
    long index_start = ftell(fp);
    if (index_start < 0 || fseek(fp, 0, SEEK_END) != 0) {
      free(v->palette.palette);
      v->palette.palette = NULL;
      return false;
    }
    long file_end = ftell(fp);
    if (file_end < 0 || fseek(fp, index_start, SEEK_SET) != 0 ||
        file_end < index_start + (long)SPLAT_FOOTER_DISK_BYTES) {
      LOG_ERROR("❌ Truncated compressed index\n");
      free(v->palette.palette);
      v->palette.palette = NULL;
      return false;
    }
    size_t comp_len = (size_t)(file_end - index_start) - SPLAT_FOOTER_DISK_BYTES;
    if (!read_index_compressed(fp, &v->index, total, v->header.flags, comp_len, codec)) {
      LOG_ERROR("❌ Failed to decompress index\n");
      free(v->palette.palette);
      v->palette.palette = NULL;
      return false;
    }
  }

  // Read footer
  if (!read_splat4DFooter(fp, &v->footer)) {
    free(v->palette.palette);
    free(v->index.index);
    v->palette.palette = NULL;
    v->index.index = NULL;
    return false;
  }

  // ---- Validate footer ----
  // 1. Recompute CRC from in-memory payload
  uint32_t recomputed = compute_video_checksum(v);
  if (recomputed != v->footer.checksum) {
    LOG_ERROR("❌ CRC mismatch: file=0x%08X recomputed=0x%08X\n", v->footer.checksum, recomputed);
    free(v->palette.palette);
    free(v->index.index);
    v->palette.palette = NULL;
    v->index.index = NULL;
    return false;
  }

  // 2. Validate offset consistency
  if (!sanity_check_idxoffset_file(fp, &v->header, &v->footer)) {
    LOG_ERROR("❌ Index offset mismatch (footer=%" PRIu64 ", expect=%" PRIu64 ")\n",
              (uint64_t)v->footer.idxoffset,
              (uint64_t)sizeof(Splat4DHeader) +
                  (uint64_t)v->header.pSize * (uint64_t)palette_entry_disk_bytes(v->header.flags));
    // free allocations before returning
    free(v->palette.palette);
    free(v->index.index);
    v->palette.palette = NULL;
    v->index.index = NULL;
    return false;
  }

  // 3. Validate footer end marker
  if (v->footer.end != 0x4C505334) {
    LOG_ERROR("❌ Invalid footer end marker\n");
    free(v->palette.palette);
    free(v->index.index);
    v->palette.palette = NULL;
    v->index.index = NULL;
    return false;
  }

  return true;
}

bool validate_splat4DVideo(const Splat4DVideo *v) {
  if (!v) {
    LOG_ERROR("❌ Video reference required\n");
    return false;
  }

  if (v->header.magic != 0x3453504C) {
    LOG_ERROR("❌ Unsupported format\n");
    return false;
  }

  if (v->header.version[0] != 1) {
    LOG_ERROR("❌ Unsupported version\n");
    return false;
  }

  if (v->header.height == 0) {
    LOG_ERROR("❌ Positive height required\n");
    return false;
  }

  if (v->header.width == 0) {
    LOG_ERROR("❌ Positive width required\n");
    return false;
  }

  if (v->header.depth == 0) {
    LOG_ERROR("❌ Positive depth required\n");
    return false;
  }

  if (v->header.frames == 0) {
    LOG_ERROR("❌ Positive number of frames required\n");
    return false;
  }

  if (v->header.pSize == 0) {
    LOG_ERROR("❌ Positive palette size required\n");
    return false;
  }

  if (!flags_supported(v->header.flags))
    return false;

  if (v->footer.end != 0x4C505334) {
    LOG_ERROR("❌ Invalid end-of-file marker\n");
    return false;
  }

  uint32_t expected = compute_video_checksum(v);
  if (v->footer.checksum != expected) {
    LOG_ERROR("❌ Checksum mismatch (got 0x%08X expected 0x%08X)\n", v->footer.checksum, expected);
    return false;
  }

  printf("✅ 4Splat file validated!\n");

  return true;
}

void free_splat4DVideo(Splat4DVideo *v) {
  if (!v)
    return;
  free(v->palette.palette);
  free(v->index.index);
  v->palette.palette = NULL;
  v->index.index = NULL;
}

// --- lossless image codec ---------------------------------------------------
//
// A 2D RGB image maps directly onto the format: each distinct color becomes a
// palette splat (its r/g/b, with mu/sigma set from the spatial spread of the
// pixels that use it), and every pixel stores the index of its color. Decoding
// writes each pixel back from its palette color, so 8-bit RGB round-trips
// exactly. Self-contained: no math library required.

static double splat_sqrt(double x) {
  if (x <= 0.0)
    return 0.0;
  double g = x > 1.0 ? x : 1.0;
  for (int i = 0; i < 60; ++i)
    g = 0.5 * (g + x / g);
  return g;
}

static size_t splat_next_pow2(size_t n) {
  size_t p = 1;
  while (p < n)
    p <<= 1;
  return p;
}

// Open-addressing map from a 24-bit 0xRRGGBB color to its palette index.
typedef struct {
  uint32_t *key; // stored as color+1; 0 marks an empty slot
  uint32_t *val;
  size_t cap; // power of two
} ColorMap;

static bool colormap_init(ColorMap *m, size_t expected) {
  size_t cap = splat_next_pow2(expected < 16 ? 16 : expected * 2);
  m->key = calloc(cap, sizeof(uint32_t));
  m->val = malloc(cap * sizeof(uint32_t));
  if (!m->key || !m->val) {
    free(m->key);
    free(m->val);
    return false;
  }
  m->cap = cap;
  return true;
}

static void colormap_free(ColorMap *m) {
  free(m->key);
  free(m->val);
  m->key = m->val = NULL;
}

// Look up `color`; if present set *out and return true, else return false.
static bool colormap_get(const ColorMap *m, uint32_t color, uint32_t *out) {
  size_t mask = m->cap - 1;
  size_t h = ((size_t)color * 2654435761u) & mask;
  while (m->key[h] != 0) {
    if (m->key[h] == color + 1) {
      *out = m->val[h];
      return true;
    }
    h = (h + 1) & mask;
  }
  return false;
}

static void colormap_put(ColorMap *m, uint32_t color, uint32_t value) {
  size_t mask = m->cap - 1;
  size_t h = ((size_t)color * 2654435761u) & mask;
  while (m->key[h] != 0)
    h = (h + 1) & mask;
  m->key[h] = color + 1;
  m->val[h] = value;
}

// --- median-cut color quantization -----------------------------------------
//
// Reduce `nu` distinct colors (with pixel-count weights) to at most max_colors
// representative colors. Fills quant_of[u] with the representative index for
// each input color and rep_colors[j] with each representative's packed RGB;
// returns the number of representatives, or 0 on allocation failure.

static const uint32_t *g_mc_colors; // sort context (single-threaded CLI/codec)
static int g_mc_shift;

static int mc_channel_cmp(const void *a, const void *b) {
  int va = (int)((g_mc_colors[*(const uint32_t *)a] >> g_mc_shift) & 0xFF);
  int vb = (int)((g_mc_colors[*(const uint32_t *)b] >> g_mc_shift) & 0xFF);
  return va - vb;
}

static uint32_t splat_median_cut(const uint32_t *colors, const double *counts, uint32_t nu,
                                 uint32_t max_colors, uint32_t *quant_of, uint32_t *rep_colors) {
  uint32_t *order = malloc((size_t)nu * sizeof(uint32_t));
  uint32_t *bstart = malloc((size_t)max_colors * sizeof(uint32_t));
  uint32_t *bend = malloc((size_t)max_colors * sizeof(uint32_t));
  if (!order || !bstart || !bend) {
    free(order);
    free(bstart);
    free(bend);
    return 0;
  }
  for (uint32_t i = 0; i < nu; ++i)
    order[i] = i;

  bstart[0] = 0;
  bend[0] = nu;
  uint32_t nboxes = 1;

  while (nboxes < max_colors) {
    // Pick the splittable box with the widest single-channel range.
    int best = -1;
    uint32_t best_range = 0, best_shift = 16;
    for (uint32_t b = 0; b < nboxes; ++b) {
      if (bend[b] - bstart[b] < 2)
        continue;
      int lo[3] = {255, 255, 255}, hi[3] = {0, 0, 0};
      for (uint32_t k = bstart[b]; k < bend[b]; ++k) {
        uint32_t c = colors[order[k]];
        int ch[3] = {(int)((c >> 16) & 0xFF), (int)((c >> 8) & 0xFF), (int)(c & 0xFF)};
        for (int d = 0; d < 3; ++d) {
          if (ch[d] < lo[d])
            lo[d] = ch[d];
          if (ch[d] > hi[d])
            hi[d] = ch[d];
        }
      }
      for (int d = 0; d < 3; ++d) {
        uint32_t range = (uint32_t)(hi[d] - lo[d]);
        if (best < 0 || range > best_range) {
          best_range = range;
          best = (int)b;
          best_shift = (uint32_t)(16 - 8 * d);
        }
      }
    }
    if (best < 0 || best_range == 0)
      break; // nothing left to split

    uint32_t s = bstart[best], e = bend[best];
    g_mc_colors = colors;
    g_mc_shift = (int)best_shift;
    qsort(order + s, e - s, sizeof(uint32_t), mc_channel_cmp);

    double total = 0.0;
    for (uint32_t k = s; k < e; ++k)
      total += counts[order[k]];
    double half = total / 2.0, acc = 0.0;
    uint32_t m = s + 1;
    for (uint32_t k = s; k < e; ++k) {
      acc += counts[order[k]];
      if (acc >= half) {
        m = k + 1;
        break;
      }
    }
    if (m <= s)
      m = s + 1;
    if (m >= e)
      m = e - 1;

    bend[best] = m;
    bstart[nboxes] = m;
    bend[nboxes] = e;
    nboxes++;
  }

  for (uint32_t b = 0; b < nboxes; ++b) {
    double sr = 0, sg = 0, sb = 0, sc = 0;
    for (uint32_t k = bstart[b]; k < bend[b]; ++k) {
      uint32_t c = colors[order[k]];
      double wgt = counts[order[k]];
      sr += wgt * ((c >> 16) & 0xFF);
      sg += wgt * ((c >> 8) & 0xFF);
      sb += wgt * (c & 0xFF);
      sc += wgt;
      quant_of[order[k]] = b;
    }
    uint32_t r = (uint32_t)(sr / sc + 0.5), gg = (uint32_t)(sg / sc + 0.5),
             bb = (uint32_t)(sb / sc + 0.5);
    rep_colors[b] = (r << 16) | (gg << 8) | bb;
  }

  free(order);
  free(bstart);
  free(bend);
  return nboxes;
}

// Build a video from `depth * frames` tightly packed w*h RGB8 slices that share
// one global palette (the format's core 4D model). Slices are supplied in
// t-major, z-minor order (slice index s = t*depth + z), matching the on-disk
// index order t -> z -> y -> x. Each palette color becomes a splat whose spatial
// mu/sigma come from the (x, y) spread of its pixels, and whose mu_z/sigma_t and
// mu_t/sigma_t come from the z (depth) and t (frame) positions it occupies. With
// max_colors == 0 the palette is exact (lossless); a positive max_colors
// quantizes to at most that many colors via median cut (lossy). On success *out
// owns freshly allocated palette/index.
bool stack_to_video_quantized(const uint8_t *const *slices, uint32_t depth, uint32_t frames,
                              uint32_t w, uint32_t h, uint32_t max_colors, Splat4DVideo *out) {
  if (!slices || !out || depth == 0 || frames == 0 || w == 0 || h == 0)
    return false;
  uint64_t nslices = (uint64_t)depth * (uint64_t)frames;
  for (uint64_t s = 0; s < nslices; ++s)
    if (!slices[s])
      return false;

  uint64_t npix = (uint64_t)w * (uint64_t)h;
  uint64_t total = 0;
  if (!checked_mul_u64(npix, nslices, &total) || total > SIZE_MAX / sizeof(uint64_t))
    return false;

  uint64_t *index = malloc((size_t)total * sizeof(uint64_t));
  if (!index)
    return false;

  size_t map_hint = total < (1u << 24) ? (size_t)total : (1u << 24);
  ColorMap map;
  if (!colormap_init(&map, map_hint)) {
    free(index);
    return false;
  }

  // Pass 1: distinct colors, their pixel counts, and each pixel's color index.
  uint32_t *colors = NULL;
  double *counts = NULL;
  size_t pal_cap = 0, pal_n = 0;
  bool ok = true;

  for (uint64_t s = 0; s < nslices && ok; ++s) {
    const uint8_t *rgb = slices[s];
    for (uint64_t i = 0; i < npix && ok; ++i) {
      uint32_t color =
          ((uint32_t)rgb[i * 3] << 16) | ((uint32_t)rgb[i * 3 + 1] << 8) | (uint32_t)rgb[i * 3 + 2];
      uint32_t idx;
      if (!colormap_get(&map, color, &idx)) {
        if (pal_n == UINT32_MAX) {
          ok = false;
          break;
        }
        if (pal_n == pal_cap) {
          size_t new_cap = pal_cap ? pal_cap * 2 : 256;
          uint32_t *gc = realloc(colors, new_cap * sizeof(uint32_t));
          double *gn = realloc(counts, new_cap * sizeof(double));
          if (!gc || !gn) {
            free(gc ? gc : colors);
            free(gn ? gn : counts);
            colors = NULL;
            counts = NULL;
            ok = false;
            break;
          }
          colors = gc;
          counts = gn;
          pal_cap = new_cap;
        }
        idx = (uint32_t)pal_n;
        colors[pal_n] = color;
        counts[pal_n] = 0.0;
        pal_n++;
        colormap_put(&map, color, idx);
      }
      counts[idx] += 1.0;
      index[s * npix + i] = idx;
    }
  }
  colormap_free(&map);

  if (!ok || pal_n == 0) {
    free(colors);
    free(counts);
    free(index);
    return false;
  }

  // Decide the final palette: exact, or median-cut down to max_colors.
  uint32_t *quant_of = NULL;
  uint32_t *rep = NULL;
  uint32_t final_n;
  if (max_colors == 0 || (uint64_t)max_colors >= pal_n) {
    final_n = (uint32_t)pal_n; // already <= max_colors, keep exact colors
    rep = colors;              // representatives are the colors themselves
  } else {
    quant_of = malloc(pal_n * sizeof(uint32_t));
    rep = malloc((size_t)max_colors * sizeof(uint32_t));
    if (!quant_of || !rep) {
      free(quant_of);
      free(rep);
      free(colors);
      free(counts);
      free(index);
      return false;
    }
    final_n = splat_median_cut(colors, counts, (uint32_t)pal_n, max_colors, quant_of, rep);
    if (final_n == 0) {
      free(quant_of);
      free(rep);
      free(colors);
      free(counts);
      free(index);
      return false;
    }
    // Remap each pixel from its exact color index to the representative index.
    for (uint64_t k = 0; k < total; ++k)
      index[k] = quant_of[index[k]];
  }
  free(counts);

  // Accumulate spatial/depth/temporal statistics per final palette entry.
  double *cnt = calloc(final_n, sizeof(double));
  double *sx = calloc(final_n, sizeof(double));
  double *sy = calloc(final_n, sizeof(double));
  double *sxx = calloc(final_n, sizeof(double));
  double *syy = calloc(final_n, sizeof(double));
  double *sz = calloc(final_n, sizeof(double));
  double *szz = calloc(final_n, sizeof(double));
  double *st = calloc(final_n, sizeof(double));
  double *stt = calloc(final_n, sizeof(double));
  Splat4D *palette = malloc((size_t)final_n * sizeof(Splat4D));
  if (!cnt || !sx || !sy || !sxx || !syy || !sz || !szz || !st || !stt || !palette) {
    free(cnt);
    free(sx);
    free(sy);
    free(sxx);
    free(syy);
    free(sz);
    free(szz);
    free(st);
    free(stt);
    free(palette);
    if (quant_of) {
      free(rep);
      free(quant_of);
    }
    free(colors);
    free(index);
    return false;
  }

  for (uint64_t s = 0; s < nslices; ++s) {
    double zz = (double)(s % depth), tt = (double)(s / depth);
    for (uint64_t i = 0; i < npix; ++i) {
      uint64_t j = index[s * npix + i];
      double x = (double)(i % w), y = (double)(i / w);
      cnt[j] += 1.0;
      sx[j] += x;
      sy[j] += y;
      sxx[j] += x * x;
      syy[j] += y * y;
      sz[j] += zz;
      szz[j] += zz * zz;
      st[j] += tt;
      stt[j] += tt * tt;
    }
  }

  for (uint32_t j = 0; j < final_n; ++j) {
    double n = cnt[j] > 0 ? cnt[j] : 1.0;
    double mx = sx[j] / n, my = sy[j] / n, mz = sz[j] / n, mt = st[j] / n;
    double vx = sxx[j] / n - mx * mx;
    double vy = syy[j] / n - my * my;
    double vz = szz[j] / n - mz * mz;
    double vt = stt[j] / n - mt * mt;
    uint32_t c = rep[j];
    palette[j] = create_splat4D(
        (float)mx, (float)splat_sqrt(vx), (float)my, (float)splat_sqrt(vy), (float)mz,
        (float)splat_sqrt(vz), (float)mt, (float)splat_sqrt(vt), (float)((c >> 16) & 0xFF) / 255.0f,
        (float)((c >> 8) & 0xFF) / 255.0f, (float)(c & 0xFF) / 255.0f, 1.0f);
  }

  free(cnt);
  free(sx);
  free(sy);
  free(sxx);
  free(syy);
  free(sz);
  free(szz);
  free(st);
  free(stt);
  if (quant_of) {
    free(rep);
    free(quant_of);
  }
  free(colors);

  uint32_t iw = (final_n <= 256)     ? SPLAT_INDEX_WIDTH_8
                : (final_n <= 65536) ? SPLAT_INDEX_WIDTH_16
                                     : SPLAT_INDEX_WIDTH_32;
  uint32_t flags = SPLAT_FLAG_PRECISION_FLOAT32 | (iw << SPLAT_FLAG_INDEX_WIDTH_SHIFT) |
                   (SPLAT_SHAPE_AXIS_ALIGNED << SPLAT_FLAG_SPLAT_SHAPE_SHIFT);
  Splat4DHeader header = create_splat4DHeader(w, h, depth, frames, final_n, flags);
  *out = create_splat4DVideo(header, palette, index);
  return true;
}

// A stack of frames (depth == 1) is the video case of the general codec.
bool frames_to_video_quantized(const uint8_t *const *frames, uint32_t nframes, uint32_t w,
                               uint32_t h, uint32_t max_colors, Splat4DVideo *out) {
  return stack_to_video_quantized(frames, 1, nframes, w, h, max_colors, out);
}

// Lossless: one palette entry per distinct color across all frames.
bool frames_to_video(const uint8_t *const *frames, uint32_t nframes, uint32_t w, uint32_t h,
                     Splat4DVideo *out) {
  return frames_to_video_quantized(frames, nframes, w, h, 0, out);
}

// A single image is the pseudo-GIF collapse of the video codec: one frame.
bool image_to_video(const uint8_t *rgb, uint32_t w, uint32_t h, Splat4DVideo *out) {
  return frames_to_video(&rgb, 1, w, h, out);
}

static uint8_t splat_channel_to_u8(float v) {
  float s = v * 255.0f + 0.5f;
  if (s < 0.0f)
    s = 0.0f;
  if (s > 255.0f)
    s = 255.0f;
  return (uint8_t)s;
}

// Reconstruct a tightly packed w*h RGB8 buffer from a 2D video. *rgb_out owns a
// freshly allocated buffer on success (caller frees).
bool video_to_image(const Splat4DVideo *v, uint8_t **rgb_out, uint32_t *w_out, uint32_t *h_out) {
  if (!v || !rgb_out || !v->palette.palette || !v->index.index)
    return false;
  if (v->header.depth != 1 || v->header.frames != 1) {
    LOG_ERROR("❌ Image decode requires a 2D video (depth=frames=1)\n");
    return false;
  }
  uint32_t w = v->header.width, h = v->header.height;
  uint64_t npix = (uint64_t)w * (uint64_t)h;
  if (npix == 0 || npix > SIZE_MAX / 3)
    return false;

  uint8_t *rgb = malloc((size_t)npix * 3);
  if (!rgb)
    return false;

  for (uint64_t i = 0; i < npix; ++i) {
    uint64_t idx = v->index.index[i];
    if (idx >= v->header.pSize) {
      free(rgb);
      return false;
    }
    const Splat4D *s = &v->palette.palette[idx];
    rgb[i * 3] = splat_channel_to_u8(s->r);
    rgb[i * 3 + 1] = splat_channel_to_u8(s->g);
    rgb[i * 3 + 2] = splat_channel_to_u8(s->b);
  }

  *rgb_out = rgb;
  if (w_out)
    *w_out = w;
  if (h_out)
    *h_out = h;
  return true;
}

// Reconstruct every slice of a video/volume. On success *slices_out is an array
// of depth*frames freshly allocated w*h*3 RGB8 buffers in t-major, z-minor order;
// the caller frees each buffer and then the array.
bool video_to_slices(const Splat4DVideo *v, uint8_t ***slices_out, uint32_t *nslices_out,
                     uint32_t *w_out, uint32_t *h_out) {
  if (!v || !slices_out || !v->palette.palette || !v->index.index)
    return false;
  uint32_t w = v->header.width, h = v->header.height;
  uint64_t nslices = (uint64_t)v->header.depth * (uint64_t)v->header.frames;
  uint64_t npix = (uint64_t)w * (uint64_t)h;
  if (npix == 0 || npix > SIZE_MAX / 3 || nslices == 0 || nslices > SIZE_MAX / sizeof(uint8_t *))
    return false;

  uint8_t **slices = calloc((size_t)nslices, sizeof(uint8_t *));
  if (!slices)
    return false;

  bool ok = true;
  for (uint64_t s = 0; s < nslices && ok; ++s) {
    uint8_t *rgb = malloc((size_t)npix * 3);
    if (!rgb) {
      ok = false;
      break;
    }
    slices[s] = rgb;
    for (uint64_t i = 0; i < npix; ++i) {
      uint64_t idx = v->index.index[s * npix + i];
      if (idx >= v->header.pSize) {
        ok = false;
        break;
      }
      const Splat4D *sp = &v->palette.palette[idx];
      rgb[i * 3] = splat_channel_to_u8(sp->r);
      rgb[i * 3 + 1] = splat_channel_to_u8(sp->g);
      rgb[i * 3 + 2] = splat_channel_to_u8(sp->b);
    }
  }

  if (!ok) {
    for (uint64_t s = 0; s < nslices; ++s)
      free(slices[s]);
    free(slices);
    return false;
  }

  *slices_out = slices;
  if (nslices_out)
    *nslices_out = (uint32_t)nslices;
  if (w_out)
    *w_out = w;
  if (h_out)
    *h_out = h;
  return true;
}

// Reconstruct every frame of a video (depth must be 1).
bool video_to_frames(const Splat4DVideo *v, uint8_t ***frames_out, uint32_t *nframes_out,
                     uint32_t *w_out, uint32_t *h_out) {
  if (v && v->header.depth != 1) {
    LOG_ERROR("❌ Video decode requires depth = 1\n");
    return false;
  }
  return video_to_slices(v, frames_out, nframes_out, w_out, h_out);
}

#ifndef UNIT_TEST
typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t depth;
  uint32_t frames;
  uint32_t palette_size;
  uint32_t flags;
  bool width_set;
  bool height_set;
  bool depth_set;
  bool frames_set;
  bool palette_size_set;
  bool flags_set;
} MetadataOptions;

typedef struct {
  const char *palette_path;
  const char *index_path;
  const char *output_path;
  MetadataOptions meta;
  bool precision_set;       // an explicit --precision was given
  uint32_t precision_value; // 0=float16, 1=float32, 2=float64
} EncodeOptions;

static void print_usage(FILE *stream) {
  fprintf(stream,
          "Usage:\n"
          "  4splat encode --palette <palette.bin> --index <index.bin> --output <file.4spl> "
          "--width <w> --height <h> --depth <d> --frames <f> [--palette-size <n>] [--flags <n>]\n"
          "      [--precision float16|float32|float64] [--compression <scheme>] "
          "[--index-width 1|2|4|8]\n"
          "      [--splat-shape <shape>] [--color-space <space>] [--interpolation <mode>] "
          "[--sorted] [--metadata <0-255>]\n"
          "  4splat decode --input <file.4spl> [--palette <palette.bin>] [--index <index.bin>] "
          "[--output <file.4spl>] [--to-color <space>] [--print] [--validate]\n"
          "  4splat encode-image [--compress <scheme>] [--colors <N>] <in.ppm> <out.4spl>\n"
          "  4splat decode-image <in.4spl> <out.ppm>\n"
          "  4splat encode-video [--compress <scheme>] [--colors <N>] <out.4spl> <frame.ppm>...\n"
          "  4splat decode-video <in.4spl> <out-prefix>   (writes <prefix>NNNN.ppm)\n"
          "  4splat encode-volume [--compress <scheme>] [--colors <N>] <out.4spl> <slice.ppm>...\n"
          "  4splat decode-volume <in.4spl> <out-prefix>   (writes <prefix>NNNN.ppm)\n");
}

// Parse a color-space name (as used on the command line) into its flag value.
static bool parse_color_space_name(const char *name, uint32_t *out) {
  static const struct {
    const char *name;
    uint32_t value;
  } table[] = {
      {"srgb", SPLAT_COLOR_SRGB},
      {"linear-srgb", SPLAT_COLOR_LINEAR_SRGB},
      {"oklab", SPLAT_COLOR_OKLAB},
      {"display-p3", SPLAT_COLOR_DISPLAY_P3},
      {"rec709", SPLAT_COLOR_REC709},
      {"rec2020", SPLAT_COLOR_REC2020},
      {"dci-p3", SPLAT_COLOR_DCI_P3},
      {"aces-ap0", SPLAT_COLOR_ACES_AP0},
      {"prophoto", SPLAT_COLOR_PROPHOTO_RGB},
      {"rec2100", SPLAT_COLOR_REC2100},
      {"lab", SPLAT_COLOR_CIE_LAB},
      {"xyz-d65", SPLAT_COLOR_CIE_XYZ_D65},
      {"acescg-ap1", SPLAT_COLOR_ACESCG_AP1},
      {"rec601", SPLAT_COLOR_REC601},
      {"xyz-d50", SPLAT_COLOR_CIE_XYZ_D50},
  };
  for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); ++i) {
    if (strcmp(name, table[i].name) == 0) {
      *out = table[i].value;
      return true;
    }
  }
  return false;
}

static bool lookup_named_value(const char *name, uint32_t *out, const char *const *names,
                               size_t count) {
  for (size_t i = 0; i < count; ++i) {
    if (names[i] && strcmp(name, names[i]) == 0) {
      *out = (uint32_t)i;
      return true;
    }
  }
  return false;
}

static bool parse_precision_name(const char *name, uint32_t *out) {
  if (!strcmp(name, "float16") || !strcmp(name, "f16") || !strcmp(name, "16"))
    *out = 0;
  else if (!strcmp(name, "float32") || !strcmp(name, "f32") || !strcmp(name, "32"))
    *out = 1;
  else if (!strcmp(name, "float64") || !strcmp(name, "f64") || !strcmp(name, "64"))
    *out = 2;
  else
    return false;
  return true;
}

static bool parse_index_width_name(const char *name, uint32_t *out) {
  if (!strcmp(name, "1"))
    *out = SPLAT_INDEX_WIDTH_8;
  else if (!strcmp(name, "2"))
    *out = SPLAT_INDEX_WIDTH_16;
  else if (!strcmp(name, "4"))
    *out = SPLAT_INDEX_WIDTH_32;
  else if (!strcmp(name, "8"))
    *out = SPLAT_INDEX_WIDTH_64;
  else
    return false;
  return true;
}

static bool parse_splat_shape_name(const char *name, uint32_t *out) {
  static const char *const names[] = {"isotropic", "axis-aligned", "full-covariance"};
  return lookup_named_value(name, out, names, sizeof(names) / sizeof(names[0]));
}

static bool parse_compression_name(const char *name, uint32_t *out) {
  static const char *const names[] = {"none",  "rle",    "deflate", "rar", "lzo", "zlib",
                                      "bzip2", "lzma",   "zpaq",    "xz",  "lz4", "snappy",
                                      "lzham", "brotli", "lzfse",   "zstd"};
  return lookup_named_value(name, out, names, sizeof(names) / sizeof(names[0]));
}

static bool parse_interpolation_name(const char *name, uint32_t *out) {
  static const char *const names[] = {"none",
                                      "nearest",
                                      "axis-aligned",
                                      "smooth",
                                      "lanczos",
                                      "gaussian",
                                      "catmull-rom",
                                      "nurbs",
                                      "rbf",
                                      "optical-flow",
                                      "neural",
                                      "akima",
                                      "inverse-distance",
                                      "fourier",
                                      "moving-least-squares",
                                      "cubic-hermite"};
  return lookup_named_value(name, out, names, sizeof(names) / sizeof(names[0]));
}

static void set_flag_field(uint32_t *flags, uint32_t mask, uint32_t shift, uint32_t value) {
  *flags = (*flags & ~mask) | ((value << shift) & mask);
}

static bool parse_u32(const char *arg, uint32_t *out) {
  if (!arg || !out)
    return false;
  errno = 0;
  char *end = NULL;
  unsigned long value = strtoul(arg, &end, 10);
  if (errno != 0 || !end || *end != '\0' || value > UINT32_MAX)
    return false;
  *out = (uint32_t)value;
  return true;
}

static bool load_file_into_buffer(const char *path, size_t element_size, void **buffer,
                                  uint64_t *count_out) {
  if (!path || !buffer || !count_out)
    return false;

  FILE *fp = fopen(path, "rb");
  if (!fp) {
    LOG_ERROR("❌ Unable to open '%s': %s\n", path, strerror(errno));
    return false;
  }

  if (fseek(fp, 0, SEEK_END) != 0) {
    LOG_ERROR("❌ Failed to seek '%s'\n", path);
    fclose(fp);
    return false;
  }

  long size = ftell(fp);
  if (size < 0) {
    LOG_ERROR("❌ Failed to determine size of '%s'\n", path);
    fclose(fp);
    return false;
  }

  if (size % (long)element_size != 0) {
    LOG_ERROR("❌ File '%s' is not aligned to element size %zu\n", path, element_size);
    fclose(fp);
    return false;
  }

  uint64_t count = (uint64_t)size / element_size;
  if (count == 0) {
    LOG_ERROR("❌ File '%s' does not contain any entries\n", path);
    fclose(fp);
    return false;
  }

  if (fseek(fp, 0, SEEK_SET) != 0) {
    LOG_ERROR("❌ Failed to rewind '%s'\n", path);
    fclose(fp);
    return false;
  }

  if (element_size == 0 || count > SIZE_MAX / element_size) {
    LOG_ERROR("❌ File '%s' is too large to load into memory\n", path);
    fclose(fp);
    return false;
  }

  void *data = malloc(count * element_size);
  if (!data) {
    LOG_ERROR("❌ Out of memory while reading '%s'\n", path);
    fclose(fp);
    return false;
  }

  if (fread(data, element_size, count, fp) != count) {
    LOG_ERROR("❌ Failed to read '%s'\n", path);
    free(data);
    fclose(fp);
    return false;
  }

  fclose(fp);
  *buffer = data;
  *count_out = count;
  return true;
}

static bool load_palette_from_file(const char *path, Splat4D **palette_out, uint32_t *count_out) {
  uint64_t count = 0;
  void *buffer = NULL;
  if (!load_file_into_buffer(path, sizeof(Splat4D), &buffer, &count))
    return false;
  if (count > UINT32_MAX) {
    LOG_ERROR("❌ Palette '%s' has too many entries (%" PRIu64 ")\n", path, count);
    free(buffer);
    return false;
  }
  *palette_out = buffer;
  *count_out = (uint32_t)count;
  return true;
}

static bool load_index_from_file(const char *path, uint64_t **index_out, uint64_t *count_out) {
  return load_file_into_buffer(path, sizeof(uint64_t), (void **)index_out, count_out);
}

static bool save_buffer_to_file(const char *path, const void *buffer, size_t element_size,
                                uint64_t count) {
  if (!path || !buffer || element_size == 0 || count == 0)
    return false;
  FILE *fp = fopen(path, "wb");
  if (!fp) {
    LOG_ERROR("❌ Unable to write '%s': %s\n", path, strerror(errno));
    return false;
  }
  size_t written = fwrite(buffer, element_size, count, fp);
  fclose(fp);
  if (written != count) {
    LOG_ERROR("❌ Short write while writing '%s'\n", path);
    return false;
  }
  return true;
}

static bool save_palette_to_file(const char *path, const Splat4DVideo *video) {
  return save_buffer_to_file(path, video->palette.palette, sizeof(Splat4D), video->header.pSize);
}

static bool save_index_to_file(const char *path, const Splat4DVideo *video) {
  return save_buffer_to_file(path, video->index.index, sizeof(uint64_t),
                             header_total_indices(&video->header));
}

static bool parse_metadata_option(const char *name, const char *value, MetadataOptions *meta) {
  if (strcmp(name, "--width") == 0) {
    if (!parse_u32(value, &meta->width))
      return false;
    meta->width_set = true;
  } else if (strcmp(name, "--height") == 0) {
    if (!parse_u32(value, &meta->height))
      return false;
    meta->height_set = true;
  } else if (strcmp(name, "--depth") == 0) {
    if (!parse_u32(value, &meta->depth))
      return false;
    meta->depth_set = true;
  } else if (strcmp(name, "--frames") == 0) {
    if (!parse_u32(value, &meta->frames))
      return false;
    meta->frames_set = true;
  } else if (strcmp(name, "--palette-size") == 0) {
    if (!parse_u32(value, &meta->palette_size))
      return false;
    meta->palette_size_set = true;
  } else if (strcmp(name, "--flags") == 0) {
    if (!parse_u32(value, &meta->flags))
      return false;
    meta->flags_set = true;
  } else {
    return false;
  }
  return true;
}

static int execute_encode(EncodeOptions *opts) {
  Splat4D *palette = NULL;
  uint32_t palette_count = 0;
  if (!load_palette_from_file(opts->palette_path, &palette, &palette_count))
    return EXIT_FAILURE;

  if (!opts->meta.palette_size_set)
    opts->meta.palette_size = palette_count;

  if (opts->meta.palette_size != palette_count) {
    LOG_ERROR("❌ Palette size mismatch: option=%u file=%u\n", opts->meta.palette_size,
              palette_count);
    free(palette);
    return EXIT_FAILURE;
  }

  uint64_t *indices = NULL;
  uint64_t index_count = 0;
  if (!load_index_from_file(opts->index_path, &indices, &index_count)) {
    free(palette);
    return EXIT_FAILURE;
  }

  uint64_t expected_indices = (uint64_t)opts->meta.width * (uint64_t)opts->meta.height *
                              (uint64_t)opts->meta.depth * (uint64_t)opts->meta.frames;
  if (expected_indices != index_count) {
    LOG_ERROR("❌ Index count mismatch: expected %" PRIu64
              " (from dimensions) but file has %" PRIu64 " entries\n",
              expected_indices, index_count);
    free(palette);
    free(indices);
    return EXIT_FAILURE;
  }

  uint32_t effective_flags = opts->meta.flags_set ? opts->meta.flags : 0u;
  uint8_t idx_width = get_index_width_bytes(effective_flags);
  uint64_t bad_pos = 0;
  SplatIndexCheck idx_check =
      check_index_values(indices, index_count, opts->meta.palette_size, idx_width, &bad_pos);
  if (idx_check == SPLAT_INDEX_OUT_OF_RANGE) {
    LOG_ERROR("❌ Index %" PRIu64 " at position %" PRIu64 " is out of range for palette size %u\n",
              indices[bad_pos], bad_pos, opts->meta.palette_size);
    free(palette);
    free(indices);
    return EXIT_FAILURE;
  }
  if (idx_check == SPLAT_INDEX_TOO_WIDE) {
    LOG_ERROR("❌ Index %" PRIu64 " at position %" PRIu64 " does not fit the %u-byte index width; "
              "select a wider index width via --index-width\n",
              indices[bad_pos], bad_pos, idx_width);
    free(palette);
    free(indices);
    return EXIT_FAILURE;
  }

  Splat4DHeader header =
      create_splat4DHeader(opts->meta.width, opts->meta.height, opts->meta.depth, opts->meta.frames,
                           opts->meta.palette_size, effective_flags);
  // create_splat4DHeader normalizes an unset precision (float16 bits) up to
  // float32; honor an explicit --precision float16 by restoring it here.
  if (opts->precision_set)
    set_flag_field(&header.flags, SPLAT_FLAG_PRECISION_MASK, SPLAT_FLAG_PRECISION_SHIFT,
                   opts->precision_value);

  // Refuse to emit a file this build could not read back (unsupported precision,
  // a compression codec that is not linked in, reserved/encrypted fields).
  if (!flags_supported(header.flags)) {
    free(palette);
    free(indices);
    return EXIT_FAILURE;
  }

  Splat4DVideo video = create_splat4DVideo(header, palette, indices);

  FILE *fp = fopen(opts->output_path, "wb");
  if (!fp) {
    LOG_ERROR("❌ Unable to create '%s': %s\n", opts->output_path, strerror(errno));
    free_splat4DVideo(&video);
    return EXIT_FAILURE;
  }

  bool wrote = write_splat4DVideo(fp, &video);
  fclose(fp);
  free_splat4DVideo(&video);

  if (!wrote) {
    LOG_ERROR("❌ Failed to write 4Splat file '%s'\n", opts->output_path);
    return EXIT_FAILURE;
  }

  printf("✅ Wrote 4Splat file to '%s'\n", opts->output_path);
  return EXIT_SUCCESS;
}

static int command_encode(int argc, char **argv) {
  EncodeOptions opts = {0};

  for (int i = 0; i < argc; i++) {
    const char *arg = argv[i];
    if (strcmp(arg, "--palette") == 0 && i + 1 < argc) {
      opts.palette_path = argv[++i];
    } else if (strcmp(arg, "--index") == 0 && i + 1 < argc) {
      opts.index_path = argv[++i];
    } else if (strcmp(arg, "--output") == 0 && i + 1 < argc) {
      opts.output_path = argv[++i];
    } else if ((strcmp(arg, "--width") == 0 || strcmp(arg, "--height") == 0 ||
                strcmp(arg, "--depth") == 0 || strcmp(arg, "--frames") == 0 ||
                strcmp(arg, "--palette-size") == 0 || strcmp(arg, "--flags") == 0) &&
               i + 1 < argc) {
      if (!parse_metadata_option(arg, argv[++i], &opts.meta)) {
        fprintf(stderr, "❌ Invalid value for %s\n", arg);
        return EXIT_FAILURE;
      }
    } else if (strcmp(arg, "--precision") == 0 && i + 1 < argc) {
      uint32_t v;
      if (!parse_precision_name(argv[++i], &v)) {
        fprintf(stderr, "❌ Unknown precision '%s' (float16|float32|float64)\n", argv[i]);
        return EXIT_FAILURE;
      }
      opts.precision_set = true;
      opts.precision_value = v;
      set_flag_field(&opts.meta.flags, SPLAT_FLAG_PRECISION_MASK, SPLAT_FLAG_PRECISION_SHIFT, v);
      opts.meta.flags_set = true;
    } else if (strcmp(arg, "--compression") == 0 && i + 1 < argc) {
      uint32_t v;
      if (!parse_compression_name(argv[++i], &v)) {
        fprintf(stderr, "❌ Unknown compression scheme '%s'\n", argv[i]);
        return EXIT_FAILURE;
      }
      set_flag_field(&opts.meta.flags, SPLAT_FLAG_COMPRESSION_MASK, SPLAT_FLAG_COMPRESSION_SHIFT,
                     v);
      opts.meta.flags_set = true;
    } else if (strcmp(arg, "--index-width") == 0 && i + 1 < argc) {
      uint32_t v;
      if (!parse_index_width_name(argv[++i], &v)) {
        fprintf(stderr, "❌ Invalid index width '%s' (1|2|4|8)\n", argv[i]);
        return EXIT_FAILURE;
      }
      set_flag_field(&opts.meta.flags, SPLAT_FLAG_INDEX_WIDTH_MASK, SPLAT_FLAG_INDEX_WIDTH_SHIFT,
                     v);
      opts.meta.flags_set = true;
    } else if (strcmp(arg, "--splat-shape") == 0 && i + 1 < argc) {
      uint32_t v;
      if (!parse_splat_shape_name(argv[++i], &v)) {
        fprintf(stderr, "❌ Unknown splat shape '%s' (isotropic|axis-aligned|full-covariance)\n",
                argv[i]);
        return EXIT_FAILURE;
      }
      set_flag_field(&opts.meta.flags, SPLAT_FLAG_SPLAT_SHAPE_MASK, SPLAT_FLAG_SPLAT_SHAPE_SHIFT,
                     v);
      opts.meta.flags_set = true;
    } else if (strcmp(arg, "--color-space") == 0 && i + 1 < argc) {
      uint32_t v;
      if (!parse_color_space_name(argv[++i], &v)) {
        fprintf(stderr, "❌ Unknown color space '%s'\n", argv[i]);
        return EXIT_FAILURE;
      }
      set_flag_field(&opts.meta.flags, SPLAT_FLAG_COLOR_SPACE_MASK, SPLAT_FLAG_COLOR_SPACE_SHIFT,
                     v);
      opts.meta.flags_set = true;
    } else if (strcmp(arg, "--interpolation") == 0 && i + 1 < argc) {
      uint32_t v;
      if (!parse_interpolation_name(argv[++i], &v)) {
        fprintf(stderr, "❌ Unknown interpolation '%s'\n", argv[i]);
        return EXIT_FAILURE;
      }
      set_flag_field(&opts.meta.flags, SPLAT_FLAG_INTERP_MASK, SPLAT_FLAG_INTERP_SHIFT, v);
      opts.meta.flags_set = true;
    } else if (strcmp(arg, "--metadata") == 0 && i + 1 < argc) {
      uint32_t v;
      if (!parse_u32(argv[++i], &v) || v > 0xFF) {
        fprintf(stderr, "❌ Invalid metadata byte '%s' (0-255)\n", argv[i]);
        return EXIT_FAILURE;
      }
      set_flag_field(&opts.meta.flags, SPLAT_FLAG_METADATA_MASK, SPLAT_FLAG_METADATA_SHIFT, v);
      opts.meta.flags_set = true;
    } else if (strcmp(arg, "--sorted") == 0) {
      opts.meta.flags |= SPLAT_FLAG_SORTED;
      opts.meta.flags_set = true;
    } else {
      fprintf(stderr, "❌ Unknown or incomplete option '%s'\n", arg);
      return EXIT_FAILURE;
    }
  }

  if (!opts.palette_path || !opts.index_path || !opts.output_path || !opts.meta.width_set ||
      !opts.meta.height_set || !opts.meta.depth_set || !opts.meta.frames_set) {
    fprintf(stderr, "❌ encode requires --palette, --index, --output, --width, --height, --depth, "
                    "and --frames\n");
    return EXIT_FAILURE;
  }

  return execute_encode(&opts);
}

static int command_decode(int argc, char **argv) {
  const char *input_path = NULL;
  const char *palette_out = NULL;
  const char *index_out = NULL;
  const char *output_path = NULL;
  const char *to_color = NULL;
  bool print_summary = false;
  bool do_validate = false;

  for (int i = 0; i < argc; i++) {
    const char *arg = argv[i];
    if (strcmp(arg, "--input") == 0 && i + 1 < argc) {
      input_path = argv[++i];
    } else if (strcmp(arg, "--palette") == 0 && i + 1 < argc) {
      palette_out = argv[++i];
    } else if (strcmp(arg, "--index") == 0 && i + 1 < argc) {
      index_out = argv[++i];
    } else if (strcmp(arg, "--output") == 0 && i + 1 < argc) {
      output_path = argv[++i];
    } else if (strcmp(arg, "--to-color") == 0 && i + 1 < argc) {
      to_color = argv[++i];
    } else if (strcmp(arg, "--print") == 0) {
      print_summary = true;
    } else if (strcmp(arg, "--validate") == 0) {
      do_validate = true;
    } else {
      LOG_ERROR("❌ Unknown or incomplete option '%s'\n", arg);
      return EXIT_FAILURE;
    }
  }

  if (!input_path) {
    LOG_ERROR("❌ decode requires --input <file.4spl>\n");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(input_path, "rb");
  if (!fp) {
    LOG_ERROR("❌ Unable to open '%s': %s\n", input_path, strerror(errno));
    return EXIT_FAILURE;
  }

  Splat4DVideo video;
  bool read_ok = read_splat4DVideo(fp, &video);
  fclose(fp);

  if (!read_ok) {
    LOG_ERROR("❌ Failed to read '%s'\n", input_path);
    return EXIT_FAILURE;
  }

  if (do_validate && !validate_splat4DVideo(&video)) {
    free_splat4DVideo(&video);
    return EXIT_FAILURE;
  }

  if (to_color) {
    uint32_t target = 0;
    if (!parse_color_space_name(to_color, &target)) {
      LOG_ERROR("❌ Unknown color space '%s'\n", to_color);
      free_splat4DVideo(&video);
      return EXIT_FAILURE;
    }
#ifdef SPLAT_WITH_LCMS2
    uint32_t source =
        (video.header.flags & SPLAT_FLAG_COLOR_SPACE_MASK) >> SPLAT_FLAG_COLOR_SPACE_SHIFT;
    if (!splat_convert_palette_colors(video.palette.palette, video.header.pSize, source, target)) {
      free_splat4DVideo(&video);
      return EXIT_FAILURE;
    }
    video.header.flags = (video.header.flags & ~SPLAT_FLAG_COLOR_SPACE_MASK) |
                         (target << SPLAT_FLAG_COLOR_SPACE_SHIFT);
    video.footer.checksum = compute_video_checksum(&video);
    printf("✅ Converted palette to %s\n", splat_color_space_name((SplatColorSpace)target));
#else
    LOG_ERROR("❌ Color-space conversion requires a build with lcms2 (SPLAT_WITH_LCMS2)\n");
    free_splat4DVideo(&video);
    return EXIT_FAILURE;
#endif
  }

  if (print_summary)
    print_splat4DVideo(&video);

  if (output_path) {
    FILE *out = fopen(output_path, "wb");
    if (!out) {
      LOG_ERROR("❌ Unable to create '%s': %s\n", output_path, strerror(errno));
      free_splat4DVideo(&video);
      return EXIT_FAILURE;
    }
    bool wrote = write_splat4DVideo(out, &video);
    fclose(out);
    if (!wrote) {
      LOG_ERROR("❌ Failed to write '%s'\n", output_path);
      free_splat4DVideo(&video);
      return EXIT_FAILURE;
    }
    printf("✅ Wrote 4Splat file to '%s'\n", output_path);
  }

  if (palette_out && !save_palette_to_file(palette_out, &video)) {
    free_splat4DVideo(&video);
    return EXIT_FAILURE;
  }

  if (index_out && !save_index_to_file(index_out, &video)) {
    free_splat4DVideo(&video);
    return EXIT_FAILURE;
  }

  free_splat4DVideo(&video);
  return EXIT_SUCCESS;
}

// Read a binary PPM (P6). On success returns an RGB8 buffer (caller frees) and
// its dimensions. Supports comments and 8-bit maxval only.
static uint8_t *read_ppm(const char *path, uint32_t *w_out, uint32_t *h_out) {
  FILE *fp = fopen(path, "rb");
  if (!fp) {
    LOG_ERROR("❌ Unable to open '%s': %s\n", path, strerror(errno));
    return NULL;
  }
  char magic[3] = {0};
  if (fscanf(fp, "%2s", magic) != 1 || strcmp(magic, "P6") != 0) {
    LOG_ERROR("❌ '%s' is not a binary PPM (P6)\n", path);
    fclose(fp);
    return NULL;
  }

  // Read width, height, maxval, skipping whitespace and #-comments.
  long vals[3];
  int got = 0;
  while (got < 3) {
    int c = fgetc(fp);
    if (c == EOF) {
      LOG_ERROR("❌ Truncated PPM header in '%s'\n", path);
      fclose(fp);
      return NULL;
    }
    if (c == '#') {
      while ((c = fgetc(fp)) != EOF && c != '\n')
        ;
      continue;
    }
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
      continue;
    ungetc(c, fp);
    if (fscanf(fp, "%ld", &vals[got]) != 1) {
      LOG_ERROR("❌ Malformed PPM header in '%s'\n", path);
      fclose(fp);
      return NULL;
    }
    got++;
  }
  if (vals[0] <= 0 || vals[1] <= 0 || vals[2] != 255) {
    LOG_ERROR("❌ Unsupported PPM (need positive dimensions and maxval 255)\n");
    fclose(fp);
    return NULL;
  }
  fgetc(fp); // single whitespace byte after maxval

  uint32_t w = (uint32_t)vals[0], h = (uint32_t)vals[1];
  size_t n = (size_t)w * (size_t)h * 3;
  uint8_t *rgb = malloc(n);
  if (!rgb) {
    fclose(fp);
    return NULL;
  }
  if (fread(rgb, 1, n, fp) != n) {
    LOG_ERROR("❌ Truncated pixel data in '%s'\n", path);
    free(rgb);
    fclose(fp);
    return NULL;
  }
  fclose(fp);
  *w_out = w;
  *h_out = h;
  return rgb;
}

static bool write_ppm(const char *path, const uint8_t *rgb, uint32_t w, uint32_t h) {
  FILE *fp = fopen(path, "wb");
  if (!fp) {
    LOG_ERROR("❌ Unable to create '%s': %s\n", path, strerror(errno));
    return false;
  }
  fprintf(fp, "P6\n%u %u\n255\n", w, h);
  size_t n = (size_t)w * (size_t)h * 3;
  bool ok = fwrite(rgb, 1, n, fp) == n;
  fclose(fp);
  return ok;
}

// Parse leading --compress <scheme> / --colors <N> options for the image and
// video encoders. Sets *codec and *max_colors and returns the index of the
// first positional argument, or -1 on error.
static int parse_encode_options(int argc, char **argv, uint32_t *codec, uint32_t *max_colors) {
  *codec = SPLAT_COMPRESSION_NONE;
  *max_colors = 0;
  int i = 0;
  while (i < argc && argv[i][0] == '-' && argv[i][1] == '-') {
    if (strcmp(argv[i], "--compress") == 0 && i + 1 < argc) {
      if (!parse_compression_name(argv[i + 1], codec)) {
        LOG_ERROR("❌ Unknown compression scheme '%s'\n", argv[i + 1]);
        return -1;
      }
      if (!splat_compression_available(*codec)) {
        LOG_ERROR("❌ Compression scheme not available in this build: %s\n",
                  splat_compression_display_name(*codec));
        return -1;
      }
      i += 2;
    } else if (strcmp(argv[i], "--colors") == 0 && i + 1 < argc) {
      if (!parse_u32(argv[i + 1], max_colors) || *max_colors == 0) {
        LOG_ERROR("❌ Invalid --colors value '%s' (positive integer)\n", argv[i + 1]);
        return -1;
      }
      i += 2;
    } else {
      LOG_ERROR("❌ Unknown or incomplete option '%s'\n", argv[i]);
      return -1;
    }
  }
  return i;
}

static int command_encode_image(int argc, char **argv) {
  uint32_t codec = 0, max_colors = 0;
  int p = parse_encode_options(argc, argv, &codec, &max_colors);
  if (p < 0)
    return EXIT_FAILURE;
  if (argc - p != 2) {
    LOG_ERROR("❌ Usage: 4splat encode-image [--compress <scheme>] [--colors <N>] "
              "<in.ppm> <out.4spl>\n");
    return EXIT_FAILURE;
  }
  const char *in_path = argv[p], *out_path = argv[p + 1];

  uint32_t w = 0, h = 0;
  uint8_t *rgb = read_ppm(in_path, &w, &h);
  if (!rgb)
    return EXIT_FAILURE;

  Splat4DVideo video;
  const uint8_t *one_frame = rgb;
  bool built = frames_to_video_quantized(&one_frame, 1, w, h, max_colors, &video);
  free(rgb);
  if (!built) {
    LOG_ERROR("❌ Failed to build 4Splat video from image\n");
    return EXIT_FAILURE;
  }
  if (codec != SPLAT_COMPRESSION_NONE)
    set_flag_field(&video.header.flags, SPLAT_FLAG_COMPRESSION_MASK, SPLAT_FLAG_COMPRESSION_SHIFT,
                   codec);

  FILE *fp = fopen(out_path, "wb");
  if (!fp) {
    LOG_ERROR("❌ Unable to create '%s': %s\n", out_path, strerror(errno));
    free_splat4DVideo(&video);
    return EXIT_FAILURE;
  }
  bool wrote = write_splat4DVideo(fp, &video);
  fclose(fp);
  printf("✅ Encoded %ux%u image (%u colors) to '%s'\n", w, h, video.header.pSize, out_path);
  free_splat4DVideo(&video);
  return wrote ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int command_decode_image(int argc, char **argv) {
  if (argc != 2) {
    LOG_ERROR("❌ Usage: 4splat decode-image <in.4spl> <out.ppm>\n");
    return EXIT_FAILURE;
  }
  FILE *fp = fopen(argv[0], "rb");
  if (!fp) {
    LOG_ERROR("❌ Unable to open '%s': %s\n", argv[0], strerror(errno));
    return EXIT_FAILURE;
  }
  Splat4DVideo video;
  bool read_ok = read_splat4DVideo(fp, &video);
  fclose(fp);
  if (!read_ok) {
    LOG_ERROR("❌ Failed to read '%s'\n", argv[0]);
    return EXIT_FAILURE;
  }

  uint8_t *rgb = NULL;
  uint32_t w = 0, h = 0;
  bool ok = video_to_image(&video, &rgb, &w, &h);
  free_splat4DVideo(&video);
  if (!ok)
    return EXIT_FAILURE;

  bool wrote = write_ppm(argv[1], rgb, w, h);
  free(rgb);
  if (!wrote) {
    LOG_ERROR("❌ Failed to write '%s'\n", argv[1]);
    return EXIT_FAILURE;
  }
  printf("✅ Decoded '%s' to %ux%u image '%s'\n", argv[0], w, h, argv[1]);
  return EXIT_SUCCESS;
}

static int command_encode_video(int argc, char **argv) {
  uint32_t codec = 0, max_colors = 0;
  int a = parse_encode_options(argc, argv, &codec, &max_colors);
  if (a < 0)
    return EXIT_FAILURE;
  if (argc - a < 2) {
    LOG_ERROR("❌ Usage: 4splat encode-video [--compress <scheme>] [--colors <N>] "
              "<out.4spl> <frame.ppm>...\n");
    return EXIT_FAILURE;
  }
  const char *out_path = argv[a++];
  uint32_t nframes = (uint32_t)(argc - a);

  uint8_t **frames = calloc(nframes, sizeof(uint8_t *));
  if (!frames)
    return EXIT_FAILURE;

  uint32_t w = 0, h = 0;
  bool ok = true;
  for (uint32_t t = 0; t < nframes; ++t) {
    uint32_t fw = 0, fh = 0;
    frames[t] = read_ppm(argv[a + t], &fw, &fh);
    if (!frames[t]) {
      ok = false;
      break;
    }
    if (t == 0) {
      w = fw;
      h = fh;
    } else if (fw != w || fh != h) {
      LOG_ERROR("❌ Frame '%s' is %ux%u; expected %ux%u\n", argv[a + t], fw, fh, w, h);
      ok = false;
      break;
    }
  }

  Splat4DVideo video;
  bool built = ok && frames_to_video_quantized((const uint8_t *const *)frames, nframes, w, h,
                                               max_colors, &video);
  for (uint32_t t = 0; t < nframes; ++t)
    free(frames[t]);
  free(frames);
  if (!built) {
    if (ok)
      LOG_ERROR("❌ Failed to build 4Splat video from frames\n");
    return EXIT_FAILURE;
  }
  if (codec != SPLAT_COMPRESSION_NONE)
    set_flag_field(&video.header.flags, SPLAT_FLAG_COMPRESSION_MASK, SPLAT_FLAG_COMPRESSION_SHIFT,
                   codec);

  FILE *fp = fopen(out_path, "wb");
  if (!fp) {
    LOG_ERROR("❌ Unable to create '%s': %s\n", out_path, strerror(errno));
    free_splat4DVideo(&video);
    return EXIT_FAILURE;
  }
  bool wrote = write_splat4DVideo(fp, &video);
  fclose(fp);
  printf("✅ Encoded %u frame(s) %ux%u (%u colors) to '%s'\n", nframes, w, h, video.header.pSize,
         out_path);
  free_splat4DVideo(&video);
  return wrote ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int command_decode_video(int argc, char **argv) {
  if (argc != 2) {
    LOG_ERROR("❌ Usage: 4splat decode-video <in.4spl> <out-prefix>\n");
    return EXIT_FAILURE;
  }
  FILE *fp = fopen(argv[0], "rb");
  if (!fp) {
    LOG_ERROR("❌ Unable to open '%s': %s\n", argv[0], strerror(errno));
    return EXIT_FAILURE;
  }
  Splat4DVideo video;
  bool read_ok = read_splat4DVideo(fp, &video);
  fclose(fp);
  if (!read_ok) {
    LOG_ERROR("❌ Failed to read '%s'\n", argv[0]);
    return EXIT_FAILURE;
  }

  uint8_t **frames = NULL;
  uint32_t nframes = 0, w = 0, h = 0;
  bool ok = video_to_frames(&video, &frames, &nframes, &w, &h);
  free_splat4DVideo(&video);
  if (!ok)
    return EXIT_FAILURE;

  bool wrote = true;
  for (uint32_t t = 0; t < nframes && wrote; ++t) {
    char path[4096];
    SAFE_SNPRINTF(path, sizeof path, "%s%04u.ppm", argv[1], t);
    wrote = write_ppm(path, frames[t], w, h);
    if (!wrote)
      LOG_ERROR("❌ Failed to write '%s'\n", path);
  }
  for (uint32_t t = 0; t < nframes; ++t)
    free(frames[t]);
  free(frames);
  if (!wrote)
    return EXIT_FAILURE;

  printf("✅ Decoded '%s' to %u frame(s) %ux%u ('%s0000.ppm'...)\n", argv[0], nframes, w, h,
         argv[1]);
  return EXIT_SUCCESS;
}

static int command_encode_volume(int argc, char **argv) {
  uint32_t codec = 0, max_colors = 0;
  int a = parse_encode_options(argc, argv, &codec, &max_colors);
  if (a < 0)
    return EXIT_FAILURE;
  if (argc - a < 2) {
    LOG_ERROR("❌ Usage: 4splat encode-volume [--compress <scheme>] [--colors <N>] "
              "<out.4spl> <slice.ppm>...\n");
    return EXIT_FAILURE;
  }
  const char *out_path = argv[a++];
  uint32_t depth = (uint32_t)(argc - a);

  uint8_t **slices = calloc(depth, sizeof(uint8_t *));
  if (!slices)
    return EXIT_FAILURE;

  uint32_t w = 0, h = 0;
  bool ok = true;
  for (uint32_t z = 0; z < depth; ++z) {
    uint32_t fw = 0, fh = 0;
    slices[z] = read_ppm(argv[a + z], &fw, &fh);
    if (!slices[z]) {
      ok = false;
      break;
    }
    if (z == 0) {
      w = fw;
      h = fh;
    } else if (fw != w || fh != h) {
      LOG_ERROR("❌ Slice '%s' is %ux%u; expected %ux%u\n", argv[a + z], fw, fh, w, h);
      ok = false;
      break;
    }
  }

  Splat4DVideo video;
  bool built = ok && stack_to_video_quantized((const uint8_t *const *)slices, depth, 1, w, h,
                                              max_colors, &video);
  for (uint32_t z = 0; z < depth; ++z)
    free(slices[z]);
  free(slices);
  if (!built) {
    if (ok)
      LOG_ERROR("❌ Failed to build 4Splat volume from slices\n");
    return EXIT_FAILURE;
  }
  if (codec != SPLAT_COMPRESSION_NONE)
    set_flag_field(&video.header.flags, SPLAT_FLAG_COMPRESSION_MASK, SPLAT_FLAG_COMPRESSION_SHIFT,
                   codec);

  FILE *fp = fopen(out_path, "wb");
  if (!fp) {
    LOG_ERROR("❌ Unable to create '%s': %s\n", out_path, strerror(errno));
    free_splat4DVideo(&video);
    return EXIT_FAILURE;
  }
  bool wrote = write_splat4DVideo(fp, &video);
  fclose(fp);
  printf("✅ Encoded %u slice(s) %ux%u (%u colors) to '%s'\n", depth, w, h, video.header.pSize,
         out_path);
  free_splat4DVideo(&video);
  return wrote ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int command_decode_volume(int argc, char **argv) {
  if (argc != 2) {
    LOG_ERROR("❌ Usage: 4splat decode-volume <in.4spl> <out-prefix>\n");
    return EXIT_FAILURE;
  }
  FILE *fp = fopen(argv[0], "rb");
  if (!fp) {
    LOG_ERROR("❌ Unable to open '%s': %s\n", argv[0], strerror(errno));
    return EXIT_FAILURE;
  }
  Splat4DVideo video;
  bool read_ok = read_splat4DVideo(fp, &video);
  fclose(fp);
  if (!read_ok) {
    LOG_ERROR("❌ Failed to read '%s'\n", argv[0]);
    return EXIT_FAILURE;
  }

  uint8_t **slices = NULL;
  uint32_t nslices = 0, w = 0, h = 0;
  bool ok = video_to_slices(&video, &slices, &nslices, &w, &h);
  free_splat4DVideo(&video);
  if (!ok)
    return EXIT_FAILURE;

  bool wrote = true;
  for (uint32_t s = 0; s < nslices && wrote; ++s) {
    char path[4096];
    SAFE_SNPRINTF(path, sizeof path, "%s%04u.ppm", argv[1], s);
    wrote = write_ppm(path, slices[s], w, h);
    if (!wrote)
      LOG_ERROR("❌ Failed to write '%s'\n", path);
  }
  for (uint32_t s = 0; s < nslices; ++s)
    free(slices[s]);
  free(slices);
  if (!wrote)
    return EXIT_FAILURE;

  printf("✅ Decoded '%s' to %u slice(s) %ux%u ('%s0000.ppm'...)\n", argv[0], nslices, w, h,
         argv[1]);
  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    print_usage(stderr);
    return EXIT_FAILURE;
  }

  const char *command = argv[1];
  if (strcmp(command, "encode") == 0) {
    return command_encode(argc - 2, argv + 2);
  }
  if (strcmp(command, "decode") == 0) {
    return command_decode(argc - 2, argv + 2);
  }
  if (strcmp(command, "encode-image") == 0) {
    return command_encode_image(argc - 2, argv + 2);
  }
  if (strcmp(command, "decode-image") == 0) {
    return command_decode_image(argc - 2, argv + 2);
  }
  if (strcmp(command, "encode-video") == 0) {
    return command_encode_video(argc - 2, argv + 2);
  }
  if (strcmp(command, "decode-video") == 0) {
    return command_decode_video(argc - 2, argv + 2);
  }
  if (strcmp(command, "encode-volume") == 0) {
    return command_encode_volume(argc - 2, argv + 2);
  }
  if (strcmp(command, "decode-volume") == 0) {
    return command_decode_volume(argc - 2, argv + 2);
  }

  print_usage(stderr);
  return EXIT_FAILURE;
}
#endif // UNIT_TEST
