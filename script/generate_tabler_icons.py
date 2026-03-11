#!/usr/bin/env python3
import argparse
import io
import os
import shutil
import subprocess
import sys
import urllib.request
import zipfile
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from typing import Iterable

from PIL import Image


DEFAULT_VERSION = "3.40.0"
DEFAULT_TMP_DIR = Path("tmp")
DEFAULT_OUT_ROOT = Path("share/icons")
THEME_COLORS = {
    "light": "#000000",
    "dark": "#ffffff",
}
DEFAULT_STYLES = ("outline", "filled")
DEFAULT_SIZES = (24, 48)


def parse_csv(value: str) -> list[str]:
    return [x.strip() for x in value.split(",") if x.strip()]


def parse_sizes(value: str) -> list[int]:
    out: list[int] = []
    for item in parse_csv(value):
        size = int(item)
        if size <= 0:
            raise ValueError(f"invalid size: {size}")
        out.append(size)
    if not out:
        raise ValueError("sizes list is empty")
    return out


def run(cmd: list[str], *, capture_output: bool = False) -> subprocess.CompletedProcess[bytes]:
    return subprocess.run(cmd, check=True, capture_output=capture_output)


def ensure_rsvg_convert() -> None:
    if shutil.which("rsvg-convert") is None:
        raise RuntimeError("rsvg-convert not found in PATH")


def ensure_zip(tmp_dir: Path, version: str) -> Path:
    zip_name = f"tabler-icons-v{version}.zip"
    zip_path = tmp_dir / zip_name
    if zip_path.exists():
        return zip_path

    tmp_dir.mkdir(parents=True, exist_ok=True)
    url = f"https://github.com/tabler/tabler-icons/archive/refs/tags/v{version}.zip"
    print(f"downloading: {url}")
    urllib.request.urlretrieve(url, zip_path)
    return zip_path


def ensure_extracted(tmp_dir: Path, version: str, zip_path: Path) -> Path:
    extract_root = tmp_dir / f"tabler-icons-v{version}"
    repo_root = extract_root / f"tabler-icons-{version}"
    icons_root = repo_root / "icons"
    if icons_root.exists():
        return repo_root

    extract_root.mkdir(parents=True, exist_ok=True)
    print(f"extracting: {zip_path} -> {extract_root}")
    with zipfile.ZipFile(zip_path, "r") as zf:
        zf.extractall(extract_root)

    if not icons_root.exists():
        raise RuntimeError(f"expected icons directory not found: {icons_root}")
    return repo_root


def collect_svg_files(repo_root: Path, styles: Iterable[str]) -> list[tuple[str, Path]]:
    files: list[tuple[str, Path]] = []
    for style in styles:
        style_dir = repo_root / "icons" / style
        if not style_dir.is_dir():
            raise RuntimeError(f"missing style directory: {style_dir}")
        style_svgs = sorted(style_dir.glob("*.svg"))
        if not style_svgs:
            raise RuntimeError(f"no SVG files in: {style_dir}")
        files.extend((style, p) for p in style_svgs)
    return files


def optimize_png_bytes(png_bytes: bytes) -> bytes:
    with Image.open(io.BytesIO(png_bytes)) as im:
        im = im.convert("RGBA")
        out = io.BytesIO()
        im.save(out, format="PNG", optimize=True, compress_level=9)
        return out.getvalue()


def render_png(svg_path: Path, size: int, css_path: Path) -> bytes:
    cmd = [
        "rsvg-convert",
        "--format",
        "png",
        "--keep-aspect-ratio",
        "--background-color",
        "none",
        "--width",
        str(size),
        "--height",
        str(size),
        "--stylesheet",
        str(css_path),
        str(svg_path),
    ]
    proc = run(cmd, capture_output=True)
    return optimize_png_bytes(proc.stdout)


def ensure_license(repo_root: Path, out_root: Path) -> None:
    src = repo_root / "LICENSE"
    if not src.exists():
        raise RuntimeError(f"missing license file: {src}")

    for theme in THEME_COLORS:
        dst = out_root / theme / "tabler" / "LICENSE"
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)


