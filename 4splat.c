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

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static inline Splat4DFlags splat4d_flags_default(void) {
  return splat4d_flags_make(SPLAT_ENDIAN_LITTLE, SPLAT_SORT_UNSORTED, SPLAT_PRECISION_FLOAT32,
                            SPLAT_COMPRESSION_NONE, SPLAT_INDEX_WIDTH_32, SPLAT_SHAPE_ISOTROPIC,
                            SPLAT_COLOR_SRGB, SPLAT_INTERP_NONE);
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
  Splat4DFlags flags;
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

// utils //
typedef struct {
  uint32_t v;
} crc32_t;

uint32_t crc32(const void *data, size_t len) {
  const uint8_t *p = data;
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= p[i];
    for (unsigned j = 0; j < 8; j++)
      crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
  }
  return ~crc;
}

static inline void crc32_init(crc32_t *c) { c->v = 0xFFFFFFFFu; }

static inline void crc32_update(crc32_t *c, const void *p, size_t n) {
  const uint8_t *b = p;
  for (size_t i = 0; i < n; i++) {
    c->v ^= b[i];
    for (unsigned j = 0; j < 8; j++)
      c->v = (c->v >> 1) ^ (0xEDB88320u & -(c->v & 1u));
  }
}

static inline uint32_t crc32_final(crc32_t *c) { return ~c->v; }

uint64_t header_total_indices(const Splat4DHeader *h) {
  return (uint64_t)h->width * (uint64_t)h->height * (uint64_t)h->depth * (uint64_t)h->frames;
}

uint32_t compute_video_checksum(const Splat4DVideo *v) {
  uint64_t total = header_total_indices(&v->header);
  size_t total_bytes =
      sizeof(Splat4DHeader) + v->header.pSize * sizeof(Splat4D) + total * sizeof(uint64_t);

  // Allocate a temporary contiguous buffer
  uint8_t *buf = malloc(total_bytes);
  if (!buf)
    return 0;

  uint8_t *ptr = buf;

  memcpy(ptr, &v->header, sizeof(Splat4DHeader));
  ptr += sizeof(Splat4DHeader);

  memcpy(ptr, v->palette.palette, v->header.pSize * sizeof(Splat4D));
  ptr += v->header.pSize * sizeof(Splat4D);

  memcpy(ptr, v->index.index, total * sizeof(uint64_t));

  uint32_t crc = crc32(buf, total_bytes);
  free(buf);
  return crc;
}

uint32_t compute_idxoffset_forward(const Splat4DHeader *h) {
  return sizeof(Splat4DHeader) + h->pSize * sizeof(Splat4D);
}

uint32_t compute_idxoffset_reverse(const Splat4DHeader *h) {
  uint64_t total = header_total_indices(h);
  uint32_t filesize = sizeof(Splat4DHeader) + h->pSize * sizeof(Splat4D) +
                      total * sizeof(uint64_t) + sizeof(Splat4DFooter);
  return filesize - (sizeof(Splat4DFooter) + total * sizeof(uint64_t));
}

bool sanity_check_idxoffset_file(FILE *fp, const Splat4DHeader *h, const Splat4DFooter *f) {
  uint64_t expect =
      (uint64_t)sizeof(Splat4DHeader) + (uint64_t)h->pSize * (uint64_t)sizeof(Splat4D);
  return f->idxoffset == expect;
}

