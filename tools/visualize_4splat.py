"""Interactive visualization for 4Splat codec data.

This module provides a small exploratory tool that parses a .4spl video and
builds an interactive matplotlib dashboard.  The viewer shows:

* The reconstructed RGB frame (respecting palette toggles).
* A categorical heat-map of palette assignments for the current frame/depth.
* A scatter plot of the Gaussian "splat" centers (μx, μy) whose marker size
  responds to how close their temporal mean μt is to the selected frame.
* Checkboxes that allow enabling/disabling the most used palette entries to
  observe how they contribute to the reconstruction.

Example usage::

    python tools/visualize_4splat.py path/to/video.4spl

The tool requires NumPy and Matplotlib.
"""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Sequence, Tuple

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.colors import ListedColormap
from matplotlib.widgets import CheckButtons, Slider


HEADER_STRUCT = struct.Struct("<4s4B6I")
PALETTE_ENTRY_FLOATS = 12
PALETTE_ENTRY_SIZE = PALETTE_ENTRY_FLOATS * 4
FOOTER_STRUCT = struct.Struct("<QI4s")
INDEX_WIDTH_LOOKUP = {0: 1, 1: 2, 2: 4, 3: 8}


@dataclass(frozen=True)
class FourSplatHeader:
    """Represents the 32-byte 4Splat header."""

    magic: bytes
    version: Tuple[int, int, int, int]
    width: int
    height: int
    depth: int
    frames: int
    palette_size: int
    flags: int

    @property
    def total_voxels(self) -> int:
        return self.width * self.height * self.depth * self.frames

    @property
    def index_width_code(self) -> int:
        return (self.flags >> 8) & 0b11

    @property
    def index_bytes(self) -> int:
        code = self.index_width_code
        if code not in INDEX_WIDTH_LOOKUP:
            raise ValueError(f"Unsupported index width code: {code}")
        return INDEX_WIDTH_LOOKUP[code]


@dataclass
class PaletteEntry:
    """Stores the Gaussian + color attributes for a single palette slot."""

    mu_x: float
    mu_y: float
    mu_z: float
    sigma_x: float
    sigma_y: float
    sigma_z: float
    mu_t: float
    sigma_t: float
    r: float
    g: float
    b: float
    alpha: float

    def rgb(self) -> np.ndarray:
        return np.asarray([self.r, self.g, self.b], dtype=np.float32)

    def rgba(self) -> np.ndarray:
        return np.asarray([self.r, self.g, self.b, self.alpha], dtype=np.float32)

    def spatial_mu(self) -> Tuple[float, float]:
        return self.mu_x, self.mu_y


@dataclass
class FourSplatVideo:
    header: FourSplatHeader
    palette: List[PaletteEntry]
    indices: np.ndarray  # shape (frames, depth, height, width)
    idxoffset: int
    checksum: int

    @property
    def palette_rgba(self) -> np.ndarray:
        return np.stack([entry.rgba() for entry in self.palette], axis=0)

    @property
    def palette_rgb(self) -> np.ndarray:
        return self.palette_rgba[:, :3]

    @property
    def palette_alpha(self) -> np.ndarray:
        return self.palette_rgba[:, 3]

    def palette_means(self) -> np.ndarray:
        return np.array([entry.spatial_mu() for entry in self.palette], dtype=np.float32)

    def palette_temporal(self) -> np.ndarray:
        return np.array([[entry.mu_t, entry.sigma_t] for entry in self.palette], dtype=np.float32)


