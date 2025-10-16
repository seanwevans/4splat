```
  ╭ .4spl File Format ╮
╭─╯                   ╰───────────────────────────────────────────────────╮
│ The 4Splat codec stores a palette of Gaussian “splats” plus an index    │
│ volume. 4Splat files always encode the volume in little-endian order,   │
│ laid out t-major → z-major → y-major → x-major.                         │
╰─────────────────────────────────────────────────────────────────────────╯

Binary layout
─────────────

Header (32 bytes)
  • magic      – ASCII "4SPL" (0x3453504C)
  • version    – uint8[4], currently {1, 0, 0, 0}
  • width      – uint32 width of the x dimension
  • height     – uint32 height of the y dimension
  • depth      – uint32 depth of the z dimension
  • frames     – uint32 number of frames (t dimension)
  • paletteSize– uint32 number of palette entries
  • flags      – uint32 feature flags (see below)

Palette (paletteSize entries)
Each palette entry is written as 12 little-endian float32 values matching
the `Splat4D` structure in the source:

  • mu_x, mu_y, mu_z, mu_t – mean position along each axis
  • sigma_x, sigma_y, sigma_z, sigma_t – Gaussian standard deviations
  • r, g, b, alpha – premultiplied colour and opacity

Index data
The voxel index volume immediately follows the palette. Indices are stored
as little-endian uint64_t values and there is one entry for every voxel in
the volume (`width * height * depth * frames`). The iteration order is
x-fastest within y, then z, then t.

Footer
The encoder appends a fixed-size footer containing integrity metadata:

  • checksum  – uint32 CRC-32 (polynomial 0xEDB88320) of the header,
                palette, and index payload
  • idxoffset – uint64 byte offset where the index array starts
                (equal to sizeof(header) + paletteSize * sizeof(Splat4D))
  • end       – ASCII "LPS4" (0x4C505334) sentinel

Supported flags
───────────────

`4splat` currently recognises only the endianness and precision bits of the
flags word. Files must be little-endian and use float32 splat attributes.
Any other flag bits – including compression, colour spaces, interpolation,
or wider index widths – cause decoding and validation to fail. The encoder
sanitises inputs by forcing little-endian mode and upgrading float16
requests to float32.

Command line interface
──────────────────────

```
4splat encode --palette palette.bin --index index.bin --output video.4spl \
              --width W --height H --depth D --frames F [--palette-size N] [--flags N]
4splat decode --input video.4spl [--palette palette.bin] [--index index.bin] [--print] [--validate]
```

• The encoder expects `palette.bin` to contain tightly packed `Splat4D`
  structs and `index.bin` to contain uint64_t indices.
• If `--palette-size` is omitted it is inferred from the palette file.
• The checksum in the footer is recomputed automatically when encoding.
• `--validate` recomputes the checksum and verifies the header and footer
  invariants while decoding.
```
