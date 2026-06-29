# VisualStateModel — Phase 2 Completion Report

Date: 2026-06-29  
Branch: master  
Tasks completed: 0007–0013

---

## Summary

All seven Phase 2 tasks are done. Four build targets produce clean MSVS26x64 debug
shared blitz builds; ten headless test suites all pass.

---

## Tasks

### 0007 — Phase 1 audit

**Commit:** `93292cb` VisualStateModel: audit phase 1 foundation

- Wrote `reference/VisualStateModel/PHASE1_AUDIT.md` covering build status, headless
  constraint, sample JSON, replay diagnostics, report output, workbench persistence,
  test coverage, and Phase 2 ordering.
- Fixed `%dx%d` bug in `reference/VisualStateReplayReport/ReportWriter.cpp`: U++
  `Format()` interprets `%dx` as a combined specifier. Fixed by building the rect
  string with `IntStr(w) + "x" + IntStr(h)`.

---

### 0008 — Session storage and assets

**Commit:** `981e6bde` VisualStateModel: add session storage and assets

New files:
- `uppsrc/VisualStateModel/SessionStorage.h` — `VsmSessionPaths`, `VsmFrameAsset`,
  `VsmCropAsset`, `VsmAssetRef`, `VsmSessionManifest`, `VsmSessionStore`
- `uppsrc/VisualStateModel/SessionStorage.cpp`

Key design points:
- All types in `namespace Upp {}` with `Moveable<T>` where required.
- Timestamp produced with `Format("%04d-%02d-%02dT...", t.year, ...)` — `Time::Format()`
  does not exist in U++.
- `AllocateFrame` / `AllocateCrop` write placeholder text files so `Resolve()` can
  verify asset existence without real image data.
- `VsmSessionManifest` round-trips via `Jsonize()` / `StoreAsJson` / `LoadFromJson`.

Test: `TestSessionStorage()` — Create, AllocateFrame, AllocateCrop, Resolve, round-trip.

---

### 0009 — Region annotation editor

**Commit:** `67e856c3` VisualStateModel: add region annotation editor

New files:
- `uppsrc/VisualStateModel/Annotation.h` — `VsmAnchorPoint`, `VsmHotspot`,
  `VsmRegionBinding`, `VsmRegionAnnotation`, `VsmAnnotationLayer`
- `uppsrc/VisualStateModel/Annotation.cpp`
- `reference/VisualStateWorkbench/DockPanels.h` / `.cpp` — `AnnotationEditorPanel`

`VsmAnnotationLayer::Validate()` checks:
- Empty annotation id
- Missing parent (parent_id set but not found in layer)
- Zero-size rects
- Cycles in parent chain (DFS with visited set)

Workbench: `AnnotationEditorPanel` lists annotations, supports create/delete/save;
`WhenLayerChanged` triggers validation and JSON persistence.

Test: `TestAnnotation()` — clean validation, round-trip, missing-parent detection.

---

### 0010 — Preprocessing pipeline model

**Commit:** `a07f05c2` VisualStateModel: add preprocessing pipeline model

New files:
- `uppsrc/VisualStateModel/Preprocess.h` — `VsmPreprocessStep`, `VsmPreprocessPipeline`,
  `VsmPreprocessResultRef`, `VsmPreprocessExecutor`
- `uppsrc/VisualStateModel/Preprocess.cpp`
- `reference/VisualStateWorkbench/DockPanels.h` / `.cpp` — `PipelineEditorPanel`

Steps implemented headlessly:
- Grayscale — luma = (R×77 + G×150 + B×29) >> 8
- Invert — 255 − value
- Threshold — hard cut at configurable value
- Normalize32 — nearest-neighbour resize to 32×32 (or configurable target)
- Otsu / EdgeDetect — deferred; emit warning, continue pipeline

Output: `out_img = pick(gs)` uses U++ move semantics.

Test: `TestPreprocess()` — 4-step execute, deferred-step warning, normalize 32×32,
pipeline round-trip.

---

### 0011 — Template match rule layer

**Commit:** `b2d09d92` VisualStateModel: add template match rule layer

New files:
- `uppsrc/VisualStateModel/TemplateMatch.h` — `VsmTemplateAsset`, `VsmTemplateCandidate`,
  `VsmTemplateRule`, `VsmTemplateMatchResult`, `VsmTemplateMatcher`
- `uppsrc/VisualStateModel/TemplateMatch.cpp`
- `reference/VisualStateWorkbench/DockPanels.h` / `.cpp` — `TemplateRulePanel`