bool check_idxoffset_file(FILE *fp, const Splat4DHeader *h, const Splat4DFooter *f) {
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
                                   uint32_t pSize, Splat4DFlags flags) {
  return (Splat4DHeader){.magic = 0x3453504C,
                         .version = {1, 0, 0, 0},
                         .width = width,
                         .height = height,
                         .depth = depth,
                         .frames = frames,
                         .pSize = pSize,
                         .flags = flags};
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
  printf("│  flags        0X%-10X │\n", h->flags.raw);
  printf("│    endian     %-14s │\n", splat_endian_name((SplatEndian)h->flags.bits.endian));
  printf("│    sort       %-14s │\n", splat_sort_order_name((SplatSortOrder)h->flags.bits.sorted));
  printf("│    precision  %-14s │\n",
         splat_precision_name((SplatPrecision)h->flags.bits.precision));
  printf("│    compress   %-14s │\n",
         splat_compression_name((SplatCompression)h->flags.bits.compression));
  printf("│    index      %-14s │\n",
         splat_index_width_name((SplatIndexWidth)h->flags.bits.index_width));
  printf("│    shape      %-14s │\n", splat_shape_name((SplatShape)h->flags.bits.splat_shape));
  printf("│    color      %-14s │\n",
         splat_color_space_name((SplatColorSpace)h->flags.bits.color_space));
  printf("│    interp     %-14s │\n",
         splat_interpolation_name((SplatInterpolation)h->flags.bits.interpolation));
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
  p->palette = malloc(count * sizeof(Splat4D));
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

bool write_splat4DIndex(FILE *fp, const Splat4DIndex *i, uint64_t total) {
  if (!fp || !i || !i->index || total == 0)
    return false;
  return fwrite(i->index, sizeof(uint64_t), total, fp) == total;
}

bool read_splat4DIndex(FILE *fp, Splat4DIndex *i, uint64_t total) {
  if (!fp || !i || total == 0)
    return false;
  i->index = malloc(total * sizeof(uint64_t));
  if (!i->index)
    return false;
  if (fread(i->index, sizeof(uint64_t), total, fp) != total) {
    free(i->index);
    i->index = NULL;
    return false;
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
  printf("│    idxoffset 0x%08lX   │\n", f->idxoffset);
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
  uint64_t total = header_total_indices(&v->header);
  v->footer.idxoffset = sizeof(Splat4DHeader) + v->header.pSize * sizeof(Splat4D);

  // Write everything but footer first
  // long start_pos = ftell(fp);
  if (!write_splat4DHeader(fp, &v->header))
    return false;
  if (!write_splat4DPalette(fp, &v->palette, v->header.pSize))
    return false;
  if (!write_splat4DIndex(fp, &v->index, total))
    return false;
  // long end_pos = ftell(fp);

  // Compute CRC32 of everything before footer
  crc32_t c;
  crc32_init(&c);
  crc32_update(&c, &v->header, sizeof v->header);
  crc32_update(&c, v->palette.palette, v->header.pSize * sizeof(Splat4D));
  crc32_update(&c, v->index.index, total * sizeof(uint64_t));
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

  // Read palette
  if (!read_splat4DPalette(fp, &v->palette, v->header.pSize))
    return false;

  // Read index
  uint64_t total = header_total_indices(&v->header);
  if (!read_splat4DIndex(fp, &v->index, total)) {
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
  // 1. Recompute CRC
  long current = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  uint8_t *buf = malloc(v->footer.idxoffset + total * sizeof(uint64_t));
  if (!buf)
    return false;
  fread(buf, 1, v->footer.idxoffset + total * sizeof(uint64_t), fp);
  uint32_t recomputed = crc32(buf, v->footer.idxoffset + total * sizeof(uint64_t));
  free(buf);
  fseek(fp, current, SEEK_SET);

  if (recomputed != v->footer.checksum) {
    fprintf(stderr, "❌ CRC mismatch: file=0x%08X recomputed=0x%08X\n", v->footer.checksum,
            recomputed);
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
// main
int main(void) {
  const uint32_t width = 2;
  const uint32_t height = 2;
  const uint32_t depth = 1;
  const uint32_t frames = 1;
  const uint32_t palette_size = 2;
  const Splat4DFlags flags = splat4d_flags_default();
  // const uint32_t indicies = 4;

  Splat4DHeader head = create_splat4DHeader(width, height, depth, // width, height, depth
                                            frames, palette_size, // frames, palette size
                                            flags);               // flags
  Splat4D splats[2] = {create_splat4D(0, 1, 2, 3, 4, 5, 6, 7, 0.5, 0.6, 0.7, 1.0),
                       create_splat4D(1, 1, 2, 2, 3, 3, 4, 4, 0.8, 0.2, 0.1, 0.9)};
  uint64_t idxs[4] = {0, 1, 0, 1};
  Splat4DVideo video = create_splat4DVideo(head, splats, idxs);

  FILE *out = fopen("demo.4spl", "wb+");
  write_splat4DVideo(out, &video);
  fclose(out);

  FILE *in = fopen("demo.4spl", "rb");
  Splat4DVideo loaded;
  if (read_splat4DVideo(in, &loaded)) {
    print_splat4DVideo(&loaded);
    printf("✅ File verified OK!\n");
  } else {
    fprintf(stderr, "❌ Verification failed.\n");
  }
  fclose(in);

  validate_splat4DVideo(&video);

  free_splat4DVideo(&loaded);

  return 0;
}
#endif // UNIT_TEST
