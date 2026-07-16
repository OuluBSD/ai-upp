#!/usr/bin/env python3
"""Build a single self-contained dark-themed HTML report from a JSON manifest.

Usage:
    python script/vsm_report/render_report.py <manifest.json> <output.html>

Manifest format (JSON object):
{
  "title": "Task NNNN: Some Title",
  "sections": [
    {"type": "prose",   "heading": "Summary", "paragraphs": ["text...", "more text..."]},
    {"type": "diagram", "heading": "Level 0: Overview", "image": "uml/level0.dark.png",
     "caption": "optional caption text"},
    {"type": "pre",     "heading": "Command Output", "text": "raw stdout/stderr..."},
    {"type": "table",   "heading": "Divergences", "columns": ["Frame", "Field"],
     "rows": [["1", "board_cards"], ["2", "pot"]]}
  ]
}

Design notes:
- Every text field that could originate from data/logs/OCR/model output is
  HTML-escaped using the same rule as docs/HTML_REPORT.md: `<`, `>`, and `&`
  are converted to `&lt;`, `&gt;`, `&amp;`.
- `image` paths are resolved relative to the manifest file's directory (or
  used as-is if absolute), read as bytes, and inlined as base64 `data:` URIs
  so the resulting `index.html` is a single self-contained file with no
  sibling-file dependency and no external CDN/JS.
- This module is intentionally generic (section-list + type dispatch) so it
  can be reused for future VSM task reports, not just Task 0258.
"""
from __future__ import annotations

import argparse
import base64
import json
import sys
from pathlib import Path
from typing import Any


def html_escape(text: Any) -> str:
    """Escape text for safe HTML embedding: &, <, > only (HTML_REPORT.md rule)."""
    s = str(text)
    s = s.replace("&", "&amp;")
    s = s.replace("<", "&lt;")
    s = s.replace(">", "&gt;")
    return s


def load_manifest(manifest_path: Path) -> dict:
    with manifest_path.open("r", encoding="utf-8") as f:
        return json.load(f)


def image_to_data_uri(image_path: Path) -> str:
    if not image_path.exists():
        raise FileNotFoundError(f"diagram image does not exist: {image_path}")
    data = image_path.read_bytes()
    if not data:
        raise ValueError(f"diagram image is empty: {image_path}")
    encoded = base64.b64encode(data).decode("ascii")
    suffix = image_path.suffix.lower()
    mime = {
        ".png": "image/png",
        ".jpg": "image/jpeg",
        ".jpeg": "image/jpeg",
        ".svg": "image/svg+xml",
        ".gif": "image/gif",
    }.get(suffix, "application/octet-stream")
    return f"data:{mime};base64,{encoded}"


def render_prose(section: dict) -> str:
    parts = []
    heading = section.get("heading")
    if heading:
        parts.append(f"<h2>{html_escape(heading)}</h2>")
    for para in section.get("paragraphs", []):
        parts.append(f"<p>{html_escape(para)}</p>")
    return "\n".join(parts)


def render_diagram(section: dict, manifest_dir: Path) -> str:
    parts = []
    heading = section.get("heading")
    if heading:
        parts.append(f"<h2>{html_escape(heading)}</h2>")
    image_field = section.get("image")
    if not image_field:
        raise ValueError(f"diagram section missing 'image': {section}")
    image_path = Path(image_field)
    if not image_path.is_absolute():
        image_path = manifest_dir / image_path
    data_uri = image_to_data_uri(image_path)
    alt = html_escape(section.get("caption") or heading or image_path.name)
    parts.append(f'<img src="{data_uri}" alt="{alt}" class="diagram">')
    caption = section.get("caption")
    if caption:
        parts.append(f"<p class=\"caption\">{html_escape(caption)}</p>")
    return "\n".join(parts)


def render_pre(section: dict) -> str:
    parts = []
    heading = section.get("heading")
    if heading:
        parts.append(f"<h2>{html_escape(heading)}</h2>")
    text = section.get("text", "")
    parts.append(f"<pre>{html_escape(text)}</pre>")
    return "\n".join(parts)


