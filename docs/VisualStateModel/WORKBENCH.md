# VisualStateModel — Workbench Overlay and Selection Tools

## Overview

`reference/VisualStateWorkbench` provides a practical GUI for inspecting session
data, changing regions, annotations, template/OCR results, and model state.

---

## Active Session Model

`MainWindow` holds two session backends, but only ever presents **one active
session** to the toolbar and the Frame/Regions tabs at a time:

- **Sample replay session** (`replay_`, a `VsmReplaySession`) — loaded by
  `LoadSampleSession()`, which runs automatically at startup and whenever
  toolbar **Reset** confirms (or silently, if no opened session was active).
- **Opened/imported session** (`src_source_`, a `VsmSessionStoreSource`) —
  loaded via `File → Open/Import Session…` (source type "Existing session
  directory", "Image sequence", or "Built-in E2E sample data"), all of which
  ultimately go through `OpenSessionPath()`.

`has_src_session_` is the single source of truth for which one is active:
`true` once an opened/imported session has been loaded, `false` while only
the sample is loaded. Every control below branches on it instead of assuming
a fixed session:

| Control | `has_src_session_ == false` | `has_src_session_ == true` |
|---|---|---|
| Toolbar **Step** / **Run All** | `replay_.Step()` / `RunAll()` (event-based) | `src_source_.ReadFrame()`, once or in a loop |
| Toolbar **Reset** | reloads the sample, no prompt | prompts for confirmation first (see below); on confirm, closes `src_source_` and reloads the sample |
| Toolbar/File **Run Pipeline** | `VsmObservationPipeline::Run()` over `replay_.GetSession()` | `VsmObservationPipeline::RunFromSource(src_source_)` |
| FrameCanvas region click / Regions tab row select | looks up the clicked/selected id in `replay_.GetSession().regions` | see "Region Data Gap" below |

### Reset confirmation

Toolbar/timeline **Reset** calls `OnResetReplay()`. If an opened/imported
session is active, it shows a yes/no prompt ("This will discard the currently
opened session and reload the built-in sample. Continue?") before doing
anything; answering No leaves the opened session untouched. If no opened
session is active, Reset reloads the sample immediately with no prompt, same
as before this session-identity unification.

### Region data gap (opened/imported sessions)

Opened/imported sessions (B) have no `VsmRegionNode`-shaped region list in
the headless API: `VsmSessionStore`/`VsmSessionStoreSource` only track
frames and crops (`VsmFrameAsset`/`VsmCropAsset`), and
`VsmObservationPipeline::RunFromSource()` only emits a synthetic full-frame
`VsmChangedRect` plus `VsmObservation` records (no stable region id/rect).
So while an opened session is active, clicking a region on `FrameCanvas` or
selecting a row in the Regions tab reports "no region data available for the
opened session" instead of incorrectly matching against the sample session's
regions (which is what happened before this fix). Closing this gap for real
would mean extending the headless session/observation model to expose a
stable region list for source-backed sessions — out of scope for workbench
bookkeeping alone.

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
4. `MainWindow` routes this to `RegionPropsPanel` and logs the selection,
   looking up the id in whichever session is active (see "Active Session
   Model" above) — not always the sample session.

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

| Panel | Content | Dock |
|---|---|---|
| Region Properties | Selected region/annotation properties | RIGHT |
| Session Info | Session manifest fields | RIGHT |
| Replay Timeline | Replay step/run/reset controls | BOTTOM |
| Annotation Editor | Create/edit/delete region annotations | LEFT (primary) |
| Model State | Objects, transitions, divergences (three tabs) | LEFT (primary) |
| Pipeline Editor | Preprocessing pipeline steps | LEFT (secondary, tabbed) |
| Template Rules | Template rules list and match results | LEFT (secondary, tabbed) |
| OCR Rules | OCR rules and result display | LEFT (secondary, tabbed) |

### LEFT dock: primary/secondary tiers

The five LEFT-docked panels are not flat/equally-weighted. `MainWindow`
splits them into two tiers (`OnResetDockLayout()` in `MainWindow.cpp`):

- **Primary** (own pane, visible by default): `AnnotationEditorPanel` and
  `ModelStatePanel`. Defining what to annotate is the first thing a new
  session needs, and `ModelStatePanel` is where the payoff (divergences)
  shows up — both stay in view without any extra clicks.
- **Secondary** ("Rules & Preprocessing", one shared tabbed dock slot, not
  the default front tab): `PipelineEditorPanel`, `TemplateRulePanel`,
  `OcrRulePanel`. These matter once a session and annotations already exist
  (all three share the Run-against-real-frame affordance from Phase 4/task
  0060), so instead of three always-visible panes they share one dock slot
  via `DockWindow::Tabify(target, dc)` — U++ Docking's primitive for
  "attach `dc` as another tab of whatever container `target` is docked in"
  (see `uppsrc/Docking/Docking.h`; the same pattern
  `reference/DockingTemplate2/INVESTIGATION.md` documents adopting from
  `nide`'s `RecognizerEditorWindow::OnResetDockLayout()`). `PipelineEditorPanel`
  is the default front tab (preprocessing precedes rule matching);
  `TemplateRulePanel`/`OcrRulePanel` are present in the same slot and fully
  usable by clicking their tab, but are not in front by default.

Nothing about the three secondary panels' behavior changed — this is
docking layout only. A user can still select/expand the "Rules &
Preprocessing" group and use `PipelineEditorPanel`/`TemplateRulePanel`/
`OcrRulePanel` exactly as before.

**Persisted layouts:** `AppRegistry`'s `dock.layout` blob (`SaveUserLayout()`
/ `LoadUserLayout()`) identifies each docked panel by its index into the
`Register()`-order docker list, not by screen position — and that
registration order was not changed by this restructuring. A layout saved
before this change (five separate LEFT panes) still deserializes cleanly:
each panel is repositioned into whatever container/pane the blob recorded
for it, which is the old flat five-pane arrangement, not the new tabbed
grouping. In other words, existing users keep their previously-saved flat
layout after upgrading (no crash, no data loss); they only see the new
primary/secondary grouping once they use **View → Reset Dock Layout** (or on
a fresh profile with no saved `dock.layout` blob yet).

---

## Source Open and Import Flow (Phase 4)

### File → Open/Import Session…

Single entry point for every way to load a session (replaces the old four
separate flat File-menu items). Opens a small modal dialog
(`OpenImportDialog`, file-local to `MainWindow.cpp`) with:

- A source-type chooser (`DropList`): *Existing session directory*, *Image
  sequence (.vsm/.jpg/.png)*, *Built-in sample data*, *Built-in E2E sample
  data*.
- A directory field + "Browse…" button (calling `SelectDirectory()`), shown
  only for the two directory-based types.
- "Open" and "Cancel" buttons. "Open" refuses to close (with a prompt) if a
  directory-based type has no path chosen yet.

`MainWindow::OnOpenImportSession()` reads back the chosen type/path and
dispatches to the exact same calls the old menu items used, unchanged:

| Source type | Dispatches to |
|---|---|
| Existing session directory | `OpenSessionPath(path)` |
| Image sequence | `DispatchImageSequenceImport(path)` — detects `.vsm` vs `.jpg`/`.png` (`HasVsmFiles()`/`HasJpegFiles()`) and calls `RunVsmImport(path)` or `RunJpegImport(path)`, same logic the old "Import Image Sequence…" handler used |
| Built-in sample data | `LoadSampleSession()` |
| Built-in E2E sample data | `OnLoadE2ESample()` |

Opening a session this way (directory or image-sequence types, and the E2E
sample) makes the opened session the active session — see "Active Session
Model" above. The last opened path/import dir are still persisted under
`session.last_path`/`session.last_import_dir` in `AppRegistry`, same as
before.

### File → Run Pipeline (with source session)

When a session is opened via `Open/Import Session…` (any of the
directory-based or E2E-sample types), `Run Pipeline` calls
`VsmObservationPipeline::RunFromSource()` instead of the change-event-based
`Run()`. Summary shows frame count, observations, transitions, divergences,
and cache hits/misses.

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
- Opened/imported sessions have no region-list data available for Frame/
  Regions tab selection — see "Region data gap" under "Active Session Model"
  above.
