# VisualStateModel Phase 5 Audit

Date: 2026-07-03
Tasks audited: 0029–0033

---

## Build and Test Status

All commands run from `E:/active/sblo/Dev/ai-upp`.

| Target | Build | Test run |
|---|---|---|
| VisualStateModel (library) | **LNK2019 expected** (no `main`; library-only package) | — |
| VisualStateModelTest | **OK** | 16 suites pass (all checks) |
| VisualStateReplayReport | **OK** | — |
| VisualStateWorkbench | **OK** | — |
| VisualStateEndToEndSample | **OK** | 10/10 acceptance checks pass |
| VisualStateRecordSession | **OK** | all record/replay checks pass |
| VisualStateImportSequence | **OK** | all 4-frame import checks pass |

Note: `bin/build.exe -m 7 -j12 VisualStateModel` fails with LNK2019 because
`VisualStateModel` is a library package with no `mainconfig` entry; the build
system attempts to link a stub executable. This is expected — all consuming
packages build and test clean.

`git diff --check -- uppsrc/VisualStateModel reference docs plan/VisualStateModel`:
no output (no whitespace errors).

---

### Commit Verification

All Phase 5 commits are in the permanent history:

```
a0df8865c  reference: add JPEG/PNG sequence import to workbench
7572f4f7a  VisualStateModel: add ground-truth vs observed comparison workflow
b5ef14b9c  VisualStateModel: export divergences to JSON; add replay report section
a6a3f59b0  reference: extend workbench divergence panel with frame/region/detail
73e3c1083  VisualStateModel: persist per-frame timestamps in session manifest
```

Note: the completion report for task 0033 (`tmp/visual-report.txt`) contained a
"Phase 5 tasks now complete" section with mislabeled task descriptions (calling
0029 "Pipeline cache" and 0030 "MJPEG source adapter"). Those labels belong to
earlier phases. The mislabeled text appeared **only in the tmp report file**
and was **not propagated into any committed doc** — confirmed by `rg "Pipeline
cache|MJPEG source adapter" docs/VisualStateModel` returning no results. No doc
fix was required.

---

## Headless Constraint

`rg "CtrlLib|Docking|TopWindow|TabCtrl|ArrayCtrl|DockWindow" uppsrc/VisualStateModel/`
returns nothing.

`JpegSequenceImporter` and all other Draw-dependent code (JPGRaster, PNGRaster,
JPGEncoder, ImageBuffer) are confined to `reference/VisualStateWorkbench/`.
The headless comment in `ImageSequenceImporter.h` ("JPEG/PNG import requires
Draw/GUI (deferred)") remains accurate.

**Constraint is clean.**

---

## Task-by-Task Verification

### 0029 — `ts_ms` timestamp persistence (`73e3c1083`)

`VsmFrameAsset` (`SessionStorage.h:33`) gained `int64 ts_ms = -1`.
`SaveFrameImage()` (`SessionStorage.h:82`) now accepts `ts_ms` as a third arg.
`VsmCaptureSink::Record()` threads the timestamp through from `src.ReadFrame()`.
`VsmSessionStoreSource::ReadFrame()` reconstructs timestamps from the manifest.

`VisualStateModelTest` confirms round-trip:

```
Timestamp round-trip:
  ts_ms round-trip: 0, 33, 100 ms OK
```

### 0030 — Divergences panel columns + jump-to-frame (`a6a3f59b0`)

`DivergencesPanel` (DockPanels.h:45–53) exposes `frame`, `region_id`,
`expected_json`, `observed_json` columns and a read-only JSON detail view.
`WhenJumpToFrame` (DockPanels.h:51) fires when the user double-clicks a row.
`FrameCanvas::SetFrame(int)` (FrameCanvas.h:18) navigates the canvas.
`MainWindow::OnJumpToFrame()` (MainWindow.cpp:835–845) wires them together.

### 0031 — `divergences.json` export + replay report section (`b5ef14b9c`)

`VsmModelRuntime::SaveDivergenceReport()` (ModelRuntime.cpp:213) writes valid
JSON to `<session_root>/divergences.json`.

Called from `PipelineRunner.cpp:272–273` (standard run) and `:338–339`
(ground-truth run).

E2E sample verified the file at
`$TEMP/vsm_e2e_sample/divergences.json` (478 bytes, 2-record JSON array,
valid structure). `VisualStateReplayReport` includes a "Divergences" table
when the file is present and non-empty.

### 0032 — Ground-truth vs observed comparison (`7572f4f7a`)

`VsmGroundTruthComparison::Compare()` and `VsmObservationPipeline::RunWithGroundTruth()`
are in `PipelineRunner.h` and `.cpp`. `comparison_result.json` is written to
`<session_root>/comparison_result.json` (PipelineRunner.cpp:361).

E2E sample verified the file at
`$TEMP/vsm_e2e_sample/comparison_result.json` (351 bytes, valid JSON with
`matched`, `missing`, `unexpected` keys and `entries` array).

E2E acceptance checks:
```
OK: ground-truth comparison: 2 matched, 0 missing, 0 unexpected
```

### 0033 — JPEG/PNG sequence import (`a0df8865c`)

`JpegSequenceImporter` lives exclusively in `reference/VisualStateWorkbench/`.
`VsmJpegImportOptions`, `VsmJpegImportResult`, and `JpegSequenceImporter` are
defined in `JpegSequenceImporter.h`.

`OnImportImageSequence()` dispatches:
- `.jpg`/`.png` only → `RunJpegImport()` → `JpegSequenceImporter`
- `.vsm` present → `RunVsmImport()` → `VsmImageSequenceImporter`
- neither → `PromptOK`

`RunJpegSmokeTest()` at startup:
- Fixed source path `GetTempPath()/vsm_jpeg_smoke_src` — re-created each run.
- Fixed output path `GetTempPath()/vsm_jpeg_smoke_out` — deleted via
  `DeleteFolderDeep()` before each run.
- No temp dir accumulation on repeated runs. **Log spam / leftover dir check: PASS.**

`plugin/jpg` must be in `uses` AND `<plugin/jpg/jpg.h>` must be explicitly
included — BLITZ does not auto-include plug package headers.
`VisualStateWorkbench.upp` has `plugin/jpg` in `uses`; `JpegSequenceImporter.h`
has `#include <plugin/jpg/jpg.h>`.

---

## JSON Output Paths

| File | Written to | Verified |
|---|---|---|
| `divergences.json` | `<session_root>/divergences.json` | E2E sample session root |
| `comparison_result.json` | `<session_root>/comparison_result.json` | E2E sample session root |

Both files are well-formed JSON (confirmed by `Get-Content` and E2E acceptance
checks).

---

## Minor Issues

None found. The mislabeled Phase 5 summary in `tmp/visual-report.txt` was never
committed, and no doc corrections were required.
