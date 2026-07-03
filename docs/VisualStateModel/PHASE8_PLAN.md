# VisualStateModel Phase 8 Plan

Date: 2026-07-03
Follows: Phase 7 audit (see PHASE7_AUDIT.md)

---

## Phase 7 State Summary

Phase 7 delivered all ten headless CLI/tooling tasks (0039–0048):

- Session validator + CLI
- Batch divergence report + CLI
- HTML output for replay report
- Session-to-session divergence diff + CLI
- Deterministic replay regression test
- Ground truth template generator + CLI
- Manifest backward-compatibility regression test
- Annotation validator CLI (+ bounds checking)
- Pipeline cache stats CLI
- Region fingerprint dump CLI

All 17 test suites pass. The headless constraint is clean (no GUI dependencies
pulled into `uppsrc/VisualStateModel/`). No audit gaps were found.

Phase 6 (0035–0038) is **partially implemented**:
- Annotation authoring UI exists but import/export menu items may be incomplete
- Replay playback controls (step/run/reset) are present but need fine-tuning
- Session/report export to zip is **not implemented**
- MJPEG decode is **not implemented** (MJPEG parser exists but returns placeholders)

---

## Phase 8 Tasks

Four tasks are proposed, all focused on completing Phase 6 work and preparing
for downstream phases. These are **concrete gaps**, not speculative work.

---

### 0050 — Complete annotation import/export UI

**Rationale:** Phase 6 added annotation authoring to the workbench but did not
wire up File→Import/Export menu items. Users cannot yet save/load annotations
to/from disk, limiting the ground-truth comparison workflow introduced in 0032.

**Scope:**

- Add "File → Import Annotations…" menu item: `FileSel` (open) → `layer.Load(path)`,
  validate with `layer.Validate()`, report errors via `PromptOK`.
- Add "File → Export Annotations…" menu item: `FileSel` (save) → `layer.Save(path)`.
- `VsmAnnotationLayer` schema version check already in JSON; load should reject
  mismatched schema with a message.
- No new headless API needed; all changes are workbench-only (`MainWindow.cpp`,
  `MenuSetup.cpp`).
- Update `docs/VisualStateModel/ANNOTATION_AUTHORING.md` to document the UI.

**Files to commit:**
- `reference/VisualStateWorkbench/MainWindow.cpp`
- `reference/VisualStateWorkbench/MenuSetup.cpp`
- `docs/VisualStateModel/ANNOTATION_AUTHORING.md` (update)

**Plan file:** `plan/VisualStateModel/0050_annotation_import_export_ui.md`

---

### 0051 — Session/report export to zip artifact

**Rationale:** Phase 7 completed divergence reporting, comparison workflows, and
ground-truth tools. The user now has many useful outputs spread across a temp
directory: session frames, `divergences.json`, `comparison_result.json`, and
replay report `.md`. There is no way to package these for sharing or archiving.
An "Export Report…" action should create a zip file with all metadata and
optionally frames.

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
- `reference/VisualStateWorkbench/MainWindow.cpp`
- `reference/VisualStateWorkbench/MenuSetup.cpp`
- `reference/VisualStateWorkbench/VisualStateWorkbench.upp` (add archive/zip)
- `docs/VisualStateModel/SESSION_EXPORT.md` (new)

**Plan file:** `plan/VisualStateModel/0051_session_report_export.md`

---

### 0052 — MJPEG source JPEG decode hardening

**Rationale:** `VsmMjpegSource` (from Phase 4) parses the MJPEG multipart
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

**Plan file:** `plan/VisualStateModel/0052_mjpeg_decode_hardening.md`

---

### 0053 — Replay timeline UI refinements

**Rationale:** Phase 6 added play/pause/step controls to the replay timeline,
but the UI may need refinement for:
- Smooth frame playback without UI lag
- Scrub bar position sync with playback state
- FPS display and speed control
- Bounds checking (stop at last frame, no loop by default)

**Scope:**

- Audit current `ReplayTimelinePanel` for responsiveness and sync issues.
- Add FPS display (from manifest or hardcoded 30 fps) and speed control dropdown.
- Ensure scrub bar position updates in real-time during playback.
- Add frame counter display (current / total).
- Fix any UI lag due to frame rendering blocking the timer callback.
- Update `docs/VisualStateModel/WORKBENCH.md` to document replay controls.

**Files to commit:**
- `reference/VisualStateWorkbench/DockPanels.h`
- `reference/VisualStateWorkbench/DockPanels.cpp`
- `reference/VisualStateWorkbench/MainWindow.cpp` (if menu callbacks added)
- `docs/VisualStateModel/WORKBENCH.md` (update)

**Plan file:** `plan/VisualStateModel/0053_replay_timeline_refinements.md`

---

## Non-Goals for Phase 8

- Batch UI (multi-session processing dialog).
- Video file export (.avi, .mp4).
- Real-time network MJPEG streaming (synthetic tests only).
- Redesign of any Phase 5/6/7 component.

---

## Phase 8 Task Summary

| # | Task | Scope | Location |
|---|---|---|---|
| 0050 | Annotation import/export UI | File→Import/Export Annotations menu items | Workbench only |
| 0051 | Session/report export (zip) | VsmSessionExporter + workbench UI | Headless + workbench |
| 0052 | MJPEG decode hardening | MjpegDrawSource wrapping VsmMjpegSource | Workbench only |
| 0053 | Replay timeline refinements | UI polish, FPS display, scrub sync | Workbench only |

---

## Sequencing Rationale

1. **0050 (annotation UI)** first — fastest and unblocks ground-truth workflows.
2. **0051 (session export)** second — wraps up user-facing export story.
3. **0052 (MJPEG decode)** third — enables end-to-end MJPEG validation.
4. **0053 (replay refinements)** last — polish and performance tuning.

Phase 8 is modest in scope: primarily completing Phase 6 work and minor
refinements. No new core concepts are introduced. All tasks are well-bounded
and low risk.
