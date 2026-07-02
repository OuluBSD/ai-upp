# VisualStateModel Phase 5 Plan — Divergence Reporting

Date: 2026-07-02

---

## Goal

Phase 5 closes the most important remaining gaps:

1. Divergence records are fully structured but not usefully surfaced (Gap 3).
2. Capture timestamps are read from sources but not persisted (Gap 2).
3. JPEG/PNG images cannot be imported via the headless package (Gap 1).
4. MJPEG live decode remains deferred (Gap 4 — leave as-is; see decision below).

The primary deliverable is a **ground-truth vs observed comparison workflow**
accessible from both the workbench GUI and the `VisualStateReplayReport` CLI.

---

## Gap 4 Decision: MJPEG

Do not harden MJPEG decode in Phase 5. The boundary parser is solid and the
decode blocker is well-documented. Address it in a later phase or as a
standalone task when there is an actual live MJPEG source to test against.

---

## Ordered Phase 5 Task List

### 0029 — Timestamp Persistence in Frame Assets

**Why**: Timestamps from `ReadFrame(img, ts_ms)` are currently discarded by
`VsmCaptureSink`. Timing-sensitive replay and frame-level divergence attribution
require real timestamps.

**Files**:
- `uppsrc/VisualStateModel/SessionStorage.h` — add `int64 ts_ms = -1` to
  `VsmFrameAsset`; update `Jsonize`
- `uppsrc/VisualStateModel/SessionStorage.cpp` — update `SaveFrameImage()` to
  accept `ts_ms` parameter and store it in the manifest entry
- `uppsrc/VisualStateModel/CaptureSink.cpp` — pass `ts_ms` from `ReadFrame()`
  to `store_.SaveFrameImage(frame_idx, img, ts_ms)`
- `uppsrc/VisualStateModel/FrameSource.cpp` — `VsmSessionStoreSource::ReadFrame()`
  returns stored `ts_ms` from manifest if ≥ 0; falls back to `frame_index * 33 ms`

**Tests**: update `TestFrameSource()` in `VisualStateModelTest` to verify that
timestamps written by `VsmCaptureSink` round-trip through `VsmSessionStoreSource`.

---

### 0030 — Divergence Detail Panel in Workbench

**Why**: The workbench "Divergences" tab shows only `message` + `severity`.
The structured fields (`frame`, `region_id`, `expected_json`, `observed_json`)
are present in `VsmDivergence` but never displayed.

**Files**:
- `reference/VisualStateWorkbench/DockPanels.h` — add columns for frame,
  region_id, expected, observed to `divergences_list_`
- `reference/VisualStateWorkbench/DockPanels.cpp` — populate all columns in
  `ModelStatePanel::Refresh()`; add `WhenSel` callback on `divergences_list_`
  that fires `WhenJumpToFrame(int frame)` event
- `reference/VisualStateWorkbench/MainWindow.h/.cpp` — wire `WhenJumpToFrame`
  from `ModelStatePanel` to `FrameCanvas::SetFrame(int)`

**Acceptance**: after running the pipeline on a session that produces divergences,
the workbench "Divergences" tab shows frame number, region, expected/observed JSON.
Double-clicking a row navigates the frame canvas to that frame.

---

### 0031 — Divergence Export and ReplayReport Section

**Why**: Divergences held in memory are lost after each pipeline run. There is
no machine-readable output and no narrative report section.

**Files**:
- `uppsrc/VisualStateModel/ModelRuntime.h/.cpp` — add
  `SaveDivergenceReport(const String& path)`: serialise
  `GetDivergences()` as a JSON array to `session/divergences.json`
- `uppsrc/VisualStateModel/PipelineRunner.cpp` — call
  `model_rt_->SaveDivergenceReport(...)` at end of `Run()` / `RunFromSource()`
  if a session store is configured
- `reference/VisualStateReplayReport/main.cpp` — add "Divergences" section to
  the Markdown report (table of frame / severity / message / region);
  read from `session/divergences.json` if present
