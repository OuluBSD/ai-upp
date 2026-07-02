# VisualStateModel Phase 4 Audit

Date: 2026-07-02
Tasks audited: 0022–0027

---

## Build and Test Status

All commands run from `E:/active/sblo/Dev/ai-upp`.

| Target | Build | Test run |
|---|---|---|
| VisualStateModel (library) | **LNK2019 expected** (no `main`; library-only package) | — |
| VisualStateModelTest | **OK** | 16 suites pass |
| VisualStateReplayReport | **OK** | — |
| VisualStateWorkbench | **OK** | — |
| VisualStateEndToEndSample | **OK** | 6/6 acceptance checks pass |
| VisualStateRecordSession | **OK** | all record/replay checks pass |
| VisualStateImportSequence | **OK** | all 4-frame import checks pass |
| VisualStateMjpegSource | **OK** | all MJPEG prototype checks pass |

Note: `bin/build.exe -m 7 -j12 VisualStateModel` fails with LNK2019 because
`VisualStateModel` is a library package with no `mainconfig` entry; the build
system attempts to link a stub executable. This is expected — all consuming
packages build and test clean.

### Commit Verification

All Phase 4 commits are in the permanent history:

```
fc875565a  VisualStateModel: add MJPEG source prototype
7e642770f  reference: add VisualStateWorkbench annotation authoring
b840bdbfb  reference: add VisualStateWorkbench source import flow
e878c27b0  VisualStateModel: add capture sink session writer
d2b5fdc96  VisualStateModel: add image sequence importer
4dfa331bc  VisualStateModel: add frame source session adapter
c69e79485  VisualStateModel: audit phase 3 source adapter plan
```

---

## Headless Constraint

`rg "CtrlLib|Docking|TopWindow|TabCtrl|ArrayCtrl|DockWindow" uppsrc/VisualStateModel/`
returns nothing. **Constraint is clean.**

---

## Reference / Docs Layout

All `reference/` directories contain a `.upp` package file and build.
No documentation prose is under `reference/` (only package `.upp` + `main.cpp`).
All prose lives under `docs/VisualStateModel/`.

---

## Gap Analysis

### Gap 1 — JPEG/PNG import in ImageSequenceImporter

**Status: CONFIRMED GAP.**

`uppsrc/VisualStateModel/ImageSequenceImporter.h` line 64 states:
```
// JPEG/PNG import requires Draw/GUI (deferred).
```
`VsmImageSequenceImporter` only recognises `.vsm` files.

`VsmFrameAsset.format` already accepts `"jpg"` / `"png"` strings (SessionStorage.h:32),
so the manifest schema is ready.

`VisualStateWorkbench` depends on CtrlLib and Draw, making it the natural home
for a Draw-based JPEG/PNG importer. Phase 5 should add this.

---

### Gap 2 — Timestamp persistence in CaptureSink

**Status: CONFIRMED GAP.**

`VsmFrameAsset` (SessionStorage.h:29–34) has no `ts_ms` or timestamp field.

`VsmCaptureSink::Record` (CaptureSink.cpp:43–45) reads `ts_ms` from
`src.ReadFrame(img, ts_ms)` but the call to `store_.SaveFrameImage(frame_idx, img)`
(CaptureSink.cpp:47) does not pass the timestamp. Timestamps are silently discarded.

`VsmSessionStoreSource::ReadFrame` (FrameSource.cpp) approximates timestamps as
`frame_index * 33 ms` regardless of original capture timing.

Consequence: inter-frame timing information from real capture sources (e.g. MJPEG
stream at variable frame rate) is lost. Replay is currently frame-count-based, not
time-based.

Phase 5 should add `int64 ts_ms` to `VsmFrameAsset`, extend `SaveFrameImage()`, and
thread the value through CaptureSink and VsmSessionStoreSource.

---

### Gap 3 — Divergence surfacing completeness

**Status: SKELETON EXISTS — surface incomplete.**

`VsmDivergence` (Types.h:117) is fully structured:
```cpp
struct VsmDivergence {
    int    frame;
    String ts, severity, message;
    VsmRegionId region_id;
    String expected_json, observed_json;
};
```

Current divergence plumbing:
- `VsmModelRuntime::GetDivergences()` accumulates all divergences.
- `VsmPipelineRunSummary.divergences` counts them.
- `VsmObservationPipeline` logs each divergence at `LOG_WARN` level.
- `VisualStateReplayReport/main.cpp` prints only the count.
- Workbench `DockPanels` "Divergences" tab shows `message` + `severity` only.

What is missing:
1. Workbench divergence panel: `frame`, `region_id`, `expected_json`, `observed_json`
   columns not shown; no "jump to frame" action.
2. Divergence export: no JSON/text dump after a pipeline run; divergences are
   held in memory only and lost on next run.
3. ReplayReport: no "Divergences" section listing individual records — only count.
4. No ground-truth-vs-observed comparison workflow for imported/recorded sessions.

---

### Gap 4 — MJPEG hardening

**Status: PROTOTYPE IS COMPLETE AS DESIGNED. No hardening needed before Phase 5.**

The MJPEG boundary parser (`VsmMjpegParser`) is solid: state machine, incremental
feed, `Content-Length` support, boundary-search fallback, and deterministic test
helper (`VsmMakeSyntheticMjpeg`). All MJPEG reference package checks pass.

JPEG decode (Draw::JPGRaster) is explicitly deferred and documented in
`docs/VisualStateModel/MJPEG_SOURCE.md`. The `VsmMjpegSource::ReadFrame()` returns
a gray placeholder buffer until decode is wired up.

**Decision: leave MJPEG as-is for Phase 5. Do not harden JPEG decode before
divergence reporting is addressed.** Rationale: the parser is functional for
testing; live MJPEG decode requires Draw and is a GUI-layer concern that can be
added independently.

---

## Phase 4 API Surface Added to `uppsrc/VisualStateModel`

```
VsmFrameSource          abstract interface (FrameSource.h)
VsmFrameSourceInfo      metadata struct
VsmSessionStoreSource   replays existing VSM sessions via VsmFrameSource
VsmImageSequenceImportOptions / VsmImageSequenceImporter
VsmImageSequenceImportResult / VsmImportWarning
VsmCaptureSinkOptions / VsmCaptureSink / VsmCaptureSummary
VsmMjpegPartHeader / VsmMjpegParser
VsmMakeSyntheticMjpeg() helper
VsmObservationPipeline::RunFromSource(VsmFrameSource&)
```

---

## Minor Doc Issues Fixed

None found during this audit.
