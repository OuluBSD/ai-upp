# VisualStateModel Architecture

## Executive Summary

VisualStateModel is a change-first desktop observation and replay system.
Its primary signal is _which pixels changed_ between consecutive frames, not
what a whole frame looks like. Changed regions drive hierarchy construction,
OCR, template matching, and application-state modelling. Replay detects
divergence between expected and observed state. Meaningful logic lives in a
headless Core package; a GUI workbench observes and edits that state.

Old working labels (`desktop-parser`, `desktop-website`, `desktop-diary`,
`game/*`) are historical memory aids and are **not** final package names.

---

## Source Paths Inspected

| Path | Purpose |
|------|---------|
| `uppsrc/Core/AppLog.h` / `AppLog.cpp` | Structured log: stores `AppLogRecord` records, fires `WhenRecord` |
| `uppsrc/Core/CoreLog.h` / `CoreLog.cpp` | Per-component log bridge: owned `Vector<String>` + forwards to `AppLog*` sink |
| `uppsrc/Core/AppRegistry.h` / `AppRegistry.cpp` | Versioned key-value / JSON / blob persistence |
| `reference/DockingTemplate2/` | GUI reference scaffold: docking, menu, toolbar, Debug log tab, `AppRegistry` persistence |
| `reference/AppRegistry/` | Console reference for `AppRegistry` and `AppLog` patterns |

Old prototype paths (`../ConvNetCpp`, `../PKR`) were not present in this
workspace and are not a blocker.

---

## Prototype Lessons To Keep

- **Frame-level metadata is useful** — timestamp, source, dimensions.
- **Crop images per detected region** — small files, external references, not inline JSON.
- **Markdown reports are inspectable** — one file per event/frame, linked from an index.
- **Ground truth comparison** — frame-level annotations compared against parsed output.
- **Region rectangles** — manually or automatically placed, carrying stable ids.
- **OCR and template matching pipelines** — triggered per-region, not per-frame.
- **Divergence is the primary failure** — expected state ≠ observed state.

## Prototype Lessons To Avoid

- Do not treat the frame as the primary unit; lead with change events.
- Do not make neural networks a required core dependency.
- Do not use Python scripts as the primary interpretation layer.
- Do not store large image data inline in JSON.
- Do not let GUI layout blobs carry meaningful model state.
- Do not hard-code region rectangles in GUI code.

---

## Old Working Labels And Their Status

| Label | Status |
|-------|--------|
| `desktop-parser` | Retired — describes OCR/parsing; not a package boundary |
| `desktop-website` | Retired — describes HTML export; optional adapter |
| `desktop-diary` | Retired — describes replay diary export; optional adapter |
| `game/*` | Historical — illustrates controlled simulator input; still valid _use case_ |

---

## Core Concepts

**Frame** — a single captured image, identified by frame index and timestamp.
A frame may reference an external image file; it is not stored inline.

**Change event** — the observation that one or more image areas changed between
frame N−1 and frame N. Change events are the primary driver of downstream
processing.

**Changed region** — a rectangle (or merged set of rectangles) that covers
pixels that changed above a noise threshold. Changed regions carry the frame
index and a confidence score.

**Region identity** — a stable logical identifier assigned to a visual region
across movement and resizing. Identity is tracked by fingerprint similarity,
not by pixel coordinates alone.

**Region fingerprint** — a compact normalized representation of a region's
visual content (e.g. 32×32 grayscale, hashed). Used for identity matching
across frames.

**Region hierarchy** — the parent-child relationships between regions (e.g. a
window contains a toolbar which contains buttons). Hierarchy is built from
mechanical observations first; user annotations refine it.

**User annotation** — a human- or tool-added label, rule, or hierarchy override
applied to a region. Annotations should not replace raw observations; they
augment them.

**Anchor / hotspot** — a named point or sub-rectangle within a region used as
a stable reference for relative coordinates (e.g. a button center).

**Preprocessing pipeline** — a sequence of image transforms (resize, normalize,
threshold, contrast) applied to a region before OCR or template matching.
Recipes are stored by reference, not inlined.

**OCR result** — text extracted from a region by an OCR engine. Attached to the
region and the frame that triggered it. May arrive asynchronously.

**Template match result** — the outcome of comparing a region image against a
stored template. Records match score and bounding rectangle.

**Observation** — any raw machine-generated fact about a region: change event,
fingerprint, OCR result, template match result. Observations are separated from
user annotations.

