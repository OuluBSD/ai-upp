# VisualStateModel — Phase 2 Audit

Date: 2026-06-29  
Branch: master  
Audits tasks: 0007–0013

---

## Layout Verification

### reference/ constraint

Every directory under `reference/` that is related to VisualStateModel is a
buildable U++ package with a `.upp` file:

| Package | .upp file | Builds |
|---|---|---|
| `reference/VisualStateWorkbench/` | VisualStateWorkbench.upp | OK |
| `reference/VisualStateModelTest/` | VisualStateModelTest.upp | OK |
| `reference/VisualStateReplayReport/` | VisualStateReplayReport.upp | OK |

`reference/VisualStateModel/` does **not** exist. It was removed when
documentation was moved to `docs/`.

### docs/ layout

Documentation lives under `docs/VisualStateModel/`:

| File | Description |
|---|---|
| `ARCHITECTURE.md` | Package structure and dependency diagram |
| `GROUND_TRUTH.md` | Ground truth JSON format specification |
| `PHASE1_AUDIT.md` | Phase 1 audit findings |
| `REPORT.md` | Phase 2 completion report (corrected test count) |
| `PHASE2_AUDIT.md` | This file |

No documentation files exist under `reference/`.

---

## Build Results

Built with `bin/build.exe -m 7 -j12` (MSVS26x64 debug shared blitz):

| Target | Result | Notes |
|---|---|---|
| VisualStateModelTest | **OK** | All 11 suites pass |
| VisualStateWorkbench | **OK** | GUI app |
| VisualStateReplayReport | **OK** | Console report tool |
| VisualStateModel (standalone) | link error | Expected — no `main()`; it is a library used by other packages |

The standalone link error for VisualStateModel is expected behaviour: in U++
all packages compile as unity source included by their consumer. The package
itself does not produce a standalone executable.

---

## Test Suite Results (11 suites)

```
=== Types round-trip ===
ChangedRect: OK
RegionFingerprint: OK

=== Region memory ===
RegionMemory count: OK
FindNearest: OK
Distance self: OK

=== Change detection ===
DetectChanges: OK
VsmCompareFrames: OK
Identical frames: no changes OK
Fingerprint hash: OK

=== Ground truth loader ===
Schema: 1 OK
Frames: 2 OK
Changes: 1 — regions: 1 OK
Regions: 1 OK
Divergences: 1 OK

=== Replay session ===
RunAll: OK
Divergences: 1 OK
AppLog records: 9

=== Application model runtime ===
Transition: screen = "Login" OK
ValidateProp match: OK
ValidateProp divergence: OK
History entries: 2 OK
ModelRule round-trip: OK

=== OCR observation layer ===
FakeOcrEngine: OK
RunRequest: OK
Compare exact match: OK
Compare contains: OK
Compare mismatch → warning: OK
OcrRule round-trip: OK

=== Template matching ===
Presence match: OK
Required-failure: OK
Rule round-trip: OK

=== Preprocessing pipeline ===
Execute: 4 steps run OK
Deferred step warning: OK
Normalize 32x32: OK
Pipeline round-trip: OK

=== Annotation layer ===
Validate clean layer: OK
Annotation round-trip: count=2 OK
Validate missing parent: OK

=== Session storage ===
Create: OK
AllocateFrame(0): OK
AllocateCrop: OK
Resolve + placeholder exists: OK
Manifest round-trip: OK

All VisualStateModel checks passed.
```

---

## Headless Constraint Check

`uppsrc/VisualStateModel` uses only the `Core` package. No GUI headers leak in:

```
rg "CtrlLib|Docking|TopWindow|TabCtrl|ArrayCtrl|DockWindow" uppsrc/VisualStateModel
```

Result: no matches.

---

## REPORT.md Correction

The Phase 2 completion report (`docs/VisualStateModel/REPORT.md`) previously
stated "10 suites" but listed 11 test functions. Corrected to "11 suites".

---

## Phase 3 Readiness

All Phase 2 targets are clean. The following are ready for Phase 3:

- Session storage with placeholder frame/crop assets
- Annotation layer with validate + JSON round-trip
- Preprocessing pipeline (grayscale, invert, threshold, normalize32)
- Template match with fingerprint-based SSD scoring
- OCR layer with fake engine and exact/contains comparison
- Model runtime with five rule types and divergence tracking
- Workbench with all dock panels wired