Matching:
- `VsmFingerprint32` (32×32 grayscale + MD5) extracted from region image
- SSD distance via `VsmRegionMemory::Distance`; `score = 1.0 − distance`
- `matched = (score >= threshold)`
- `AddSyntheticAsset()` injects known fingerprints for headless testing

Test: `TestTemplateMatch()` — presence match, required-failure, rule round-trip.

---

### 0012 — OCR observation layer

**Commit:** `fa7454c2` VisualStateModel: add OCR observation layer

New files:
- `uppsrc/VisualStateModel/OcrLayer.h` — `VsmOcrEngineInfo`, `VsmTextExpectation`,
  `VsmOcrRule`, `VsmOcrRequest`, `VsmOcrResult`, `VsmOcrComparison`,
  `VsmOcrEngine` (abstract), `VsmFakeOcrEngine`, `VsmOcrExecutor`
- `uppsrc/VisualStateModel/OcrLayer.cpp`
- `reference/VisualStateWorkbench/DockPanels.h` / `.cpp` — `OcrRulePanel`

`VsmFakeOcrEngine` returns configurable text + confidence for headless testing.

`VsmOcrExecutor::Compare()`:
- Checks confidence vs `confidence_threshold` → warning if below
- `VSM_EXPECT_EXACT` — full string match
- `VSM_EXPECT_CONTAINS` — substring match
- Mismatch → `VSM_OCR_WARNING` severity

`OcrRulePanel` wired into `MainWindow`; seeded with rule `ocr-001` (expects "Login").

Test: `TestOcrLayer()` — engine info, RunRequest, exact/contains/mismatch compare,
OcrRule round-trip.

---

### 0013 — Application model runtime skeleton

**Commit:** `d9886a47` VisualStateModel: add application model runtime skeleton

New files:
- `uppsrc/VisualStateModel/ModelRuntime.h` — `VsmModelProperty`, `VsmModelObject`,
  `VsmModelState`, `VsmModelEvent`, `VsmModelTransition`, `VsmModelRule`,
  `VsmModelRuntimeResult`, `VsmModelRuntime`
- `uppsrc/VisualStateModel/ModelRuntime.cpp`
- `reference/VisualStateWorkbench/DockPanels.h` / `.cpp` — `ModelStatePanel`

Five rule types:

| Type | Trigger event | Effect |
|---|---|---|
| `SET_PROP_FROM_OCR` | `ocr_result` | Copy OCR text into object property |
| `SET_PROP_FROM_TEMPLATE` | `template_match` | Copy template label into property |
| `CREATE_OBJECT` | `region_appeared` | Create/activate model object |
| `MARK_INACTIVE` | `region_disappeared` | Set object inactive |
| `VALIDATE_PROP` | `ocr_result` / `template_match` | Check property == expected; emit `VsmDivergence` on mismatch |

`VsmModelRuntime::ApplyEvent()` routes each event to matching rules, records
`VsmModelTransition` entries in history, and accumulates `VsmDivergence` entries.

`ModelStatePanel` shows three tabs: Objects (with properties), Transitions, Divergences.
"Run Sample Events" button exercises the runtime live in the workbench.

Test: `TestModelRuntime()` — transition on OCR event, validate-prop match, divergence
on mismatch, history count, rule round-trip.

---

## Build results

| Target | Result |
|---|---|
| VisualStateModel (lib) | OK |
| VisualStateModelTest | OK — All VisualStateModel checks passed. |
| VisualStateWorkbench | OK |
| VisualStateReplayReport | OK |

All builds: MSVS26x64 debug shared blitz, `-m 7 -j12`.

---

## Test suite (10 suites, all pass)

```
Types round-trip          OK
Region memory             OK
Change detection          OK
Ground truth loader       OK
Replay session            OK
Application model runtime OK
OCR observation layer     OK
Template matching         OK
Preprocessing pipeline    OK
Annotation layer          OK
Session storage           OK
```

---

## Headless constraint

`uppsrc/VisualStateModel/` has no dependency on CtrlLib, Docking, Draw, or DockWindow.
All new types (`VsmModelRuntime`, `VsmOcrExecutor`, `VsmPreprocessExecutor`,
`VsmTemplateMatcher`, `VsmSessionStore`, `VsmAnnotationLayer`) compile and test without
a GUI context. Workbench adapters (`DockPanels`, `MainWindow`) live exclusively in
`reference/VisualStateWorkbench/`.