**Modeled application state** — a structured, application-specific
representation derived from observations and rules (e.g. `{ screen: "Login",
field_focused: "username" }`). This is the semantic layer above raw pixels.

**Divergence** — a detected mismatch between the expected modeled state
(ground truth) and the observed/interpreted state. Divergence is the primary
replay failure mode.

**Replay session** — a sequential pass over a recorded event stream that
reconstructs observations, applies rules, and compares against ground truth.
Produces a divergence/warning log and an optional debug report.

---

## Proposed Package Boundaries

### `uppsrc/VisualStateModel` — Headless Core Package

The heart of the system. No GUI dependencies. Usable from replay tests,
CLI tools, and the workbench.

Contains:
- Data types (`VsmFrameRef`, `VsmChangeEvent`, `VsmRegionId`, `VsmRegionNode`,
  `VsmObservation`, `VsmDivergence`, `VsmReplaySession`, …)
- Ground truth JSON serialization / deserialization
- Region memory (fingerprint storage and matching)
- Replay engine (event iteration, divergence detection)
- Per-package logging via `CoreLog`, forwarded to `AppLog` sink

No dependencies on `CtrlLib`, `Docking`, `Draw`, or any GUI package.

### `reference/VisualStateWorkbench` — GUI Workbench

Observes and edits `VisualStateModel` state. Based on `DockingTemplate2`
patterns. Uses `AppLog` as the structured sink; wires headless `CoreLog`
instances to it.

Contains:
- Main frame/video tab (placeholder or real rendering)
- Region hierarchy dock
- Region memory/matches dock
- Annotation/properties dock
- Event/replay timeline dock
- Debug log tab (reuse `DebugLog` pattern from `DockingTemplate2`)
- `AppRegistry` persistence for UI prefs and dock layout

### `reference/VisualStateReplayReport` — CLI Report Generator

Headless CLI tool. Loads a ground truth/replay JSON file and writes a
markdown report directory. No GUI dependency.

### Optional Adapters (future)

- Video capture adapter (reads frames from file or live source)
- Live HTML view / diary exporter
- Game/simulator ground truth exporter/importer

---

## Headless Runtime

The headless package is the only place where meaningful model state lives.

Typical session lifecycle:
```
VsmReplaySession session;
session.SetLog(&core_log);
session.Load("recording.vsm.json");
while(session.Step()) {
    // process events: change detection, region update, model transition, divergence check
}
session.WriteReport("report/");
```

`CoreLog` carries per-session diagnostics. When attached to an `AppLog` sink,
records appear in the GUI Debug tab or are printed headless.

---

## GUI Workbench

The workbench owns an `AppLog log_` (like `DockingTemplate2/MainWindow`).
Each headless component it instantiates receives `log_.` as its `CoreLog` sink.

Visualization principles:
- Unchanged areas dimmed to ~20% brightness
- Changed regions highlighted with colored overlay
- Selected region broadcasts cursor event (reuse `ModeManager`/`WhenCursorEmit`
  pattern from `DockingTemplate2`)
- Mechanical observations and user annotations displayed in separate panes

---

## Change Detection Layer

Inputs: two consecutive frames (image buffers).
Outputs: `Vector<VsmChangedRegion>` — rectangles above noise threshold.

Steps:
1. Pixel-level absolute difference.
2. Threshold (configurable noise floor).
3. Morphological merge of nearby changed pixels into rectangles.
4. Optional: coarse grid pass for speed.

Image buffers: prefer U++ `Image` (from `Draw` package). If `Draw` requires
GUI context at link time, use a thin `VsmFrameImage` adapter that holds raw
RGBA bytes and avoids the GUI package. Investigate before committing.

---

## Region Identity And Hierarchy

Region identity tracking:
1. Compute `VsmRegionFingerprint` for each changed region (32×32 grayscale, MD5 or xxHash).
2. Query `VsmRegionMemory` for previous fingerprints within a distance threshold.
3. Assign existing `VsmRegionId` on match; allocate new id on miss.
4. Record position delta (moved) and size delta (resized) as part of the event.

Hierarchy:
- Initial containment hierarchy from bounding-box inclusion.
- User annotations can override parent/child assignments.
- Hierarchy events are stored in the ground truth stream.

---

## Rule And Pipeline Layer

Not implemented in the first tasks. Design points:
- Rules reference region ids and trigger pipelines on change events.
- Preprocessing recipes are stored by name; the engine resolves them at runtime.
- OCR and template matching are rule-triggered, not frame-global.
- Results attach to the triggering change event and region.

