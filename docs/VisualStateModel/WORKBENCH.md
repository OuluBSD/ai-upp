# VisualStateModel — Workbench Overlay and Selection Tools

## Overview

`reference/VisualStateWorkbench` provides a practical GUI for inspecting session
data, changing regions, annotations, template/OCR results, and model state.

---

## FrameCanvas Overlays

`FrameCanvas` (in `reference/VisualStateWorkbench/FrameCanvas.h`) renders a
dark placeholder frame view with optional layered overlays:

| Overlay | Color | Description |
|---|---|---|
| **Regions** | Orange/yellow | `VsmChangedRect` from replay or pipeline |
| **Annotations** | Green | `VsmRegionAnnotation` from the annotation layer |
| **Template** | Blue | `VsmTemplateMatchResult` badges (T+ matched, T- not matched) |
| **OCR** | Gold | `VsmOcrResult` text badges near top-right corner |

Selected region/annotation is highlighted in a brighter color.

### Overlay Data Methods

```cpp
frame_canvas_.SetChangedRegions(regions);
frame_canvas_.SetAnnotationLayer(&annotation_layer_);
frame_canvas_.SetTemplateResults(&template_results);
frame_canvas_.SetOcrResults(&ocr_results);
```

---

## Overlay Toggles

Four toggle buttons appear at the right end of the toolbar:

| Button | Toggle method | Key |
|---|---|---|
| Regions | `SetShowRegions(bool)` | `ShowRegions()` |
| Annotations | `SetShowAnnotations(bool)` | `ShowAnnotations()` |
| Template | `SetShowTemplate(bool)` | `ShowTemplate()` |
| OCR | `SetShowOcr(bool)` | `ShowOcr()` |

Toggle state is persisted in `AppRegistry` under keys:
```
overlay.regions
overlay.annotations
overlay.template
overlay.ocr
```

These are loaded in `DockInit → LoadOverlayState()` and saved when toggled.

---

## Selection

Clicking on the canvas:
1. Checks annotation hit first (higher priority).
2. Falls back to region hit.
3. Fires `WhenRegionSelected(id)` with the annotation id or `"region-N"`.
4. `MainWindow` routes this to `RegionPropsPanel` and logs the selection.

---

## Pipeline Integration

**Run Pipeline** is in:
- `File → Run Pipeline`
- Toolbar button (second `go_forward`)

After running:
- Summary counts appear in the Debug log panel.
- `ModelStatePanel` refreshes with new transitions/divergences/state.
- Outputs are written to `<session_root>/runs/<run_id>/` if the session store
  is open.

---

## Dock Panels

| Panel | Content |
|---|---|
| Region Properties | Selected region/annotation properties |
| Replay Timeline | Replay step/run/reset controls |
| Session Info | Session manifest fields |
| Annotation Editor | Create/edit/delete region annotations |
| Pipeline Editor | Preprocessing pipeline steps |
| Template Rules | Template rules list and match results |
| OCR Rules | OCR rules and result display |
| Model State | Objects, transitions, divergences (three tabs) |

---

## Source Open and Import Flow (Phase 4)

### File → Open Session…

Opens a directory picker. The selected directory is opened as a
`VsmSessionStoreSource`. Session Info panel updates to show the manifest.
The last opened path is persisted under `session.last_path` in `AppRegistry`.

### File → Import Image Sequence…

Opens a directory picker for the source directory of `.vsm` files. Runs
`VsmImageSequenceImporter` to create a new session in a temp directory, then
opens it with `OpenSessionPath`. Last import dir persisted under
`session.last_import_dir`.

### File → Run Pipeline (with source session)

When a session is opened via `Open Session…` or `Import Image Sequence…`,
`Run Pipeline` calls `VsmObservationPipeline::RunFromSource()` instead of the
change-event-based `Run()`. Summary shows frame count, observations, transitions,
divergences, and cache hits/misses.

### Persisted Registry Keys (Phase 4)

| Key | Type | Description |
|---|---|---|
| `session.last_path` | String | Last opened session directory |
| `session.last_import_dir` | String | Last import source directory |

---

## Limitations (Phase 3/4)

- Frame images are not yet displayed (placeholder dark background only).
  Displaying real `VsmImageBuffer` pixels requires a conversion to `Upp::Image`
  (Draw package) at the GUI boundary — planned for a future task.
- Overlay positions are relative to the top-left of the canvas using screen
  coordinates from the session data; they are not scaled to the canvas size.
- No zoom/pan — coordinates are 1:1 pixel mapping.
- Import directory picker shows all directories; `.vsm` file filter not applied
  at directory level.
