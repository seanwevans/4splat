<img width="256" alt="splat!" src="https://github.com/user-attachments/assets/63774bbb-7322-4296-b1f4-9be2b8741221" />


```
  ╭ .4spl File Format ╮
╭─╯                   ╰───────────────────────────────────────────────────╮
│ The 4Splat Codec is a palette-based, lossless video compression format. │
│ It generalizes indexed-color images to 3D and 4D spatiotemporal data:   │
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
│▒│ version      4 bytes     uint8[4]    {1,1,0,0}  0x01 0x01 0x00 0x00 │▒│
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
 ╰───────────────────────────────────────────────────────────────────────╯
```

## Benchmarks

Every number below is produced by `make bench-full` and `make bench-volumes`,
and every one of them was verified by decoding the file again and comparing it
with the source. The short version: **the format's margin scales with how many
axes the data has.** It loses badly on a photograph, draws level on pixel art,
and pulls ahead by 3-14x on a 4D grid, because one palette amortizes across
every voxel and frame while its 48-bytes-per-color cost is paid only once.

### Images

128x128 (`noise` and `sprite` are 64x64), sizes in bytes, smaller is better:

| Clip | Colors | `.4spl` | PNG | PNG8 | GIF | QOI | xz(raw) |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `checkerboard` 2-color pattern | 2 | **176** | 381 | 203 | 768 | 4,094 | 188 |
| `sprite` pixel art | 16 | 1,026 | 1,025 | **456** | 1,122 | 1,906 | 492 |
| `photo` continuous tone | 11,922 | 592,973 | **22,596** | n/a | n/a | 29,331 | 32,504 |
| `gradient` smooth ramp | 16,384 | 791,819 | **261** | n/a | n/a | 33,043 | 37,256 |
| `noise` random RGB | 4,095 | 198,281 | 12,420 | n/a | n/a | 16,398 | **12,348** |

### Video

A global palette shared across frames - the same model as an animated GIF, and
the first place the format pulls clearly ahead:

| Clip | Colors | `.4spl` | PNG/frame | PNG8/frame | GIF | QOI | xz(raw) |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `bounce` 64x64 x 12 frames | 3 | **313** | 3,086 | 2,072 | 1,803 | 2,365 | 344 |
| `screencast` 96x64 x 8 frames | 4 | **1,262** | 10,668 | 7,780 | 6,278 | 21,867 | 1,400 |

### Volumes

Against the containers built to hold a whole `(x, y, z)` grid as one object,
not against 2D image formats:

| Volume | Colors | `.4spl` | `.nii.gz` | NRRD | MetaImage | TIFF (pal) | PNG/slice | GIF |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `labels` 64x64x24 segmentation | 6 | **1,771** | 3,077 | 3,255 | 3,300 | 7,583 | 8,010 | 5,126 |
| `angio` 64x64x24 sparse | 3 | **1,165** | 2,057 | 2,225 | 2,270 | 7,158 | 8,118 | 3,826 |
| `voxel` 64x64x20 terrain | 9 | **2,868** | 4,593 | 4,764 | 4,809 | 8,294 | 12,211 | 13,454 |
| `ct` 64x64x16 continuous | 256 | 46,211 | 59,826 | 60,013 | 60,059 | 55,245 | **47,543** | 60,139 |

### 4D grids

The `(x, y, z, frame)` shape the format is named for. NIfTI's `dim` is literally
`(x, y, z, t)`, so this is a like-for-like comparison of two headers describing
the same grid - and it is the format's best showing anywhere:

| Grid | Colors | `.4spl` | `.nii.gz` | NRRD | MetaImage | TIFF | PNG/slice | GIF |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `beating` 48x48x8 x 6 frames | 4 | **1,359** | 4,619 | 4,784 | 4,842 | 14,384 | 19,060 | 11,005 |
| `perfusion` 40x40x6 x 5 frames | 256 | 34,382 | 38,576 | 38,761 | 38,820 | 44,351 | **37,716** | 47,411 |

### Lossy: `--colors N`

Median-cut quantization is what makes many-colored content viable at all. PSNR
against the source, `.4spl` sizes with the best scheme:

| Clip | Exact | 64 colors | 16 or 32 colors | Best lossless rival |
| --- | ---: | ---: | ---: | ---: |
| `photo` | 592,973 | 9,202 (36.9 dB) | 4,491 (32.1 dB) | 22,596 (PNG) |
| `gradient` | 791,819 | 3,224 (29.6 dB) | 864 (23.6 dB) | 261 (PNG) |
| `noise` | 198,281 | 6,206 (22.8 dB) | 2,815 (18.0 dB) | 12,299 (gzip) |
| `ct` | 46,211 | - | 15,139 (41.1 dB) | 35,809 (bzip2) |
| `perfusion` | 34,382 | - | 8,130 (41.3 dB) | 17,886 (bzip2) |

### What the numbers say

* **Cost per color is the whole story on the losing side.** A palette entry is
  12 floats - 48 bytes at float32 - so an exact palette costs
  `48 x distinct_colors` before a single index is written. At 11,922 colors that
  is 572 KB of palette against a 49 KB image. The header allows float16, which
  would halve it, but the image/video codecs do not expose the precision flag
  yet.
* **One palette across many slices is the whole story on the winning side.**
  `.4spl` substitutes a one-byte index per voxel before compression, so the
  stream reaching the compressor is a third the size - and unlike every
  per-slice format, that palette is paid once for the entire grid. Against real
  volumetric containers that is 1.6-2.0x on structured data; against per-slice
  PNG, 4-7x; on a 4D grid, 3.4x and 14x.
* **GIF is the closest existing relative and is beaten on its own ground** -
  5x on `screencast`, 5.8x on `bounce`, both with a global color table.
* **TIFF is the only mainstream format with the same idea.** A palette stack
  sharing one `ColorMap` is its best showing, and it still lands 2.9-10.6x
  behind because each page carries its own Deflate stream.
* **`bzip2` over raw bytes is a stubborn baseline.** It ties `.4spl` on
  `labels` and beats it on `checkerboard`, `bounce`, `angio`, `ct` and
  `perfusion`. Structured data has redundancy a block-sorting compressor finds
  without knowing it is a grid.
* **Scheme choice matters far less than palette size.** Brotli wins most clips,
  lzma is within ~4%, deflate and zlib differ by the 6-byte zlib wrapper, and
  RLE - the only real scheme in the dependency-free build - trails by 2.5-24x.