- `reference/VisualStateEndToEndSample/main.cpp` — assert divergence count and
  verify `divergences.json` was written

**Acceptance**: running `VisualStateEndToEndSample` produces
`session/divergences.json`; `VisualStateReplayReport` includes a Divergences
section with per-record detail.

---

### 0032 — Ground-Truth vs Observed Comparison Workflow

**Why**: No explicit "compare recorded session against expected model state"
workflow exists. Gap 3 lists this as the most important missing piece.

**Files**:
- `uppsrc/VisualStateModel/GroundTruth.h/.cpp` — add
  `VsmGroundTruthComparison` class that accepts a `VsmGroundTruthSession`
  (expected events) and a `Vector<VsmDivergence>` (observed divergences from
  `ModelRuntime`) and produces a `VsmComparisonResult` with per-event match
  status (matched / unexpected / missing)
- `uppsrc/VisualStateModel/PipelineRunner.h/.cpp` — add
  `RunWithGroundTruth(VsmFrameSource&, VsmGroundTruthSession&)`:
  runs the pipeline then invokes `VsmGroundTruthComparison`; stores result
- `reference/VisualStateWorkbench/MainWindow.h/.cpp` — add
  "Compare with Ground Truth…" menu item: loads a ground truth JSON file
  (FileSelect), runs `RunWithGroundTruth`, populates divergence panel with
  comparison result, shows a summary PromptOK (e.g. "3/4 events matched, 1
  unexpected divergence")
- `reference/VisualStateEndToEndSample/main.cpp` — exercise
  `RunWithGroundTruth` with the existing synthetic session and sample ground
  truth; verify comparison result

**Acceptance**: sample produces `VsmComparisonResult` with at least one matched
and one divergent event.

---

### 0033 — JPEG/PNG Sequence Import (Draw-side, Workbench only)

**Why**: `VsmImageSequenceImporter` defers JPEG/PNG because Draw is unavailable
in the headless package. The workbench already depends on Draw via CtrlLib.

**Files**:
- `reference/VisualStateWorkbench/JpegSequenceImporter.h/.cpp` — concrete
  importer (non-headless): scans directory for `.jpg`/`.png`/`.jpeg`, decodes
  each via `JPGRaster`/`PNGRaster`, converts to `VsmImageBuffer`, calls
  `VsmSessionStore::SaveFrameImage()`
- `reference/VisualStateWorkbench/MainWindow.cpp` — connect
  "Import Image Sequence…" to `JpegSequenceImporter` when the selected
  directory contains JPEG/PNG (falls back to existing
  `VsmImageSequenceImporter` for `.vsm` directories)
- `reference/VisualStateWorkbench/MainWindow.h` — add member
  `JpegSequenceImporter jpeg_importer_`
- Documentation: `docs/VisualStateModel/JPEG_PNG_IMPORT.md`

**No headless package changes needed** — JPEG decode stays in the workbench.

**Acceptance**: the workbench "Import Image Sequence…" flow successfully imports
a directory of JPEG files and replays the resulting session.

---

## Plan File Summary

| File | Title |
|---|---|
| `plan/VisualStateModel/0029_timestamp_persistence.md` | Timestamp persistence in frame assets |
| `plan/VisualStateModel/0030_divergence_detail_panel.md` | Divergence detail panel in workbench |
| `plan/VisualStateModel/0031_divergence_export_replay_report.md` | Divergence export and report section |
| `plan/VisualStateModel/0032_ground_truth_comparison_workflow.md` | Ground-truth vs observed comparison |
| `plan/VisualStateModel/0033_jpeg_png_import_workbench.md` | JPEG/PNG sequence import (Draw-side) |

---

## Execution Order

Run tasks in order: 0029 → 0030 → 0031 → 0032 → 0033.

0029 (timestamps) is a prerequisite for accurate per-frame divergence display
in 0030 and 0032. 0031 (export) can be done concurrently with 0030 (panel).
0033 (JPEG import) is independent and can be done last.
