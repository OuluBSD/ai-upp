# VisualStateModel — HTML Replay Report

## Overview

`VisualStateReplayReport` generates a Markdown report (`index.md`) containing
session statistics and divergence data from a recorded screen-interaction
session. The `--html` flag enables parallel HTML output (`index.html`),
rendering the same information as semantic HTML (plain tables and headings,
no CSS framework or JavaScript).

---

## Command Line

### Default Behavior (Markdown Only)

```sh
bin\VisualStateReplayReport.exe [output_directory]
```

Writes:
- `index.md` — session statistics and divergences table (if present)
- `events/` — per-event markdown pages

### With HTML Output

```sh
bin\VisualStateReplayReport.exe --html [output_directory]
```

Writes:
- `index.md` — Markdown report (unchanged)
- `index.html` — HTML report with the same information
- `events/` — per-event markdown pages (unchanged)

If no `output_directory` is specified, defaults to a temporary directory.

---

## HTML Report Content

The HTML report contains:

### 1. Session Information

Metadata extracted from the loaded session:
- Session ID
- Source type (e.g., "video", "image_sequence")
- Frame dimensions (width × height)
- Timestamps (started, ended)

### 2. Load Warnings

If any warnings were logged during session loading, they are listed as an
HTML unordered list.

### 3. Session Statistics

A table summarizing counts of:
- Frames
- Change events
- Regions
- OCR results
- Template matches
- State snapshots
- Divergences

### 4. Divergences Table

If `divergences.json` is present in the output directory (written by the
pipeline runtime), a table with these columns:
- **Frame** — frame number at which divergence was detected
- **Severity** — "error", "warning", "fatal", or other user-defined level
- **Region** — region ID where divergence occurred, or "—" if not region-specific
- **Message** — human-readable description of the divergence

---

## HTML Escaping

All content rendered into the HTML (session IDs, OCR text, divergence messages,
JSON blobs) is escaped for HTML safety. Characters `<`, `>`, and `&` are
converted to `&lt;`, `&gt;`, and `&amp;` respectively. This ensures that text
originating from OCR or model output cannot accidentally break the HTML
structure or introduce unintended markup.

---

## Non-Goals

- No styling framework (Bootstrap, Tailwind, etc.).
- No JavaScript or dynamic interactivity.
- No client-side filtering or search.
- No linked event pages in the HTML version (only in Markdown).

---

## Reference Tool

`reference/VisualStateReplayReport/` demonstrates:

1. Loads a sample session.
2. Runs replay to generate divergences.
3. Writes both `index.md` and (with `--html`) `index.html`.

```sh
bin/build.exe -m 7 -j12 VisualStateReplayReport
bin\VisualStateReplayReport.exe --html
```

Output:

```
Session: 'sample-session'
Events: frames=10 changes=3 regions=5 divergences=2

...

Report written to: C:\temp\vsm_report
  index.md — main entry point
  index.html — HTML version
  events/  — per-event pages
```

Both `index.md` and `index.html` can be opened directly in an editor or browser
respectively.

---

## Design Notes

- The HTML writer reuses the same session data as the Markdown writer; no
  duplicate collection or preprocessing.
- HTML output is completely independent — removing the `--html` flag restores
  purely Markdown output with no side effects.
- The generated HTML is self-contained and does not require external assets.