def render_table(section: dict) -> str:
    parts = []
    heading = section.get("heading")
    if heading:
        parts.append(f"<h2>{html_escape(heading)}</h2>")
    columns = section.get("columns", [])
    rows = section.get("rows", [])
    parts.append("<table>")
    if columns:
        parts.append("<thead><tr>" + "".join(f"<th>{html_escape(c)}</th>" for c in columns) + "</tr></thead>")
    parts.append("<tbody>")
    for row in rows:
        parts.append("<tr>" + "".join(f"<td>{html_escape(cell)}</td>" for cell in row) + "</tr>")
    parts.append("</tbody>")
    parts.append("</table>")
    return "\n".join(parts)


def render_heading(section: dict) -> str:
    text = section.get("text", "")
    return f"<h2>{html_escape(text)}</h2>"


SECTION_RENDERERS = {
    "prose": lambda section, manifest_dir: render_prose(section),
    "diagram": lambda section, manifest_dir: render_diagram(section, manifest_dir),
    "pre": lambda section, manifest_dir: render_pre(section),
    "table": lambda section, manifest_dir: render_table(section),
    "heading": lambda section, manifest_dir: render_heading(section),
}

STYLE = """
body {
  background: #12141a;
  color: #e6e6e6;
  font-family: -apple-system, Segoe UI, Helvetica, Arial, sans-serif;
  line-height: 1.5;
  max-width: 960px;
  margin: 2rem auto;
  padding: 0 1.5rem;
}
h1 { color: #ffffff; border-bottom: 1px solid #3a3f4b; padding-bottom: 0.5rem; }
h2 { color: #d7d9de; margin-top: 2.5rem; }
p { color: #c7cad1; }
img.diagram {
  max-width: 100%;
  height: auto;
  background: #12141a;
  border: 1px solid #3a3f4b;
  border-radius: 4px;
  display: block;
  margin: 1rem 0;
}
p.caption { color: #9aa0ab; font-size: 0.9rem; font-style: italic; margin-top: -0.5rem; }
pre {
  background: #1b1e26;
  color: #dcdfe4;
  border: 1px solid #3a3f4b;
  border-radius: 4px;
  padding: 1rem;
  overflow-x: auto;
  white-space: pre-wrap;
  word-break: break-word;
}
table { border-collapse: collapse; width: 100%; margin: 1rem 0; }
th, td {
  border: 1px solid #3a3f4b;
  padding: 0.4rem 0.6rem;
  text-align: left;
}
th { background: #1b1e26; color: #ffffff; }
tr:nth-child(even) td { background: #171a20; }
footer { color: #6b7280; font-size: 0.85rem; margin-top: 3rem; border-top: 1px solid #3a3f4b; padding-top: 1rem; }
"""


def render_report(manifest: dict, manifest_dir: Path) -> str:
    title = manifest.get("title", "VSM Task Report")
    body_parts = [f"<h1>{html_escape(title)}</h1>"]
    for section in manifest.get("sections", []):
        section_type = section.get("type")
        renderer = SECTION_RENDERERS.get(section_type)
        if renderer is None:
            raise ValueError(f"unknown section type: {section_type!r} in {section}")
        body_parts.append(renderer(section, manifest_dir))
    footer = manifest.get("footer")
    if footer:
        body_parts.append(f"<footer>{html_escape(footer)}</footer>")

    html = (
        "<!DOCTYPE html>\n"
        '<html lang="en">\n<head>\n<meta charset="utf-8">\n'
        f"<title>{html_escape(title)}</title>\n"
        f"<style>{STYLE}</style>\n</head>\n<body>\n"
        + "\n".join(body_parts)
        + "\n</body>\n</html>\n"
    )
    return html


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", help="Path to the JSON report manifest.")
    parser.add_argument("output", help="Path to write the generated index.html to.")
    args = parser.parse_args(argv)

    manifest_path = Path(args.manifest)
    output_path = Path(args.output)

    try:
        manifest = load_manifest(manifest_path)
        html = render_report(manifest, manifest_path.parent)
    except (FileNotFoundError, ValueError, json.JSONDecodeError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(html, encoding="utf-8")
    print(f"wrote {output_path} ({output_path.stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
