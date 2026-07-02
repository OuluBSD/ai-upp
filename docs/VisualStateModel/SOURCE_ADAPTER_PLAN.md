# VisualStateModel — Source Adapter Implementation Plan

## Goal

Implement a layered source adapter stack that allows `VsmObservationPipeline`
to process frames from any source — existing sessions, image sequences, capture
sinks, and eventually live/network sources — without changing the pipeline
internals.

---

## Adapter Hierarchy

```
VsmFrameSource          (abstract interface)
  VsmSessionStoreSource  read back an existing VSM session
  VsmImageSequenceSource read a directory of numbered .vsm files
  VsmCaptureSink         write frames from any source into a new session
  VsmMjpegSource         parse MJPEG-over-HTTP into frames (prototype)
```

---

## Phase 4 Implementation Order

### Step 1 — `VsmFrameSource` interface + `VsmSessionStoreSource` (task 0022)

**Interface** (headless, `uppsrc/VisualStateModel/FrameSource.h`):

```cpp
class VsmFrameSource {
public:
    virtual ~VsmFrameSource() {}
    virtual bool   Open(const String& uri) = 0;
    virtual void   Close() = 0;
    virtual bool   IsReady() const = 0;
    virtual int    GetWidth()  const = 0;
    virtual int    GetHeight() const = 0;
    virtual int    GetFPS()    const = 0; // 0 if unknown
    virtual bool   ReadFrame(VsmImageBuffer& out, int64& out_ts_ms) = 0;
    virtual String GetLastError()  const { return String(); }
    virtual String GetSourceInfo() const { return String(); }
};
```

**`VsmSessionStoreSource`** reads manifest frame list, loads each `.vsm` frame
via `VsmSessionStore::LoadFrameImage`, returns `false` at end.

**Pipeline integration**: add `VsmObservationPipeline::RunFromSource(VsmFrameSource&)`
overload that drives frame reads instead of walking the `VsmSession` change list.

### Step 2 — Image sequence importer (task 0023)

**`VsmImageSequenceImporter`** (headless):

- scans directory for `.vsm` files (natural-number sort)
- creates a new session via `VsmSessionStore::Create`
- copies frames in order, assigns timestamps at 30 fps default
- records source filename in manifest `source_type = "image-sequence"`
- returns `VsmImageSequenceImportResult` with frame count and any warnings

**Reference tool**: `reference/VisualStateImportSequence/`

### Step 3 — Capture sink (task 0024)

**`VsmCaptureSink`** (headless):

- wraps a `VsmFrameSource`
- creates output session via `VsmSessionStore::Create`
- pulls frames in a loop: `ReadFrame → SaveFrameImage → UpdateManifest`
- preserves source timestamps where nonzero
- records dropped/error counts in `VsmCaptureSummary`
- does not add threading

**Reference tool**: `reference/VisualStateRecordSession/`

### Step 4 — Workbench source open/import flow (task 0025)

Extends `reference/VisualStateWorkbench` with:

- `File → Open Session…` (select directory via `SelectDirectory`)
- `File → Import Image Sequence…` (select directory, runs importer)
- `AppRegistry` persistence for last session path / last import directory
- `SessionInfoPanel` shows manifest and source info
- `File → Run Pipeline` works on any opened/imported session

### Step 5 — Annotation overlay authoring tools (task 0026)

Extends `FrameCanvas` with:

- `drag` → create annotation rect (stored in `VsmAnnotationLayer`)
- `click` → select annotation (highlight in panel)
- `drag on selected` → move annotation
- `Save Annotations` action writes through session storage

### Step 6 — MJPEG source prototype (task 0027)

**`VsmMjpegSource`** (headless where Core HTTP is enough):

- opens HTTP URL via `TcpSocket`
- parses `multipart/x-mixed-replace` boundary headers
- extracts JPEG payloads
- decodes JPEG if available (U++ `JPGRaster` — Draw dependency; defer or place
  in optional package `reference/VisualStateMjpegSource/`)
- test: deterministic static MJPEG file (no live network required)

---

## Dependencies Between Steps

```
0022 (FrameSource/SessionSource)
  ├── 0023 (ImageSequenceImporter)
  │     └── 0024 (CaptureSink)
  │           └── 0025 (WorkbenchFlow)
  │                 └── 0026 (AnnotationAuthoring)
  └── 0024 (CaptureSink)
  └── 0027 (MjpegSource) — parallel, after 0022+0024
```

---

## Format Support Matrix

| Format | Import | Export | Headless |
|---|---|---|---|
| `.vsm` (VSM1 binary) | Yes | Yes | Yes |
| JPEG | No (Phase 4) | No | No (needs Draw) |
| PNG | No (Phase 4) | No | No (needs Draw) |
| Raw MJPEG payload | Save-only (0027) | No | Partial |

JPEG/PNG headless decode is deferred; `PNGEncoder`/`JPGEncoder` live in Draw.
If needed without GUI, a minimal stb_image wrapper could be added as an
optional dependency.

---

## Headless Constraint

`uppsrc/VisualStateModel` must **never** include `CtrlLib`, `Docking`,
`Draw`/`DrawDraw`, `TopWindow`, `TabCtrl`, `ArrayCtrl`, or `DockWindow`.

Verify after each task:
```sh
rg "CtrlLib|Docking|TopWindow|TabCtrl|ArrayCtrl|DockWindow" uppsrc/VisualStateModel
```

Must return nothing.
