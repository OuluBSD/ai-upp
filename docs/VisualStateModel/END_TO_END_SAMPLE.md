# VisualStateModel — End-to-End Sample

## Overview

`reference/VisualStateEndToEndSample/` is a buildable U++ console application
demonstrating the complete VisualStateModel pipeline in a single self-contained
run.

---

## What the Sample Does

1. **Creates a session** in `<temp>/vsm_e2e_sample/` with a manifest.
2. **Generates two synthetic frame images** (`.vsm` format):
   - Frame 0: solid gray 128 (baseline)
   - Frame 1: checkerboard pattern (simulates a changed region)
3. **Builds a ground truth session** with 1 change event, 1 changed rect, 1
   stable region.
4. **Sets up an annotation** covering the full frame (`ann-login`).
5. **Configures preprocessing**: grayscale → normalize 32×32.
6. **Runs template matching** with a synthetic fingerprint asset (≈ checkerboard).
7. **Runs fake OCR** returning `"Login"` for the changed region.
8. **Pre-seeds a divergence** by injecting a `"Dashboard"` OCR event before the
   pipeline run. This causes the validate rule to see `"Login" ≠ "Dashboard"`.
9. **Runs the full observation pipeline** — all observations go to model runtime.
10. **Prints a concise CLI summary** with acceptance checks.
11. **Writes run outputs**: `observations.json`, `diagnostics.json`.
12. **Generates a markdown report**: `reports/e2e_report.md`.

---

## Running the Sample

```sh
bin/build.exe -m 7 -j12 VisualStateEndToEndSample
bin\VisualStateEndToEndSample.exe
```

Expected output:

```
=== VisualStateModel End-to-End Sample ===

Session created: …\vsm_e2e_sample
Pre-seeded 'Dashboard' event to force divergence

--- Pipeline Run Summary ---
Observations:    3
Transitions:     2
Divergences:     1
Success:         yes

--- Acceptance Checks ---
OK:   Pipeline run success
OK:   At least 1 frame processed
OK:   At least 1 observation
OK:   At least 1 model transition
OK:   At least 1 divergence
OK:   At least 1 model object

All end-to-end checks passed.
```

---

## Output Files

```
<temp>/vsm_e2e_sample/
  manifest.json
  frames/
    00000000.vsm    — solid gray frame
    00000001.vsm    — checkerboard frame
  runs/
    run-<id>/
      observations.json
      diagnostics.json
  reports/
    e2e_report.md   — markdown report with transitions and divergences
```

---

## Workbench Integration

1. Run `VisualStateEndToEndSample.exe` to generate the session.
2. Open `VisualStateWorkbench.exe`.
3. Select `File → Load E2E Sample Session`.
4. The Session Info panel shows the e2e-sample-001 manifest.
5. Select `File → Run Pipeline` to run the observation pipeline on the sample.
6. The Model State panel shows transitions and divergences.
7. The Debug log shows cache hit/miss stats and summary.

---

## Build and Test

All five targets build clean:

```
bin/build.exe -m 7 -j12 VisualStateModel
bin/build.exe -m 7 -j12 VisualStateModelTest
bin/build.exe -m 7 -j12 VisualStateReplayReport
bin/build.exe -m 7 -j12 VisualStateWorkbench
bin/build.exe -m 7 -j12 VisualStateEndToEndSample
bin\VisualStateEndToEndSample.exe   → All end-to-end checks passed.
```

Headless test suite: 14 suites, all pass.
