# VisualStateModel Phase 6 Plan

Date: 2026-07-03
Follows: Phase 5 audit (see PHASE5_AUDIT.md)

---

## Phase 5 State Summary

Phase 5 delivered:

- ts_ms persisted through capture sink and session store
- Workbench divergence panel with frame/region/detail columns and jump-to-frame
- divergences.json export at session root after every pipeline run
- Ground-truth vs observed comparison workflow (VsmGroundTruthComparison,
  comparison_result.json at session root)
- JPEG/PNG sequence import in the workbench (JpegSequenceImporter, Draw-side)

No audit gaps were found. The headless constraint is clean.

---

## Phase 6 Tasks

Four tasks are proposed, ordered by user-facing impact and implementation scope.

---

### 0035 — Annotation import/export

**Rationale:** `VsmAnnotationLayer::Save(path)` and `Load(path)` already exist
(Annotation.h:68–69) but the workbench provides no UI to use them. Users must
re-author region annotations for every new session even when the screen layout is
identical across captures. This directly limits the ground-truth comparison
workflow introduced in 0032.

**Scope:**

- Add "File → Export Annotations…" menu item: `FileSel` (save) → `layer.Save(path)`.
- Add "File → Import Annotations…" menu item: `FileSel` (open) → `layer.Load(path)`,
  then validate with `layer.Validate()` and report any errors.
- `VsmAnnotationLayer` schema version is already in JSON (`"schema": 1`); load
  should reject mismatched schema with a `PromptOK` message.
- No new headless API needed; all changes are workbench-only (`MainWindow.cpp`,
  `MenuSetup.cpp`).

**Files to commit:**
- `reference/VisualStateWorkbench/MainWindow.h`
- `reference/VisualStateWorkbench/MainWindow.cpp`
- `reference/VisualStateWorkbench/MenuSetup.cpp`
- `docs/VisualStateModel/ANNOTATION_IO.md`

**Plan file:** `plan/VisualStateModel/0035_annotation_import_export.md`

---

### 0036 — Workbench replay playback controls

**Rationale:** The workbench can jump to an arbitrary frame (`WhenJumpToFrame` /
`FrameCanvas::SetFrame(int)`) but has no way to play through a session
sequentially. Play/pause/step and a scrub bar are the expected UX for inspecting
captured frame sequences and replaying divergence events in context.

**Scope:**

- Add `ReplayBar` panel (or extend `SessionPanel`) with:
  - Play / Pause toggle (using `TimeCallback` timer, ~33 ms interval for 30 fps)
  - Step-forward / step-backward buttons
  - Horizontal `ScrollBar` acting as scrub bar (position = frame index)
- `FrameCanvas::SetFrame(int)` is the display side; the replay bar owns the
  frame counter and advances it on each timer tick.
- No headless API changes needed. Draw-side only.
- Play stops at last frame (no loop by default); speed controlled by a separate
  simple dropdown or hardcoded to match session fps from manifest.

**Files to commit:**
- `reference/VisualStateWorkbench/DockPanels.h`
- `reference/VisualStateWorkbench/DockPanels.cpp`
- `reference/VisualStateWorkbench/MainWindow.h`
- `reference/VisualStateWorkbench/MainWindow.cpp`
- `docs/VisualStateModel/REPLAY_PLAYBACK.md`

**Plan file:** `plan/VisualStateModel/0036_workbench_replay_playback.md`

---

### 0037 — Session/report export (zip artifact)

**Rationale:** After running a pipeline the user has useful artifacts spread
across a temp directory: the session frames, `divergences.json`,
`comparison_result.json`, and a replay report `.md`. There is no way to share
or archive these without manually zipping the folder. An "Export Report…"
action should package everything into one `.vsm-report.zip` file.

**Scope:**