class FourSplatParser:
    """Parses .4spl files according to the README specification."""

    def __init__(self, path: Path):
        self.path = Path(path)

    def parse(self) -> FourSplatVideo:
        data = self.path.read_bytes()
        if len(data) < HEADER_STRUCT.size + FOOTER_STRUCT.size:
            raise ValueError("File too small to be a valid 4Splat container")

        header = self._parse_header(data)
        palette, palette_bytes = self._parse_palette(data, HEADER_STRUCT.size, header.palette_size)
        indices, idx_bytes = self._parse_indices(
            data,
            HEADER_STRUCT.size + palette_bytes,
            header.total_voxels,
            header.index_bytes,
        )
        footer = self._parse_footer(data, HEADER_STRUCT.size + palette_bytes + idx_bytes)

        frames = header.frames
        depth = header.depth
        height = header.height
        width = header.width
        reshaped = np.reshape(indices, (frames, depth, height, width))

        return FourSplatVideo(header, palette, reshaped, footer[0], footer[1])

    def _parse_header(self, data: bytes) -> FourSplatHeader:
        header_data = data[: HEADER_STRUCT.size]
        unpacked = HEADER_STRUCT.unpack(header_data)
        magic = unpacked[0]
        version = unpacked[1:5]
        width, height, depth, frames, palette_size, flags = unpacked[5:11]

        if magic != b"4SPL":
            raise ValueError(f"Unexpected magic: {magic!r}")
        if version[0] != 1:
            raise ValueError(f"Unsupported major version: {version[0]}")

        return FourSplatHeader(magic, version, width, height, depth, frames, palette_size, flags)

    def _parse_palette(
        self, data: bytes, offset: int, palette_size: int
    ) -> Tuple[List[PaletteEntry], int]:
        entries: List[PaletteEntry] = []
        total_size = palette_size * PALETTE_ENTRY_SIZE
        end = offset + total_size
        if len(data) < end:
            raise ValueError("Palette data truncated")

        palette_buffer = memoryview(data)[offset:end]
        floats = np.frombuffer(palette_buffer, dtype="<f4", count=palette_size * PALETTE_ENTRY_FLOATS)
        floats = floats.reshape((palette_size, PALETTE_ENTRY_FLOATS))

        for row in floats:
            entries.append(
                PaletteEntry(
                    mu_x=float(row[0]),
                    mu_y=float(row[1]),
                    mu_z=float(row[2]),
                    sigma_x=float(row[3]),
                    sigma_y=float(row[4]),
                    sigma_z=float(row[5]),
                    mu_t=float(row[6]),
                    sigma_t=float(row[7]),
                    r=float(row[8]),
                    g=float(row[9]),
                    b=float(row[10]),
                    alpha=float(row[11]),
                )
            )

        return entries, total_size

    def _parse_indices(
        self, data: bytes, offset: int, voxel_count: int, index_bytes: int
    ) -> Tuple[np.ndarray, int]:
        total_bytes = voxel_count * index_bytes
        end = offset + total_bytes
        if len(data) < end:
            raise ValueError("Index data truncated")

        buffer = memoryview(data)[offset:end]
        dtype = {1: np.uint8, 2: np.uint16, 4: np.uint32, 8: np.uint64}[index_bytes]
        indices = np.frombuffer(buffer, dtype=dtype, count=voxel_count)
        return indices, total_bytes

    def _parse_footer(self, data: bytes, offset: int) -> Tuple[int, int]:
        if len(data) < offset + FOOTER_STRUCT.size:
            raise ValueError("Footer truncated")
        idxoffset, checksum, end_ascii = FOOTER_STRUCT.unpack_from(data, offset)
        if end_ascii != b"LPS4":
            raise ValueError(f"Unexpected footer terminator: {end_ascii!r}")
        return idxoffset, checksum


