# VisualStateModel — Observation Pipeline Runner

## Overview

`VsmObservationPipeline` (in `uppsrc/VisualStateModel/PipelineRunner.h`) is the
end-to-end interpretation engine for Phase 3. It ties together all Phase 1 and
Phase 2 components:

```
session changes
    ↓
  changed regions (VsmChangedRect)
    ↓
  annotation matching (VsmAnnotationLayer)
    ↓
  region crop extraction (VsmImageBuffer or synthetic)
    ↓
  preprocessing (VsmPreprocessPipeline)
    ↓
  template match (VsmTemplateMatcher)  →  observation("template_match")
  OCR (VsmOcrEngine / VsmFakeOcrEngine) →  observation("ocr_result")
    ↓
  model events → VsmModelRuntime
    ↓
  transitions / divergences
```

---

## Data Types

### VsmObservation

A single pipeline output. Stored in `observations.json`.

| Field | Type | Description |
|---|---|---|
| `type` | String | `"template_match"`, `"ocr_result"`, `"region_appeared"`, `"region_disappeared"` |
| `region_id` | String | Source region or annotation id |
| `annotation_id` | String | Matched annotation id |
| `rule_id` | String | Which template/OCR rule fired |
| `data_json` | String | Serialized result (match score, OCR text, …) |
| `ts` | String | Timestamp |
| `frame` | int | Frame index |

### VsmPipelineRunSummary

Summary counters for a completed run.

| Field | Description |
|---|---|
| `run_id` | Unique run identifier |
| `session_id` | Source session |
| `started_at` | UTC timestamp |
| `frames_processed` | Unique frame indices |
| `change_events` | Total change events iterated |
| `regions_detected` | Total changed rects processed |
| `observations_made` | Total observations emitted |
| `transitions` | Model state transitions |
| `divergences` | Model validate divergences |
| `success` | Run completed without fatal error |

### VsmPipelineDiagnostics

Structured log of warnings and errors from the run.

| Entry field | Description |
|---|---|
| `level` | `"info"`, `"warning"`, `"error"` |
| `source` | Component: `"Pipeline"`, `"OCR"`, `"Preprocess"`, `"ModelRuntime"` |
| `message` | Human-readable text |

---

## VsmObservationPipeline API

```cpp
VsmObservationPipeline pipe;
pipe.SetLog(&log);
pipe.SetSession(&session);          // VsmSession from ground truth / replay
pipe.SetSessionStore(&store);       // optional: for real frame image I/O
pipe.SetAnnotationLayer(&layer);    // annotations → region mapping
pipe.SetPreprocessPipeline(&pp);    // preprocessing steps
pipe.SetTemplateRules(&rules);      // template rules to evaluate
pipe.SetTemplateMatcher(&matcher);  // matcher with synthetic/real assets
pipe.SetOcrRules(&ocr_rules);       // OCR rules
pipe.SetOcrEngine(&fake_ocr);       // defaults to VsmFakeOcrEngine if not set
pipe.SetModelRuntime(&runtime);     // runtime to apply events to

VsmPipelineRunSummary summary = pipe.Run();

// Access results:
pipe.GetObservations();    // Vector<VsmObservation>
pipe.GetDiagnostics();     // VsmPipelineDiagnostics

// Write files:
pipe.SaveOutputs(session_root, summary.run_id);
```

---

## Output Files

Stored under `<session_root>/runs/<run_id>/`:

```
runs/
  run-<id>/
    observations.json   — Array of VsmObservation
    diagnostics.json    — VsmPipelineDiagnostics (warnings, errors)
```

`summary.json` is not saved by the pipeline runner itself — it is the caller's
responsibility if needed. The workbench shows summary data in the debug log.

---

## Run Loop

1. Iterate `session.changes` (VsmChangeEvent array).
2. For each `VsmChangedRect` in each change event:
   - Find annotation overlapping the rect by bounding box.
   - Get region crop: real frame image (from `VsmSessionStore`) or synthetic gray
     buffer based on `rect.score`.
   - Run preprocessing pipeline on the crop.
   - Run all template rules for the annotation → emit `template_match` observations.
   - Run all OCR rules for the annotation → emit `ocr_result` observations.
3. After changes: emit `region_appeared` events for stable region nodes.
4. Each observation is applied as a `VsmModelEvent` to the model runtime.

---

## Workbench Integration

"Run Pipeline" is available in:
- `File → Run Pipeline` menu item
- Toolbar button (second `go_forward` button)

On completion:
- Summary counts are logged to the Debug panel.
- `ModelStatePanel` is refreshed with updated runtime state.
- Outputs are saved if the session store is open.

---

## Headless Constraint

`PipelineRunner.h` / `.cpp` live in `uppsrc/VisualStateModel` and depend only
on `Core`. No GUI types are used. The workbench integration in
`MainWindow.cpp` is the only GUI-side code.

---

## Test Coverage

`TestPipelineRunner()` in `reference/VisualStateModelTest/main.cpp`:

- Loads sample session with 1 change event
- Wires annotation, preprocessing, template matcher, fake OCR, model runtime
- Runs the pipeline
- Verifies: `observations > 0` and `transitions > 0`
- Calls `SaveOutputs` and verifies `observations.json` was created

All 13 test suites pass as of task 0016.