- Add `VsmSessionExporter` to headless package (`uppsrc/VisualStateModel/`):
  - `ExportOptions { session_dir, output_zip, include_frames=false }` — frames
    are large; default is metadata-only (JSON + report).
  - `Export()` writes a zip using U++ `ZipOut` (from `archive/zip`).
  - Contents: `manifest.json`, `divergences.json` (if present),
    `comparison_result.json` (if present), any `.md` files in `reports/`.
  - Optionally include `frames/` sub-tree when `include_frames=true`.
- Add "File → Export Report…" menu item in workbench: `FileSel` (save `.zip`) →
  `VsmSessionExporter::Export()`.
- No FFmpeg / external tools.

**Files to commit:**
- `uppsrc/VisualStateModel/SessionExporter.h`
- `uppsrc/VisualStateModel/SessionExporter.cpp`
- `uppsrc/VisualStateModel/VisualStateModel.h` (add include)
- `uppsrc/VisualStateModel/VisualStateModel.upp` (add files + archive/zip to uses)
- `reference/VisualStateWorkbench/MainWindow.h`
- `reference/VisualStateWorkbench/MainWindow.cpp`
- `reference/VisualStateWorkbench/MenuSetup.cpp`
- `reference/VisualStateWorkbench/VisualStateWorkbench.upp` (add archive/zip)
- `docs/VisualStateModel/SESSION_EXPORT.md`

**Plan file:** `plan/VisualStateModel/0037_session_report_export.md`

---

### 0038 — MJPEG source JPEG decode hardening

**Rationale:** `VsmMjpegSource` (added in 0027) parses the MJPEG multipart
boundary correctly but `ReadFrame()` returns a gray placeholder buffer — no
real JPEG decode. Now that ground-truth comparison and divergence tooling exist,
it is possible to validate a live-ish MJPEG source end to end. Decode should be
wired up to turn the prototype into a functional source.

**Scope:**

- The `VsmMjpegParser` lives in the headless package and must stay headless.
  JPEG decode (`JPGRaster`) requires Draw.
- Add `MjpegDrawSource` class in the workbench package
  (`reference/VisualStateWorkbench/`) implementing `VsmFrameSource`:
  - Wraps `VsmMjpegSource` (handles HTTP/TCP read and boundary parsing).
  - On each `ReadFrame()`, takes the raw JPEG payload from
    `VsmMjpegSource::GetLastPayload()` (or equivalent), decodes it via
    `JPGRaster`, converts to `VsmImageBuffer` using the same
    `ImageToVsmBuffer()` logic as `JpegSequenceImporter`.
- Update `VisualStateMjpegSource` reference sample to use `MjpegDrawSource` and
  assert that at least one real frame is decoded (not all-zero/gray).
- Keep `VsmMjpegParser` and `VsmMjpegSource` in the headless package unchanged.
- Do not add real network/webcam dependencies; synthetic MJPEG stream test
  (`VsmMakeSyntheticMjpeg()`) is sufficient for the acceptance test.

**Files to commit:**
- `reference/VisualStateWorkbench/MjpegDrawSource.h`
- `reference/VisualStateWorkbench/MjpegDrawSource.cpp`
- `reference/VisualStateWorkbench/VisualStateWorkbench.upp`
- `reference/VisualStateMjpegSource/main.cpp`
- `docs/VisualStateModel/MJPEG_SOURCE.md` (update decode status)

**Plan file:** `plan/VisualStateModel/0038_mjpeg_decode_hardening.md`

---

## Non-Goals for Phase 6

- FFmpeg, OpenCV, or webcam capture.
- Redesign of `ModelRuntime`, `PipelineRunner`, or comparison logic.
- Batch UI (no multi-session batch processing dialog).
- Video file export (no .avi/.mp4 encoding).

---

## Phase 6 Task Summary

| # | Task | Scope | Location |
|---|---|---|---|
| 0035 | Annotation import/export | Add File→Import/Export Annotations | Workbench only |
| 0036 | Replay playback controls | Play/pause/step/scrub bar | Workbench only |
| 0037 | Session/report export (zip) | VsmSessionExporter + workbench UI | Headless + workbench |
| 0038 | MJPEG decode hardening | MjpegDrawSource wrapping VsmMjpegSource | Workbench only |
