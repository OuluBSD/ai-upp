#!/usr/bin/env python3
import argparse
import base64
import os
from pathlib import Path
import xml.etree.ElementTree as ET

from openai import OpenAI
from PIL import Image


REQUIRED_SEMANTICS = [
    "generic_new_file",
    "generic_open_file",
    "generic_save_file",
    "generic_undo",
    "generic_redo",
    "mouse_cursor",
    "four_arrows_cross",
    "clockwise_arc_arrow",
    "3d_scale_arrows",
    "frame_plus_3d_perspective_block",
    "frame_plus_top_projection_flat_wide_block",
    "frame_plus_front_square_projection",
    "frame_plus_left_tall_narrow_projection",
    "gray_cube_with_black_wireframe",
    "gray_rough_sphere_with_black_wireframe",
    "gray_rough_cylinder_with_black_wireframe",
    "gray_rough_cone_with_black_wireframe",
    "gray_square_plane_in_perspective_with_black_wireframe",
    "classic_film_camera_icon_gray_black",
    "side_profile_terrain_curve_with_filled_ground_polygon",
    "coarse_3d_corridor_shape",
    "3d_cube_like_import_symbol",
    "3d_cube_like_import_symbol_animated",
    "yellow_glowing_lightbulb",
    "sun_icon_with_arrow_left_down",
    "green_2d_fir_tree",
    "blue_plane_in_perspective",
    "gray_background_with_light_B",
    "narrower_taller_variant_of_billboard_icon",
    "small_gray_particle_with_yellow_arrows_up_left_and_up_right",
    "inside_cube_view_blue_sky_walls_gray_ground",
    "speaker_cone",
    "gray_background_with_dark_2D_text",
    "2d_overlay_symbol_with_circular_background",
    "two_points_connected_by_line",
    "path_like_symbol_with_additional_points",
    "triangle_over_axis_background_with_mouse_cursor",
    "small_cursor_with_rectangle_outline",
    "triangle_over_axis_background_plus_four_direction_arrows",
    "triangle_over_axis_background_plus_arc_rotate_arrow",
    "triangle_over_axis_background_plus_perspective_scale_arrows",
    "gray_background_with_black_UV_text",
    "gear_cog",
    "play_icon",
    "plus_icon",
    "minus_icon",
    "scene_metrics_glyph",
    "scene_postfx_glyph",
]


def load_api_key() -> str:
    key_path = Path.home() / "openai-key.txt"
    if "OPENAI_API_KEY" in os.environ:
        return os.environ["OPENAI_API_KEY"]
    if key_path.exists():
        return key_path.read_text().strip()
    raise RuntimeError("OPENAI_API_KEY not set and ~/openai-key.txt not found")


def collect_icon_semantics(xml_path: Path) -> list[str]:
    root = ET.fromstring(xml_path.read_text())
    ribbon = root.find("ribbon")
    if ribbon is None:
        return []
    sems = []
    for btn in ribbon.iter("button"):
        sem = btn.get("icon_semantics") or btn.get("icon") or ""
        if sem:
            sems.append(sem)
    # preserve order, remove duplicates
    seen = set()
    ordered = []
    for s in sems:
        if s not in seen:
            seen.add(s)
            ordered.append(s)
    return ordered


def prompt_for(sem: str) -> str:
    words = sem.replace("_", " ").replace("-", " ")
    return (
        "Minimal flat UI icon, "
        f"{words}. "
        "Simple geometry, crisp edges, no text, centered. "
        "Single-color or subtle two-tone. Transparent background."
    )


def save_resized(src: Path, dst: Path, size: int) -> None:
    with Image.open(src) as im:
        im = im.convert("RGBA")
        im = im.resize((size, size), Image.LANCZOS)
        dst.parent.mkdir(parents=True, exist_ok=True)
        im.save(dst, format="PNG")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--xml", default="share/scene3d/ribbon/coppercube_ribbon.xml")
    parser.add_argument("--out", default="share/icons")
    parser.add_argument("--large-dir", default="share/icons/large")
    parser.add_argument("--limit", type=int, default=0)
    parser.add_argument("--force", action="store_true")
    parser.add_argument("--size", default="1024x1024")
    args = parser.parse_args()

    api_key = load_api_key()
    client = OpenAI(api_key=api_key)

    xml_path = Path(args.xml)
    out_dir = Path(args.out)
    large_dir = Path(args.large_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    large_dir.mkdir(parents=True, exist_ok=True)

    sems = collect_icon_semantics(xml_path)
    known = set(REQUIRED_SEMANTICS)
    unknown = [s for s in sems if s not in known]
    if unknown:
        raise RuntimeError(f"Unknown ribbon icon semantics: {unknown}")
    # ensure required semantics are generated even if XML omits them
    for sem in REQUIRED_SEMANTICS:
        if sem not in sems:
            sems.append(sem)
    if not args.force:
        missing = []
        for sem in sems:
            large_path = large_dir / f"{sem}.png"
            p24 = out_dir / f"{sem}_24.png"
            p48 = out_dir / f"{sem}_48.png"
            if large_path.exists() and p24.exists() and p48.exists():
                continue
            missing.append(sem)
        sems = missing
    if args.limit and args.limit > 0:
        sems = sems[: args.limit]

    for sem in sems:
        out_path = large_dir / f"{sem}.png"
        prompt = prompt_for(sem)
        result = client.images.generate(
            model="gpt-image-1-mini",
            prompt=prompt,
            size=args.size,
            background="transparent",
        )
        image_base64 = result.data[0].b64_json
        image_bytes = base64.b64decode(image_base64)
        out_path.write_bytes(image_bytes)
        save_resized(out_path, out_dir / f"{sem}_24.png", 24)
        save_resized(out_path, out_dir / f"{sem}_48.png", 48)
        print(f"wrote {out_path}")
        print(f"wrote {out_dir / (sem + '_24.png')}")
        print(f"wrote {out_dir / (sem + '_48.png')}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