def build_tasks(
    files: list[tuple[str, Path]],
    out_root: Path,
    sizes: list[int],
    limit: int,
    force: bool,
) -> list[tuple[str, str, Path, int, Path]]:
    tasks: list[tuple[str, str, Path, int, Path]] = []
    for style, svg_path in files:
        stem = svg_path.stem
        for size in sizes:
            for theme in THEME_COLORS:
                out_file = out_root / theme / "tabler" / style / f"{stem}_{size}.png"
                if not force and out_file.exists():
                    continue
                tasks.append((theme, style, svg_path, size, out_file))
    if limit > 0:
        return tasks[:limit]
    return tasks


def write_css_files(out_root: Path) -> dict[str, Path]:
    css_dir = out_root / ".tmp_tabler_css"
    css_dir.mkdir(parents=True, exist_ok=True)

    css_paths: dict[str, Path] = {}
    for theme, color in THEME_COLORS.items():
        css_path = css_dir / f"{theme}.css"
        css_path.write_text(f":root {{ color: {color}; }}\n", encoding="utf-8")
        css_paths[theme] = css_path
    return css_paths


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--version", default=DEFAULT_VERSION)
    parser.add_argument("--tmp-dir", default=str(DEFAULT_TMP_DIR))
    parser.add_argument("--out-root", default=str(DEFAULT_OUT_ROOT))
    parser.add_argument("--styles", default=",".join(DEFAULT_STYLES))
    parser.add_argument("--sizes", default=",".join(str(s) for s in DEFAULT_SIZES))
    parser.add_argument("--threads", type=int, default=0)
    parser.add_argument("--limit", type=int, default=0)
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()

    ensure_rsvg_convert()

    tmp_dir = Path(args.tmp_dir)
    out_root = Path(args.out_root)
    styles = parse_csv(args.styles)
    sizes = parse_sizes(args.sizes)
    if not styles:
        raise RuntimeError("styles list is empty")

    zip_path = ensure_zip(tmp_dir, args.version)
    repo_root = ensure_extracted(tmp_dir, args.version, zip_path)

    files = collect_svg_files(repo_root, styles)
    ensure_license(repo_root, out_root)
    css_paths = write_css_files(out_root)

    tasks = build_tasks(files, out_root, sizes, args.limit, args.force)
    total = len(tasks)

    print(f"tabler version: {args.version}")
    print(f"source repo: {repo_root}")
    print(f"styles: {', '.join(styles)}")
    print(f"icons found: {len(files)}")
    print(f"sizes: {', '.join(str(s) for s in sizes)}")
    print(f"themes: {', '.join(THEME_COLORS.keys())}")
    print(f"files to generate: {total}")

    if total == 0:
        shutil.rmtree(out_root / ".tmp_tabler_css", ignore_errors=True)
        return 0

    workers = args.threads if args.threads > 0 else min(32, (os.cpu_count() or 4))

    def job(task: tuple[str, str, Path, int, Path]) -> None:
        theme, _style, svg_path, size, out_file = task
        png_bytes = render_png(svg_path, size, css_paths[theme])
        out_file.parent.mkdir(parents=True, exist_ok=True)
        out_file.write_bytes(png_bytes)

    errors = 0
    done = 0
    with ThreadPoolExecutor(max_workers=workers) as exe:
        futures = [exe.submit(job, task) for task in tasks]
        for fut in as_completed(futures):
            try:
                fut.result()
            except Exception as e:
                errors += 1
                print(f"error: {e}", file=sys.stderr)
            done += 1
            if done % 200 == 0 or done == total:
                print(f"progress: {done}/{total}")

    shutil.rmtree(out_root / ".tmp_tabler_css", ignore_errors=True)

    if errors:
        print(f"completed with errors: {errors}", file=sys.stderr)
        return 1

    print("done")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
