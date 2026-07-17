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

enum {
  SPLAT_FLAG_ENDIAN_BIG = 1u << 0,
  SPLAT_FLAG_PRECISION_SHIFT = 2,
  SPLAT_FLAG_PRECISION_MASK = 0x3u << SPLAT_FLAG_PRECISION_SHIFT,
  SPLAT_FLAG_PRECISION_FLOAT16 = 0x0u << SPLAT_FLAG_PRECISION_SHIFT,
  SPLAT_FLAG_PRECISION_FLOAT32 = 0x1u << SPLAT_FLAG_PRECISION_SHIFT,
  SPLAT_FLAG_PRECISION_FLOAT64 = 0x2u << SPLAT_FLAG_PRECISION_SHIFT,
  SPLAT_FLAG_PRECISION_FLOAT128 = 0x3u << SPLAT_FLAG_PRECISION_SHIFT,
  SPLAT_FLAG_SUPPORTED_MASK = SPLAT_FLAG_ENDIAN_BIG | SPLAT_FLAG_PRECISION_MASK,
};

static uint32_t sanitize_flags(uint32_t flags) {
  // 4Splat files are always little-endian.
  flags &= ~SPLAT_FLAG_ENDIAN_BIG;

  if ((flags & SPLAT_FLAG_PRECISION_MASK) == SPLAT_FLAG_PRECISION_FLOAT16)
    flags = (flags & ~SPLAT_FLAG_PRECISION_MASK) | SPLAT_FLAG_PRECISION_FLOAT32;

  return flags;
}

static bool flags_supported(uint32_t flags) {
  if (flags & ~SPLAT_FLAG_SUPPORTED_MASK) {
    fprintf(stderr, "❌ Unsupported flag combination (0x%08X)\n", flags);
    return false;
  }

  if (flags & SPLAT_FLAG_ENDIAN_BIG) {
    fprintf(stderr, "❌ Big-endian 4Splat files are unsupported\n");
    return false;
  }

  uint32_t precision = flags & SPLAT_FLAG_PRECISION_MASK;
  if (precision != SPLAT_FLAG_PRECISION_FLOAT32) {
    fprintf(stderr, "❌ Unsupported precision flag (0x%08X)\n", precision);
    return false;
  }

  return true;
}
enum { SPLAT4D_STREAM_CHUNK_SIZE = 1 << 15 };