---

## Application Model Layer

Not implemented in the first tasks. Design points:
- A `VsmModelState` is a named set of key-value observations (strings, rects,
  booleans) representing the current interpreted UI state.
- Transitions are logged as events in the replay stream.
- Ground truth provides expected transitions; replay checks actual vs expected.

---

## Ground Truth And Replay

The ground truth file is JSON. Schema-versioned. Large image data is
external (file references only, never inline).

Key event types in the stream:
- `frame` — timestamp, index, optional image file reference
- `change` — changed region rectangles for a frame pair
- `region` — region id assignment, fingerprint, hierarchy
- `annotation` — user label/rule override
- `ocr` — OCR result attached to region
- `template` — template match result attached to region
- `state_snapshot` — full modeled state at a checkpoint
- `transition` — modeled state change
- `divergence` — expected vs observed state mismatch
- `missing_frame` — gap in the recording, with optional patch
- `warning` / `error` — replay diagnostic

---

## Logging, Diagnostics, And Reports

### Accepted Logging Path

Both `AppLog` and `CoreLog` exist in Core.

| Class | Role | Use For |
|-------|------|---------|
| `AppLog` | Structured log: `Vector<AppLogRecord>`, `WhenRecord` event | GUI-visible log, owned by the workbench `MainWindow` |
| `CoreLog` | Per-component bridge: `Vector<String>` + `AppLog*` sink | Headless components; call `LogInfo/LogWarn/LogError` free functions |

**Recommendation:** each headless class owns a `CoreLog` member. At session
start, the caller calls `component.GetLog().SetSink(&app_log)`. This keeps
headless code free of GUI types while still feeding the GUI Debug tab.

Do not create another logging abstraction.

### Headless Diagnostics

For replay CLI tools: `AppLog` with `SetForwardToUppLog(true)` prints to the
U++ LOG stream. No GUI needed.

### Debug Report

The replay report generator writes a markdown directory. One `index.md`
links to per-event pages. Divergence and warnings are at the top.
Crop images are external files referenced by path.

---

## Storage And Cache Strategy

| Data | Storage |
|------|---------|
| UI preferences, dock layout | `AppRegistry` blobs (as in `DockingTemplate2`) |
| Ground truth / replay event stream | Project-specific JSON files, external to `AppRegistry` |
| Crop images | External PNG/JPEG files, referenced by relative path |
| Region fingerprints (cache) | Optional binary cache file next to the session file |
| Preprocessing recipe definitions | JSON, loaded by name |

---

## Build And Verification Strategy

- `bin/build.exe -m 7 -j12 VisualStateModel` — headless package (no GUI links)
- `bin/build.exe -m 7 -j12 VisualStateReplayReport` — CLI report generator
- `bin/build.exe -m MSVS26x64 -j12 VisualStateWorkbench` — GUI workbench

After each headless build, verify no GUI dependencies leaked:
```sh
rg "CtrlLib|Docking|TopWindow|TabCtrl|ArrayCtrl|DockWindow" uppsrc/VisualStateModel
```

---

## Risks And Open Questions

1. **Image API dependency** — `Upp::Image` lives in the `Draw` package. If
   `Draw` pulls in GUI context, the headless package cannot use it. Mitigation:
   use a raw RGBA byte buffer adapter `VsmFrameImage` until dependency is
   clarified.

2. **Fingerprint collision** — simple 32×32 grayscale + distance may produce
   false positives in very similar UI regions. Risk is low for the first
   prototype; add a secondary hash check if needed.

3. **Async OCR latency** — OCR results may arrive after the frame is displayed.
   The event format must support late-arriving observations. Not a build risk;
   addressed in the format spec (Task 0002).

4. **Region identity across drastic changes** — if a window is closed and a
   new one opens in the same area, the fingerprint distance should exceed the
   threshold. Tuning will be empirical.

5. **Missing frame patching** — game/simulator sources may need to inject
   synthetic frames. The format must support a `missing_frame` record and a
   `patch` source.

---

## Recommended Implementation Tasks

| Task | Output |
|------|--------|
| 0002 | `GROUND_TRUTH.md` — versioned JSON format spec |
| 0003 | `uppsrc/VisualStateModel/` — compilable headless skeleton |
| 0004 | Change detection + region memory prototype in headless package |
| 0005 | `reference/VisualStateWorkbench/` — GUI skeleton based on `DockingTemplate2` |
| 0006 | `reference/VisualStateReplayReport/` — CLI replay debug report generator |
