"""Render three orthographic portfolio views from a binary STL."""

import argparse
import struct
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw


def read_mesh(path):
    """Load binary STL triangles and centre the mesh at its bounds."""
    mesh = path.read_bytes()
    if len(mesh) < 84:
        raise ValueError("The file is too small to be a binary STL.")
    count = struct.unpack_from("<I", mesh, 80)[0]
    if not count or len(mesh) != 84 + count * 50:
        raise ValueError("The STL triangle count does not match the file length.")

    record = np.dtype([
        ("normal", "<f4", 3),
        ("vertices", "<f4", (3, 3)),
        ("attribute", "<u2"),
    ])
    triangles = np.frombuffer(mesh, dtype=record, offset=84)["vertices"].astype(float)
    centre = (triangles.max(axis=(0, 1)) + triangles.min(axis=(0, 1))) / 2
    return triangles - centre


def render_mesh(triangles, output, azimuth, elevation):
    az, el = np.radians([azimuth, elevation])
    right = np.array([-np.sin(az), np.cos(az), 0])
    up = np.array([-np.cos(az)*np.sin(el), -np.sin(az)*np.sin(el), np.cos(el)])
    view = np.cross(right, up)
    projected = triangles @ np.column_stack([right, up, view])
    screen = projected[:, :, :2] * [1, -1]
    edges = np.cross(projected[:,1]-projected[:,0], projected[:,2]-projected[:,0])
    shade = np.clip((edges @ np.array([-.32, .42, .85])) / np.maximum(np.linalg.norm(edges, axis=1), 1e-12), 0, 1)
    width, height = 1600, 1040
    scale = min((width-220)/np.ptp(screen[:,:,0]), (height-170)/np.ptp(screen[:,:,1]))
    points = ((screen - (screen.max(axis=(0,1)) + screen.min(axis=(0,1)))/2)*scale + [width/2, height/2]).astype(int)
    image = Image.new("RGB", (width, height), "#10151c")
    draw = ImageDraw.Draw(image)
    depth_order = np.argsort(projected[:,:,2].mean(axis=1))
    for index in depth_order:
        if edges[index, 2] <= 0:
            continue
        tone = shade[index]
        color = tuple(int(base + gain*tone) for base, gain in zip((49, 76, 110), (125, 136, 136)))
        draw.polygon([tuple(point) for point in points[index]], fill=color)
    image.resize((1200, 780), Image.Resampling.LANCZOS).save(output, optimize=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("stl", type=Path, help="Binary STL file")
    parser.add_argument("--output-dir", type=Path, default=Path("."))
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    triangles = read_mesh(args.stl)
    views = (("oblique.png", 52, 24), ("side.png", 90, 0), ("top.png", 90, 90))
    for filename, azimuth, elevation in views:
        render_mesh(triangles, args.output_dir / filename, azimuth, elevation)


if __name__ == "__main__":
    main()