typedef struct {
  float mu_x, sigma_x, mu_y, sigma_y, mu_z, sigma_z, mu_t, sigma_t, r, g, b, alpha;
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

static inline Splat4DFlags
splat4d_flags_make(SplatEndian endian, SplatSortOrder sort_order, SplatPrecision precision,
                   SplatCompression compression, SplatIndexWidth index_width, SplatShape shape,
                   SplatColorSpace color_space, SplatInterpolation interpolation) {
  Splat4DFlags flags = {.raw = 0};
  flags.bits.endian = endian;
  flags.bits.sorted = sort_order;
  flags.bits.precision = precision;
  flags.bits.compression = compression;
  flags.bits.index_width = index_width;
  flags.bits.splat_shape = shape;
  flags.bits.color_space = color_space;
  flags.bits.interpolation = interpolation;
  return flags;
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

uint32_t crc32(const void *data, size_t len) {
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
  for (size_t i = 0; i < n; i++) {
    c->v = (c->v >> 8) ^ crc32_table[(c->v ^ b[i]) & 0xFF];
  }
}

static inline uint32_t crc32_final(crc32_t *c) { return ~c->v; }

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

static bool splat4d_stream_video_payload(const Splat4DVideo *v, size_t chunk, Splat4DChunkFn fn,
                                         void *ctx) {
  if (!v || !fn)
    return false;

  if (!splat4d_stream_block((const uint8_t *)&v->header, sizeof v->header, chunk, fn, ctx))
    return false;

  uint64_t palette_bytes_u64;
  if (!checked_mul_u64((uint64_t)v->header.pSize, sizeof(Splat4D), &palette_bytes_u64))
    return false;
  if (palette_bytes_u64 > SIZE_MAX)
    return false;

  size_t palette_bytes = (size_t)palette_bytes_u64;
  if (palette_bytes > 0) {
    if (!v->palette.palette)
      return false;
    if (!splat4d_stream_block((const uint8_t *)v->palette.palette, palette_bytes, chunk, fn, ctx))
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

        if (idx_width == 1) {
          uint8_t *p = (uint8_t *)pack_buf;
          for (uint64_t i = 0; i < to_pack; i++)
            p[i] = (uint8_t)v->index.index[items_streamed + i];
        } else if (idx_width == 2) {
          uint16_t *p = (uint16_t *)pack_buf;
          for (uint64_t i = 0; i < to_pack; i++)
            p[i] = (uint16_t)v->index.index[items_streamed + i];
        } else if (idx_width == 4) {
          uint32_t *p = (uint32_t *)pack_buf;
          for (uint64_t i = 0; i < to_pack; i++)
            p[i] = (uint32_t)v->index.index[items_streamed + i];
        }

        if (idx_width > 0 && to_pack > SIZE_MAX / idx_width)
          return false;

        if (!fn(pack_buf, (size_t)(to_pack * idx_width), ctx))
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

uint32_t compute_idxoffset_forward(const Splat4DHeader *h) {
  return sizeof(Splat4DHeader) + h->pSize * sizeof(Splat4D);
}

uint32_t compute_idxoffset_reverse(const Splat4DHeader *h) {
  uint64_t total = header_total_indices(h);
  uint64_t header_bytes = sizeof(Splat4DHeader);
  uint64_t palette_bytes = (uint64_t)h->pSize * sizeof(Splat4D);
  uint8_t idx_width = get_index_width_bytes(h->flags);
  uint64_t index_bytes = total * idx_width;
  uint64_t footer_bytes = sizeof(Splat4DFooter);
  uint64_t filesize = header_bytes + palette_bytes + index_bytes + footer_bytes;
  uint64_t offset = filesize - (footer_bytes + index_bytes);
  return (uint32_t)offset;
}

bool sanity_check_idxoffset_file(FILE *fp, const Splat4DHeader *h, const Splat4DFooter *f) {
  (void)fp;
  uint64_t expect =
      (uint64_t)sizeof(Splat4DHeader) + (uint64_t)h->pSize * (uint64_t)sizeof(Splat4D);
  return f->idxoffset == expect;
}

bool check_idxoffset_file(FILE *fp, const Splat4DHeader *h, const Splat4DFooter *f) {
  (void)fp;
  uint64_t after_header =
      (uint64_t)sizeof(Splat4DHeader) + (uint64_t)h->pSize * (uint64_t)sizeof(Splat4D);
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
                         .version = {1, 0, 0, 0},
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
  int written = snprintf(buffer, sizeof buffer, "  %-12s %s", label, value);
  if (written < 0)
    return;

  size_t len = (size_t)written;
  const size_t width = 27; // width of the content between the │ characters

  fputs("│", out);
  fputs(buffer, out);
  if (len < width) {
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

  snprintf(buffer, sizeof buffer, "0x%X", encryption_bits);
  print_flag_line(stdout, "encryption", buffer);

  snprintf(buffer, sizeof buffer, "0x%02X", metadata_bits);
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
  return fwrite(h, sizeof(Splat4DHeader), 1, fp) == 1;
}

bool read_splat4DHeader(FILE *fp, Splat4DHeader *h) {
  if (!fp || !h)
    return false;
  return fread(h, sizeof(Splat4DHeader), 1, fp) == 1;
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

bool write_splat4DPalette(FILE *fp, const Splat4DPalette *p, uint32_t count) {
  if (!fp || !p || !p->palette || count == 0)
    return false;
  return fwrite(p->palette, sizeof(Splat4D), count, fp) == count;
}

bool read_splat4DPalette(FILE *fp, Splat4DPalette *p, uint32_t count) {
  if (!fp || !p || count == 0)
    return false;
  uint64_t bytes64 = 0;
  if (!checked_mul_u64((uint64_t)count, (uint64_t)sizeof(Splat4D), &bytes64) || bytes64 > SIZE_MAX)
    return false;
  size_t bytes = (size_t)bytes64;
  p->palette = malloc(bytes);
  if (!p->palette)
    return false;
  if (fread(p->palette, sizeof(Splat4D), count, fp) != count) {
    free(p->palette);
    p->palette = NULL;
    return false;
  }
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

      if (idx_width == 1) {
        uint8_t *p = (uint8_t *)pack_buf;
        for (uint64_t k = 0; k < to_pack; k++)
          p[k] = (uint8_t)i->index[items_written + k];
      } else if (idx_width == 2) {
        uint16_t *p = (uint16_t *)pack_buf;
        for (uint64_t k = 0; k < to_pack; k++)
          p[k] = (uint16_t)i->index[items_written + k];
      } else if (idx_width == 4) {
        uint32_t *p = (uint32_t *)pack_buf;
        for (uint64_t k = 0; k < to_pack; k++)
          p[k] = (uint32_t)i->index[items_written + k];
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

      if (idx_width == 1) {
        uint8_t *p = (uint8_t *)pack_buf;
        for (uint64_t k = 0; k < to_read; k++)
          i->index[items_read + k] = p[k];
      } else if (idx_width == 2) {
        uint16_t *p = (uint16_t *)pack_buf;
        for (uint64_t k = 0; k < to_read; k++)
          i->index[items_read + k] = p[k];
      } else if (idx_width == 4) {
        uint32_t *p = (uint32_t *)pack_buf;
        for (uint64_t k = 0; k < to_read; k++)
          i->index[items_read + k] = p[k];
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
      .idxoffset = sizeof(Splat4DHeader) + h->pSize * sizeof(Splat4D),
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
  return fwrite(f, sizeof(Splat4DFooter), 1, fp) == 1;
}

bool read_splat4DFooter(FILE *fp, Splat4DFooter *f) {
  if (!fp || !f)
    return false;
  return fread(f, sizeof(Splat4DFooter), 1, fp) == 1;
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

bool write_splat4DVideo(FILE *fp, Splat4DVideo *v) {
  if (!fp || !v)
    return false;

  // Compute header-derived values
  v->footer.idxoffset = sizeof(Splat4DHeader) + v->header.pSize * sizeof(Splat4D);

  crc32_t c;
  crc32_init(&c);
  Splat4DStreamFileCtx ctx = {.fp = fp, .crc = &c};
  if (!stream_splat4DVideo(v, SPLAT4D_STREAM_CHUNK_SIZE, splat4d_stream_file_consumer, &ctx))
    return false;
  v->footer.checksum = crc32_final(&c);

  // Return to end of file and write footer
  fseek(fp, 0, SEEK_END);
  if (!write_splat4DFooter(fp, &v->footer))
    return false;

  return true;
}

bool read_splat4DVideo(FILE *fp, Splat4DVideo *v) {
  if (!fp || !v)
    return false;

  // Read header
  if (!read_splat4DHeader(fp, &v->header))
    return false;

  if (!flags_supported(v->header.flags))
    return false;
  if (v->header.pSize == 0) {
    fprintf(stderr, "❌ Invalid palette size\n");
    return false;
  }

  uint64_t palette_bytes = 0;
  if (!checked_mul_u64((uint64_t)v->header.pSize, (uint64_t)sizeof(Splat4D), &palette_bytes) ||
      palette_bytes > SIZE_MAX) {
    fprintf(stderr, "❌ Invalid palette size\n");
    return false;
  }

  uint64_t total = 0;
  if (!header_total_indices_checked(&v->header, &total)) {
    fprintf(stderr, "❌ Invalid index count\n");
    return false;
  }

  uint64_t index_bytes = 0;
  if (!checked_mul_u64(total, (uint64_t)sizeof(uint64_t), &index_bytes) || index_bytes > SIZE_MAX) {
    fprintf(stderr, "❌ Invalid index count\n");
    return false;
  }

  // Read palette
  if (!read_splat4DPalette(fp, &v->palette, v->header.pSize))
    return false;

  // Read index
  if (!read_splat4DIndex(fp, &v->index, total, v->header.flags)) {
    free(v->palette.palette);
    v->palette.palette = NULL;
    return false;
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
    fprintf(stderr, "❌ CRC mismatch: file=0x%08X recomputed=0x%08X\n", v->footer.checksum,
            recomputed);
    free(v->palette.palette);
    free(v->index.index);
    v->palette.palette = NULL;
    v->index.index = NULL;
    return false;
  }

  // 2. Validate offset consistency
  if (!sanity_check_idxoffset_file(fp, &v->header, &v->footer)) {
    fprintf(stderr, "❌ Index offset mismatch (footer=%" PRIu64 ", expect=%" PRIu64 ")\n",
            (uint64_t)v->footer.idxoffset,
            (uint64_t)sizeof(Splat4DHeader) +
                (uint64_t)v->header.pSize * (uint64_t)sizeof(Splat4D));
    // free allocations before returning
    free(v->palette.palette);
    free(v->index.index);
    v->palette.palette = NULL;
    v->index.index = NULL;
    return false;
  }

  // 3. Validate footer end marker
  if (v->footer.end != 0x4C505334) {
    fprintf(stderr, "❌ Invalid footer end marker\n");
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
    fprintf(stderr, "❌ Video reference required\n");
    return false;
  }

  if (v->header.magic != 0x3453504C) {
    fprintf(stderr, "❌ Unsupported format\n");
    return false;
  }

  if (v->header.version[0] != 1) {
    fprintf(stderr, "❌ Unsupported version\n");
    return false;
  }

  if (v->header.height == 0) {
    fprintf(stderr, "❌ Positive height required\n");
    return false;
  }

  if (v->header.width == 0) {
    fprintf(stderr, "❌ Positive width required\n");
    return false;
  }

  if (v->header.depth == 0) {
    fprintf(stderr, "❌ Positive depth required\n");
    return false;
  }

  if (v->header.frames == 0) {
    fprintf(stderr, "❌ Positive number of frames required\n");
    return false;
  }

  if (v->header.pSize == 0) {
    fprintf(stderr, "❌ Positive palette size required\n");
    return false;
  }

  if (!flags_supported(v->header.flags))
    return false;

  if (v->footer.end != 0x4C505334) {
    fprintf(stderr, "❌ Invalid end-of-file marker\n");
    return false;
  }

  uint32_t expected = compute_video_checksum(v);
  if (v->footer.checksum != expected) {
    fprintf(stderr, "❌ Checksum mismatch (got 0x%08X expected 0x%08X)\n", v->footer.checksum,
            expected);
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

static void print_usage(FILE *stream) {
  fprintf(stream,
          "Usage:\n"
          "  4splat encode --palette <palette.bin> --index <index.bin> --output <file.4spl> "
          "--width <w> --height <h> --depth <d> --frames <f> [--palette-size <n>] [--flags <n>]\n"
          "  4splat decode --input <file.4spl> [--palette <palette.bin>] [--index <index.bin>] "
          "[--print] [--validate]\n");
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
    fprintf(stderr, "❌ Unable to open '%s': %s\n", path, strerror(errno));
    return false;
  }

  if (fseek(fp, 0, SEEK_END) != 0) {
    fprintf(stderr, "❌ Failed to seek '%s'\n", path);
    fclose(fp);
    return false;
  }

  long size = ftell(fp);
  if (size < 0) {
    fprintf(stderr, "❌ Failed to determine size of '%s'\n", path);
    fclose(fp);
    return false;
  }

  if (size % (long)element_size != 0) {
    fprintf(stderr, "❌ File '%s' is not aligned to element size %zu\n", path, element_size);
    fclose(fp);
    return false;
  }

  uint64_t count = (uint64_t)size / element_size;
  if (count == 0) {
    fprintf(stderr, "❌ File '%s' does not contain any entries\n", path);
    fclose(fp);
    return false;
  }

  if (fseek(fp, 0, SEEK_SET) != 0) {
    fprintf(stderr, "❌ Failed to rewind '%s'\n", path);
    fclose(fp);
    return false;
  }

  if (element_size == 0 || count > SIZE_MAX / element_size) {
    fprintf(stderr, "❌ File '%s' is too large to load into memory\n", path);
    fclose(fp);
    return false;
  }

  void *data = malloc(count * element_size);
  if (!data) {
    fprintf(stderr, "❌ Out of memory while reading '%s'\n", path);
    fclose(fp);
    return false;
  }

  if (fread(data, element_size, count, fp) != count) {
    fprintf(stderr, "❌ Failed to read '%s'\n", path);
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
    fprintf(stderr, "❌ Palette '%s' has too many entries (%" PRIu64 ")\n", path, count);
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
    fprintf(stderr, "❌ Unable to write '%s': %s\n", path, strerror(errno));
    return false;
  }
  size_t written = fwrite(buffer, element_size, count, fp);
  fclose(fp);
  if (written != count) {
    fprintf(stderr, "❌ Short write while writing '%s'\n", path);
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

typedef struct {
  const char *palette_path;
  const char *index_path;
  const char *output_path;
  MetadataOptions meta;
} EncodeOptions;

static int execute_encode(EncodeOptions *opts) {
  Splat4D *palette = NULL;
  uint32_t palette_count = 0;
  if (!load_palette_from_file(opts->palette_path, &palette, &palette_count))
    return EXIT_FAILURE;

  if (!opts->meta.palette_size_set)
    opts->meta.palette_size = palette_count;

  if (opts->meta.palette_size != palette_count) {
    fprintf(stderr, "❌ Palette size mismatch: option=%u file=%u\n", opts->meta.palette_size,
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
    fprintf(stderr,
            "❌ Index count mismatch: expected %" PRIu64 " (from dimensions) but file has %" PRIu64
            " entries\n",
            expected_indices, index_count);
    free(palette);
    free(indices);
    return EXIT_FAILURE;
  }

  Splat4DHeader header =
      create_splat4DHeader(opts->meta.width, opts->meta.height, opts->meta.depth, opts->meta.frames,
                           opts->meta.palette_size, opts->meta.flags_set ? opts->meta.flags : 0u);
  Splat4DVideo video = create_splat4DVideo(header, palette, indices);

  FILE *fp = fopen(opts->output_path, "wb");
  if (!fp) {
    fprintf(stderr, "❌ Unable to create '%s': %s\n", opts->output_path, strerror(errno));
    free_splat4DVideo(&video);
    return EXIT_FAILURE;
  }

  bool wrote = write_splat4DVideo(fp, &video);
  fclose(fp);
  free_splat4DVideo(&video);

  if (!wrote) {
    fprintf(stderr, "❌ Failed to write 4Splat file '%s'\n", opts->output_path);
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
    } else if (strcmp(arg, "--print") == 0) {
      print_summary = true;
    } else if (strcmp(arg, "--validate") == 0) {
      do_validate = true;
    } else {
      fprintf(stderr, "❌ Unknown or incomplete option '%s'\n", arg);
      return EXIT_FAILURE;
    }
  }

  if (!input_path) {
    fprintf(stderr, "❌ decode requires --input <file.4spl>\n");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(input_path, "rb");
  if (!fp) {
    fprintf(stderr, "❌ Unable to open '%s': %s\n", input_path, strerror(errno));
    return EXIT_FAILURE;
  }

  Splat4DVideo video;
  bool read_ok = read_splat4DVideo(fp, &video);
  fclose(fp);

  if (!read_ok) {
    fprintf(stderr, "❌ Failed to read '%s'\n", input_path);
    return EXIT_FAILURE;
  }

  if (do_validate && !validate_splat4DVideo(&video)) {
    free_splat4DVideo(&video);
    return EXIT_FAILURE;
  }

  if (print_summary)
    print_splat4DVideo(&video);

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

  print_usage(stderr);
  return EXIT_FAILURE;
}
#endif // UNIT_TEST
