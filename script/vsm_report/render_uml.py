#!/usr/bin/env python3
"""Render .uml (PlantUML) sources to light-theme PNGs, then derive dark-theme
siblings via ImageMagick color inversion.

Usage:
    python script/vsm_report/render_uml.py <file-or-dir> [<file-or-dir> ...]

For each `<name>.uml` input, this produces:
    <name>.png          -- plantuml's normal light-theme render
    <name>.dark.png      -- `magick <name>.png -negate <name>.dark.png`

Both `plantuml` and `magick` must be callable by those exact names on PATH.
This script does NOT silently skip rendering or fall back to a hardcoded
path if either tool is missing -- it fails loudly (non-zero exit, message
on stderr) so a missing toolchain is never mistaken for "nothing to render".
"""
from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path


class RenderError(RuntimeError):
    """Raised for any failure that should abort the whole run loudly."""


def require_tool(name: str) -> str:
    path = shutil.which(name)
    if path is None:
        raise RenderError(
            f"required tool '{name}' was not found on PATH. "
            f"This script refuses to silently skip diagram rendering or "
            f"fall back to a hardcoded path -- install/expose '{name}' as "
            f"'{name}' on PATH and re-run."
        )
    return path


def collect_uml_files(inputs: list[str]) -> list[Path]:
    files: list[Path] = []
    for raw in inputs:
        p = Path(raw)
        if not p.exists():
            raise RenderError(f"input path does not exist: {p}")
        if p.is_dir():
            found = sorted(p.glob("*.uml"))
            if not found:
                raise RenderError(f"no *.uml files found in directory: {p}")
            files.extend(found)
        elif p.is_file():
            if p.suffix.lower() != ".uml":
                raise RenderError(f"not a .uml file: {p}")
            files.append(p)
        else:
            raise RenderError(f"input path is neither a file nor a directory: {p}")
    if not files:
        raise RenderError("no .uml files to render")
    return files


def render_light(plantuml_path: str, uml_file: Path) -> Path:
    """Run plantuml on a single .uml file, producing '<stem>.png' next to it."""
    result = subprocess.run(
        [plantuml_path, str(uml_file)],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        raise RenderError(
            f"plantuml failed on {uml_file} (exit {result.returncode}):\n"
            f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}"
        )
    light_png = uml_file.with_suffix(".png")
    if not light_png.exists() or light_png.stat().st_size == 0:
        raise RenderError(
            f"plantuml reported success for {uml_file} but expected output "
            f"{light_png} is missing or empty"
        )
    return light_png


def render_dark(magick_path: str, light_png: Path) -> Path:
    """Derive a dark-theme sibling PNG via ImageMagick color inversion."""
    dark_png = light_png.with_name(light_png.stem + ".dark.png")
    result = subprocess.run(
        [magick_path, str(light_png), "-negate", str(dark_png)],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        raise RenderError(
            f"magick failed to negate {light_png} (exit {result.returncode}):\n"
            f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}"
        )
    if not dark_png.exists() or dark_png.stat().st_size == 0:
        raise RenderError(
            f"magick reported success for {light_png} but expected output "
            f"{dark_png} is missing or empty"
        )
    return dark_png


def render_all(inputs: list[str]) -> list[tuple[Path, Path, Path]]:
    """Render every .uml under `inputs`. Returns (uml, light_png, dark_png) triples."""
    plantuml_path = require_tool("plantuml")
    magick_path = require_tool("magick")

    uml_files = collect_uml_files(inputs)

    results: list[tuple[Path, Path, Path]] = []
    for uml_file in uml_files:
        light_png = render_light(plantuml_path, uml_file)
        dark_png = render_dark(magick_path, light_png)
        results.append((uml_file, light_png, dark_png))
    return results


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "inputs",
        nargs="+",
        help="One or more .uml files or directories containing .uml files.",
    )
    args = parser.parse_args(argv)

    try:
        results = render_all(args.inputs)
    except RenderError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    for uml_file, light_png, dark_png in results:
        print(f"{uml_file} -> {light_png} ({light_png.stat().st_size} bytes), "
              f"{dark_png} ({dark_png.stat().st_size} bytes)")
    print(f"rendered {len(results)} diagram(s), {len(results) * 2} PNG(s) total")
    return 0


if __name__ == "__main__":
    sys.exit(main())
