<img width="256" alt="splat!" src="https://github.com/user-attachments/assets/63774bbb-7322-4296-b1f4-9be2b8741221" />


```
  ╭ .4spl File Format ╮
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
 ╰───────────────────────────────────────────────────────────────────────╯
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
| `header_defaults_to_float32_precision` | Verifies header construction defaults the precision flag to float32 when unspecified. |

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