Methodology, the full contender list and how to run these against your own
files are in [Benchmarks against existing formats](#benchmarks-against-existing-formats)
and [Volumetric benchmark against 3D formats](#volumetric-benchmark-against-3d-formats).

## Palette layout (v0.2)

Version `{1,1,0,0}` pins the palette layout, resolving an ambiguity in the
original diagram. Each entry stores its floats in this field order (all
little-endian, at the width set by the precision flag):

```
mu_x, mu_y, mu_z,  <covariance block>,  mu_t, sigma_t,  r, g, b, alpha
```

The covariance block's size is set by the **splat shape** flag, so the entry
size varies (the 48-byte entry in the diagram above is the axis-aligned case):

| Shape | Covariance block | Floats | Bytes (float32) |
| --- | --- | --- | --- |
| Isotropic (`00`) | `sigma` (one shared spatial σ) | 10 | 40 |
| Axis-Aligned (`01`) | `sigma_x, sigma_y, sigma_z` | 12 | 48 |
| Full Covariance (`10`) | `sigma_x, sigma_y, sigma_z, sigma_xy, sigma_xz, sigma_yz` | 15 | 60 |

Full covariance stores the symmetric 3×3 spatial covariance as its diagonal
followed by the upper-triangle off-diagonals. An isotropic entry expands its one
σ to all three axes on read. The palette section is therefore
`paletteSize * entry_bytes`, with `entry_bytes` derived from the shape and
precision flags.

## Building

The codec is a single translation unit. A bare build is fully self-contained and
needs no third-party libraries:

```bash
make plain          # or: gcc -Wall -Wpedantic -std=c11 -o 4splat 4splat.c
```

In this configuration the index payload can be stored with the **None** or
**RLE** compression schemes. The remaining schemes in the format spec are
provided by mature third-party libraries, enabled at compile time:

```bash
make                # full-featured build (links all backends below)
```

| Flag value | Scheme | Backend | Build macro |
| --- | --- | --- | --- |
| `0000` | None | built-in | always |
| `0001` | RLE | built-in | always |
| `0010` | DEFLATE | zlib (raw) | `SPLAT_WITH_ZLIB` |
| `0101` | zlib | zlib | `SPLAT_WITH_ZLIB` |
| `0110` | bzip2 | libbz2 | `SPLAT_WITH_BZIP2` |
| `0111` | LZMA | liblzma | `SPLAT_WITH_LZMA` |
| `1001` | XZ | liblzma | `SPLAT_WITH_LZMA` |
| `1010` | LZ4 | liblz4 | `SPLAT_WITH_LZ4` |
| `1101` | Brotli | libbrotli | `SPLAT_WITH_BROTLI` |
| `1111` | Zstd | libzstd | `SPLAT_WITH_ZSTD` |

`SPLAT_WITH_ALL` turns on every backend at once. The remaining scheme values
(RAR, LZO, ZPAQ, Snappy, LZHAM, LZFSE) have no free-to-link encoder available and
are rejected on read with a clear diagnostic. Compression applies only to the
index section; the checksum always covers the uncompressed logical payload, so a
file written by one build reads identically on another regardless of the codec
library version.

The palette is stored at the precision named by the header's precision field —
float16, float32 (default) or float64 — and every descriptive flag field (index
width, splat shape, color space, interpolation, sort order and the metadata byte)
round-trips unchanged.

## Selecting flags on the command line

Rather than computing a raw `--flags` integer, `encode` accepts a named option
for each flag field:

```bash
4splat encode --palette pal.bin --index idx.bin --output out.4spl \
  --width 20 --height 20 --depth 1 --frames 1 \
  --precision float64 --compression zstd --index-width 2 \
  --color-space rec2020 --splat-shape axis-aligned --interpolation lanczos \
  --sorted --metadata 42
```

| Option | Values |
| --- | --- |
| `--precision` | `float16`, `float32` (default), `float64` |
| `--compression` | `none`, `rle`, `deflate`, `zlib`, `bzip2`, `lzma`, `xz`, `lz4`, `brotli`, `zstd` (plus the spec's other names, rejected if unbuilt) |
| `--index-width` | `1`, `2`, `4`, `8` (bytes) |
| `--splat-shape` | `isotropic`, `axis-aligned`, `full-covariance` |
| `--color-space` | `srgb`, `rec2020`, `display-p3`, … (see below) |
| `--interpolation` | `none`, `nearest`, `lanczos`, `gaussian`, … |
| `--sorted` | (flag, no value) |
| `--metadata` | `0`–`255` |

`encode` refuses up front to write a file the current build could not read back
— for example selecting `--compression zstd` in the dependency-free build fails
with a clear message rather than producing an unreadable file. A raw `--flags`
value is still accepted and individual named options override their field.

## Image & video codec

The format is a video codec with a **global palette shared across all frames**
(the same model as an animated GIF's global color table). Each distinct color
across every frame becomes a palette splat — its `r/g/b`, with spatial
`mu`/`sigma` from the pixels that use it and temporal `mu_t`/`sigma_t` from the
frames it appears in — and every pixel stores the index of its color, in
`t`-major then row-major order. 8-bit RGB round-trips exactly.

A single image is simply the `frames == 1` collapse of the video codec — a
pseudo-GIF — and shares the same code path.

```bash
# image (one frame)
4splat encode-image [--compress <scheme>] [--colors <N>] input.ppm output.4spl
4splat decode-image output.4spl restored.ppm

# video (frames share one palette)
4splat encode-video [--compress <scheme>] [--colors <N>] out.4spl frame0.ppm frame1.ppm ...
4splat decode-video out.4spl restored_        # writes restored_0000.ppm, ...

# volume (a stack of z-slices; depth > 1, frames = 1)
4splat encode-volume [--compress <scheme>] [--colors <N>] vol.4spl slice0.ppm slice1.ppm ...
4splat decode-volume vol.4spl restored_       # writes restored_0000.ppm, ...

# 4D grid (a volume that changes over time; depth > 1, frames > 1)
4splat encode-4d [--compress <scheme>] [--colors <N>] --depth 8 grid.4spl slice0.ppm ...
4splat decode-4d grid.4spl restored_          # writes restored_0000.ppm, ...
```

`encode-4d` takes `depth * frames` slices in **t-major, z-minor** order - slice
`s` is `t = s / depth`, `z = s % depth`, the same order the index itself uses -
and refuses a count that is not a whole number of volumes. This is the full
`(x, y, z, frame)` grid the format is named for: one palette spanning depth
*and* time, with each entry carrying `mu_z`/`sigma_z` and `mu_t`/`sigma_t`.

All of these share one implementation over the format's `(x, y, z, t)` grid: an
image is `depth = frames = 1`, a video is `depth = 1, frames = N`, a volume is
`depth = N, frames = 1`, and `encode-4d` is the general case with both axes set.
A volume's splats carry real `mu_z`/`sigma_z` from the slices each color
occupies, just as a video's carry `mu_t`/`sigma_t`; a 4D grid carries both.

Input/output is binary PPM (`P6`, maxval 255); all frames must share dimensions.
`--compress` (e.g. `zstd`, `rle`) compresses the index — a 2-color checkerboard
shrinks from a 30 KB PPM to a few hundred bytes, decoding back bit-for-bit.

Without `--colors` the palette is **exact and lossless** (one entry per distinct
color). `--colors N` runs **median-cut quantization** down to at most `N`
colors — lossy, but it makes photographic content compress: a 1024-color
gradient at `--colors 16` drops from ~51 KB to ~1.8 KB. Quantization is shared
across all frames, so it acts as a global palette for the whole clip.

## Color-space conversion

When built with LittleCMS (`SPLAT_WITH_LCMS2`, included in `make`), `decode` can
convert the palette colors from the space named in the header to another space
and update the color-space field:

```bash
4splat decode --input in.4spl --to-color rec2020 --output out.4spl
```

Supported target/source spaces are `srgb`, `linear-srgb`, `display-p3`,
`rec709`, `rec2020`, `prophoto`, `lab`, `xyz-d65`, `xyz-d50` and `oklab`. OKLab
is implemented directly (it is not an ICC space) and pivots through sRGB, so it
converts to and from any of the others too. The remaining enumerated spaces (the
ACES and Rec.2100/Rec.601/DCI-P3 variants) are recognized as tags and round-trip
in the container, but are refused as conversion endpoints with a clear
diagnostic rather than being approximated.

## Benchmarks against existing formats

`tools/benchmark.py` encodes the same pixels with `4splat` and with established
lossless formats, then reports the sizes side by side. It needs nothing beyond
the standard library: PNG, GIF and QOI are implemented in `tools/refcodecs.py`,
and gzip/bzip2/xz come from Python's own modules.

```bash
make bench          # quick subset, whichever schemes this build carries
make bench-full     # every clip, every scheme, plus a lossy --colors 64 run

python3 tools/benchmark.py --list                     # show the synthetic corpus
python3 tools/benchmark.py --corpus sprite,bounce     # pick clips
python3 tools/benchmark.py --input shots/ frame.ppm   # your own P6 files
python3 tools/benchmark.py --colors 64 --json bench.json --csv bench.csv
```

Contenders, all lossless unless marked:

| Entry | What it is |
| --- | --- |
| `raw RGB` | the uncompressed pixels - the denominator for "vs raw" |
| `gzip` / `bzip2` / `xz` | general-purpose compressors over the raw pixels |
| `PNG` | truecolor PNG with per-scanline adaptive filtering |
| `PNG8` | indexed PNG, when the clip has ≤256 colors |
| `GIF` | GIF89a with one global color table, animated for multi-frame clips |
| `QOI` | Quite OK Image v1.0, summed over frames |
| `4splat` | one row per compression scheme, plus `--colors N` runs scored by PSNR |

Every entry is decoded again and compared with the source, so a row only
reports a size for a file that provably decodes back to the original; a
`--colors N` row is scored in dB instead. The harness exits non-zero if
anything that claims to be lossless is not, which makes it a round-trip test
of the codec as well as a size comparison.

### Results

The sizes live in [Benchmarks](#benchmarks) at the top of this file, together
with the volumetric and 4D results, so all three can be read side by side.
They are recomputed from the corpus by `make bench-full`, not stored - re-run it
and you get the same numbers, on any machine that builds the same backends.

The harness and its reference codecs have their own test suite - round trips
through every PNG filter, GIF's LZW table overflow and each QOI opcode, plus an
end-to-end run against the built binary:

```bash
python3 -m pytest tests/test_benchmark.py
```

## Volumetric benchmark against 3D formats

The comparison above pits `.4spl` against 2D image formats. `tools/benchmark_volumes.py`
is the separate, volumetric one: a stack of z-slices encoded with
`encode-volume` - or a time-varying grid encoded with `encode-4d` - against the
containers built to hold a whole `(x, y, z[, t])` grid as a single object.

```bash
make bench-volumes                                    # every volume, every scheme

python3 tools/benchmark_volumes.py --list             # show the synthetic volumes
python3 tools/benchmark_volumes.py --corpus labels    # pick one
python3 tools/benchmark_volumes.py --input slices/    # your own stack of P6 slices
```

| Entry | What it is |
| --- | --- |
| `NIfTI` | NIfTI-1 single-file volume (`n+1`, `DT_RGB24`), plain and as `.nii.gz` |
| `NRRD` | NRRD with `gzip` encoding - the ITK / 3D Slicer format |
| `MetaImage` | `.mha` with `CompressedData = True` (zlib) |
| `TIFF stack` | multi-page TIFF: Deflate RGB pages, or palette pages sharing one `ColorMap` |
| `PNG` / `PNG8` / `QOI` | one file per slice - what you get without a volumetric container |
| `GIF` | the slices as animation frames, sharing a global color table |
| `gzip` / `bzip2` / `xz` | general-purpose compressors over the raw voxel grid |
| `4splat` | `encode-volume` or `encode-4d`, one row per scheme, plus lossy `--colors N` runs |

These are real files, not approximations: the reference encoders in
`tools/refvolumes.py` were cross-checked during development against the
canonical readers for each format - `nibabel`, `pynrrd`, ITK (SimpleITK) and
Pillow - and every size the harness reports was decoded back and compared voxel
for voxel before it was printed.

### Results

The volume and 4D-grid sizes are in [Benchmarks](#benchmarks) at the top of this
file. Two things about them are worth restating here, because they are what the
volumetric comparison is for:

* NIfTI, NRRD and MetaImage each have a time axis of their own, so for a 4D clip
  the harness hands them the depth and they describe the same `(x, y, z, t)`
  grid the `.4spl` header does - NIfTI's `dim` is literally that tuple. TIFF has
  no such axis; its pages stay a flat sequence, and its rows say so.
* The palette-color TIFF stack shares one `ColorMap` across every page, which is
  the closest a mainstream format comes to what this one does. It is included
  precisely so the comparison is not flattering.

```bash
python3 -m pytest tests/test_benchmark_volumes.py
```

## Test Suite

| Test | Description |
| --- | --- |
| `crc32_known_value` | Confirms the CRC32 implementation reproduces the standard checksum for the canonical "123456789" vector. |
| `checksum_matches_footer` | Ensures a freshly built video records the same checksum in its footer as produced by the compute helper. |
| `idxoffset_helpers_agree` | Checks that forward and reverse idxoffset helpers agree on the palette/index boundary. |
| `validate_succeeds_for_valid_video` | Verifies that a well-formed sample video passes validation. |
| `validate_fails_for_bad_checksum` | Expects validation to fail when the footer checksum is tampered with. |
| `validate_fails_for_null_video` | Asserts validation rejects a null video pointer. |
| `validate_fails_for_bad_magic` | Confirms validation catches an incorrect magic number. |
| `validate_fails_for_bad_version` | Confirms validation rejects files with an unsupported version field. |
| `validate_fails_for_zero_width` | Ensures validation fails if the header width is zero. |
| `validate_fails_for_zero_height` | Ensures validation fails when the header height is zero. |
| `validate_fails_for_zero_depth` | Ensures validation fails when the header depth is zero. |
| `validate_fails_for_zero_frames` | Ensures validation fails when the header frame count is zero. |
| `validate_fails_for_zero_palette_size` | Ensures validation fails when the palette size is zero. |
| `validate_fails_for_bad_footer_marker` | Confirms validation detects an incorrect footer terminator. |
| `validate_fails_for_big_endian_flag` | Verifies validation flags files marked as big-endian, which the codec does not support. |
| `validate_fails_for_unsupported_precision` | Ensures validation rejects headers requesting unsupported float precision. |
| `validate_detects_corrupted_index` | Confirms validation catches index data corruption without a matching checksum update. |
| `write_and_read_round_trip` | Exercises writing a video to disk and reading it back to guarantee structural fidelity. |
| `write_header_rejects_null_fp` | Checks that header writing fails when given a null file pointer. |
| `write_header_rejects_null_header` | Verifies header writing refuses to operate on a null header. |
| `read_header_rejects_null_fp` | Ensures header reading fails when the file pointer is null. |
| `read_header_rejects_null_header` | Ensures header reading fails when the output header pointer is null. |
| `write_palette_rejects_nulls` | Confirms palette writing validates its pointers and entry count. |
| `read_palette_rejects_invalid_inputs` | Ensures palette reading rejects null pointers or zero-length requests. |
| `read_palette_fails_on_short_file` | Verifies palette reading detects truncated palette data. |
| `write_index_rejects_nulls` | Confirms index writing validates its pointers and entry count. |
| `read_index_rejects_invalid_inputs` | Ensures index reading rejects null pointers or zero-length requests. |
| `read_index_fails_on_short_file` | Verifies index reading detects truncated index data. |
| `write_footer_rejects_nulls` | Confirms footer writing fails with null file or footer pointers. |
| `read_footer_rejects_nulls` | Ensures footer reading fails when provided null inputs. |
| `read_footer_fails_on_short_file` | Verifies footer reading detects truncated footer data. |
| `write_video_rejects_nulls` | Confirms video writing validates both the file pointer and video structure. |
| `read_video_rejects_nulls` | Ensures video reading refuses null file pointers or video outputs. |
| `read_video_fails_on_truncated_index` | Verifies video reading catches truncated index sections and cleans up allocations. |
| `read_video_fails_on_crc_mismatch` | Ensures video reading fails when the stored checksum does not match the payload. |
| `read_video_rejects_big_endian_flag` | Confirms video reading rejects files flagged as big-endian. |
| `read_video_fails_on_invalid_footer_marker` | Ensures video reading fails when the footer terminator is corrupted. |
| `idxoffset_sanity_mismatch` | Checks the idxoffset sanity helpers catch mismatched offsets between header and footer. |
| `grid4d_round_trip` | Builds a depth 2 x 3 frame grid and checks every slice comes back in t-major, z-minor order. |
| `grid4d_separates_depth_from_time` | Confirms a 4D grid's palette entries record the z and t of the slice each color came from. |
| `header_defaults_to_float32_precision` | Verifies header construction defaults the precision flag to float32 when unspecified. |

## Fuzzing

`tests/fuzz_read.c` feeds arbitrary bytes to `read_splat4DVideo` — the main
untrusted-input surface, since the reader sizes allocations from
attacker-controlled header dimensions. The reader rejects any header whose
declared palette/index sizes cannot fit the actual file (and caps a compressed
index's decompressed size), so malformed input fails cheaply instead of
attempting a huge allocation.

```bash
make fuzz && ./tests/fuzz_read tests/fuzz_corpus         # libFuzzer (clang + runtime)
make fuzz-standalone && ./tests/fuzz_read corpus/*       # portable runner, any sanitizer
```

CI runs the standalone harness under AddressSanitizer + UBSan over the seed
corpus plus random and header-mutated inputs on every push.

## Palette Explorer

An exploratory visualization tool lives in `tools/visualize_4splat.py`. It parses a
`.4spl` video, reconstructs frames using the stored palette, and renders an
interactive Matplotlib dashboard featuring:

* Reconstructed RGB frames for the active palette entries.
* Heat-map slices that show how palette indices are distributed across space.
* Scatter markers for each Gaussian "splat" centered at `(μx, μy)` with marker
  sizes proportional to how close their temporal mean `(μt)` is to the selected
  frame.
* Checkboxes to toggle the most-used palette entries on and off for quick
  reconstruction experiments.

Launch it with:

```bash
python tools/visualize_4splat.py path/to/video.4spl
```

Use the sliders at the bottom to step through time and depth slices, and the
checkboxes on the left to enable or disable palette slots.

## GitHub Pages Demo

This repository includes a static browser demo at `docs/index.html` and a
GitHub Actions workflow (`.github/workflows/deploy-pages.yml`) that deploys the
`docs/` folder to GitHub Pages on pushes to `main`.

After merging to `main`, enable **Settings → Pages → Build and deployment → GitHub Actions**
if not already enabled.