class PaletteViewer:
    """Matplotlib-based exploratory viewer for 4Splat palettes."""

    def __init__(self, video: FourSplatVideo, max_entries: int = 10):
        self.video = video
        self.max_entries = max_entries
        self.fig = None
        self.ax_frame = None
        self.ax_heat = None
        self.frame_image = None
        self.heat_image = None
        self.scatter = None
        self.frame_slider = None
        self.depth_slider = None
        self.check_buttons = None
        self.active_entries: Dict[int, bool] = {}
        self._init_active_entries()

    def _init_active_entries(self) -> None:
        indices = self.video.indices
        flat = indices.ravel()
        counts = np.bincount(flat.astype(np.int64), minlength=len(self.video.palette))
        ordered = np.argsort(counts)[::-1]
        limited = ordered[: min(self.max_entries, len(ordered))]
        self.active_entries = {int(idx): True for idx in limited}

    def show(self) -> None:
        plt.close("all")
        self.fig, (self.ax_frame, self.ax_heat) = plt.subplots(1, 2, figsize=(13, 6))
        plt.subplots_adjust(left=0.32, bottom=0.22, right=0.97)

        frame0 = 0
        depth0 = 0
        rgb_slice = self._reconstruct_slice(frame0, depth0)
        heat_slice = self.video.indices[frame0, depth0]

        self.ax_frame.set_title("Reconstructed Frame")
        self.frame_image = self.ax_frame.imshow(rgb_slice, origin="lower")
        self.ax_frame.set_axis_off()

        self.ax_heat.set_title("Palette Assignment Heat-map")
        cmap = self._build_palette_colormap()
        self.heat_image = self.ax_heat.imshow(heat_slice, origin="lower", cmap=cmap, vmin=0, vmax=len(self.video.palette) - 1)
        self.scatter = self.ax_heat.scatter([], [], s=0, c="white", edgecolor="black", linewidths=0.5, alpha=0.85)
        self.ax_heat.set_xlim(-0.5, self.video.header.width - 0.5)
        self.ax_heat.set_ylim(-0.5, self.video.header.height - 0.5)
        self.ax_heat.invert_yaxis()

        self._init_sliders()
        self._init_checkboxes()
        self._update_scatter(frame0)

        self.ax_heat.set_xlabel("x (columns)")
        self.ax_heat.set_ylabel("y (rows)")
        self.ax_frame.set_xlabel("x (columns)")
        self.ax_frame.set_ylabel("y (rows)")

        self.fig.suptitle("4Splat Palette Explorer", fontsize=16, fontweight="bold")
        plt.show()

    def _build_palette_colormap(self) -> ListedColormap:
        palette_rgb = self.video.palette_rgb
        if np.max(palette_rgb) > 1.0:
            palette_rgb = np.clip(palette_rgb / 255.0, 0, 1)
        colors = np.vstack([palette_rgb, np.array([[0.0, 0.0, 0.0]])])
        return ListedColormap(colors)

    def _init_sliders(self) -> None:
        axcolor = "lightgoldenrodyellow"
        ax_frame_slider = plt.axes([0.32, 0.1, 0.6, 0.03], facecolor=axcolor)
        ax_depth_slider = plt.axes([0.32, 0.05, 0.6, 0.03], facecolor=axcolor)

        self.frame_slider = Slider(ax_frame_slider, "Frame", 0, self.video.header.frames - 1, valinit=0, valstep=1)
        self.depth_slider = Slider(ax_depth_slider, "Depth", 0, self.video.header.depth - 1, valinit=0, valstep=1)

        self.frame_slider.on_changed(lambda val: self._update_display())
        self.depth_slider.on_changed(lambda val: self._update_display())

    def _init_checkboxes(self) -> None:
        palette_count = len(self.video.palette)
        if palette_count == 0:
            return

        bbox_height = 0.55
        rax = plt.axes([0.05, 0.35, 0.22, bbox_height], facecolor="whitesmoke")
        labels = []
        states: List[bool] = []
        counts = np.bincount(self.video.indices.ravel().astype(np.int64), minlength=palette_count)
        total = counts.sum() if counts.sum() > 0 else 1

        for entry_id, active in self.active_entries.items():
            usage = counts[entry_id] / total
            label = f"{entry_id:3d} | {usage:5.1%}"
            labels.append(label)
            states.append(active)

        self.check_buttons = CheckButtons(rax, labels, states)

        def on_clicked(label: str) -> None:
            entry_id = int(label.split("|")[0])
            self.active_entries[entry_id] = not self.active_entries[entry_id]
            self._update_display()

        self.check_buttons.on_clicked(on_clicked)

    def _active_palette_mask(self) -> np.ndarray:
        palette_count = len(self.video.palette)
        mask = np.ones(palette_count, dtype=bool)
        if not self.active_entries:
            return mask
        for entry_id, active in self.active_entries.items():
            mask[entry_id] = active
        return mask

    def _reconstruct_slice(self, frame_idx: int, depth_idx: int) -> np.ndarray:
        indices = self.video.indices[frame_idx, depth_idx]
        palette_rgb = self.video.palette_rgb
        alpha = self.video.palette_alpha[:, None]
        mask = self._active_palette_mask()
        rgb = palette_rgb.copy()
        inactive = ~mask
        if np.any(inactive):
            rgb[inactive] = 0.0
        reconstructed = rgb[indices]
        alphas = alpha[indices]
        return np.clip(reconstructed * alphas, 0.0, 1.0)

    def _update_display(self) -> None:
        frame_idx = int(self.frame_slider.val)
        depth_idx = int(self.depth_slider.val)
        rgb_slice = self._reconstruct_slice(frame_idx, depth_idx)
        heat_slice = self.video.indices[frame_idx, depth_idx]

        self.frame_image.set_data(rgb_slice)
        self.heat_image.set_data(heat_slice)
        self._update_scatter(frame_idx)
        self.fig.canvas.draw_idle()

    def _update_scatter(self, frame_idx: int) -> None:
        means = self.video.palette_means()
        temporal = self.video.palette_temporal()
        if means.size == 0:
            self.scatter.set_offsets([])
            self.scatter.set_sizes([])
            return

        mu_t = temporal[:, 0]
        sigma_t = np.clip(temporal[:, 1], a_min=1e-6, a_max=None)
        frame_positions = np.full_like(mu_t, frame_idx, dtype=np.float32)
        weights = np.exp(-0.5 * ((frame_positions - mu_t) / sigma_t) ** 2)
        weights = np.nan_to_num(weights, nan=0.0, posinf=0.0, neginf=0.0)
        sizes = 100.0 * weights + 10.0

        self.scatter.set_offsets(means)
        self.scatter.set_sizes(sizes)
        self.scatter.set_array(weights)
        self.scatter.set_cmap("viridis")
        self.scatter.set_clim(vmin=0.0, vmax=weights.max() if weights.size else 1.0)


def load_video(path: Path) -> FourSplatVideo:
    return FourSplatParser(path).parse()


def main(argv: Sequence[str] | None = None) -> None:
    parser = argparse.ArgumentParser(description="Explore 4Splat palettes visually")
    parser.add_argument("video", type=Path, help="Path to the .4spl video file")
    parser.add_argument(
        "--max-entries",
        type=int,
        default=10,
        help="Maximum number of palette entries to expose as toggles (default: 10)",
    )
    args = parser.parse_args(argv)

    video = load_video(args.video)
    viewer = PaletteViewer(video, max_entries=args.max_entries)
    viewer.show()


if __name__ == "__main__":
    main()
