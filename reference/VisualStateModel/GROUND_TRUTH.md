# VisualStateModel Ground Truth And Replay Format

## Schema Version

This document describes **schema version 1** (`"schema": 1`).

All ground truth files must include a top-level `"schema"` integer field.
Parsers must reject files with a schema version they do not understand.
Forward-compatible additions (new optional fields) increment a minor counter
that parsers may ignore. Breaking changes increment the major `"schema"` integer.

---

## Top-Level File Structure

A ground truth file is a UTF-8 JSON object:

```json
{
  "schema": 1,
  "producer": { ... },
  "session": { ... },
  "events": [ ... ]
}
```

All timestamps are ISO-8601 strings with millisecond precision, e.g.
`"2026-01-15T14:23:00.123Z"`. Frame indices are zero-based integers.

---

## Producer Metadata

```json
"producer": {
  "name": "VisualStateModel",
  "version": "0.1.0",
  "created_at": "2026-01-15T14:23:00.000Z",
  "host_os": "Windows",
  "notes": "optional free-text field"
}
```

---

## Session Metadata

```json
"session": {
  "id": "uuid-or-slug",
  "source_type": "desktop_recording",
  "frame_width": 1920,
  "frame_height": 1080,
  "fps_hint": 30,
  "started_at": "2026-01-15T14:23:00.000Z",
  "ended_at":   "2026-01-15T14:23:59.999Z",
  "image_dir":  "frames/",
  "crop_dir":   "crops/"
}
```

`source_type` values: `"desktop_recording"`, `"game_export"`, `"simulator"`,
`"synthetic"`.

`image_dir` and `crop_dir` are relative to the JSON file's directory.
Large image data is **never** embedded in the JSON; always use file references.

---

## Event Array

`"events"` is an ordered array. Events are processed in array order.
Each event has at minimum:

```json
{
  "type": "<event_type>",
  "frame": 42,
  "ts": "2026-01-15T14:23:01.400Z"
}
```

`frame` is the zero-based frame index. `ts` is the wall-clock timestamp.
Events without a meaningful frame (session-level) may omit `frame`.

---

## Event Types

### `frame`

A new captured frame is available.

```json
{
  "type": "frame",
  "frame": 0,
  "ts": "2026-01-15T14:23:00.000Z",
  "image_file": "frames/000000.png"
}
```

`image_file` is optional. When absent, only metadata is available for
this frame.

---

### `change`

Pixel-level change events between frame N−1 and frame N.

```json
{
  "type": "change",
  "frame": 1,
  "ts": "2026-01-15T14:23:00.033Z",
  "regions": [
    { "x": 100, "y": 200, "w": 80, "h": 40, "score": 0.91 },
    { "x": 500, "y": 10,  "w": 200, "h": 30, "score": 0.75 }
  ]
}
```

`score` is the fraction of pixels changed inside the rectangle (0–1).

---

### `region`

Region identity assignment or update.

```json
{
  "type": "region",
  "frame": 1,
  "ts": "2026-01-15T14:23:00.033Z",
  "region_id": "rgn-0001",
  "action": "created",
  "rect": { "x": 100, "y": 200, "w": 80, "h": 40 },
  "parent_id": "rgn-0000",
  "fingerprint": "sha1:a3f8c...",
  "fingerprint_file": "crops/rgn-0001-f001.bin"
}
```

`action` values: `"created"`, `"moved"`, `"resized"`, `"merged"`, `"split"`,
`"closed"`.

`parent_id` links to the parent region in the hierarchy; omit for root regions.

`fingerprint` is a compact hash string. `fingerprint_file` is an optional
path to the full normalized fingerprint buffer (32×32 grayscale bytes).

---

### `annotation`

User- or tool-added label, rule, or hierarchy override.

```json
{
  "type": "annotation",
  "frame": 5,
  "ts": "2026-01-15T14:23:00.166Z",
  "region_id": "rgn-0001",
  "label": "LoginButton",
  "parent_override": "rgn-0000",
  "properties": {
    "ocr_enabled": true,
    "template": "templates/login_btn.png"
  },
  "author": "human"
}
```

`author` values: `"human"`, `"tool"`, `"import"`.

---

### `ocr`

OCR result for a region. May arrive after the triggering frame.

```json
{
  "type": "ocr",
  "frame": 5,
  "ts": "2026-01-15T14:23:00.500Z",
  "region_id": "rgn-0001",
  "trigger_frame": 5,
  "text": "Log In",
  "confidence": 0.97,
  "engine": "tesseract",
  "lang": "eng",
  "crop_file": "crops/rgn-0001-f005-ocr.png"
}
```

`trigger_frame` is the frame that caused the OCR run. The event `frame`
field records when the result became available (may differ for async engines).

---

### `template`

Template match result for a region.

```json
{
  "type": "template",
  "frame": 5,
  "ts": "2026-01-15T14:23:00.510Z",
  "region_id": "rgn-0001",
  "template_name": "login_btn",
  "score": 0.93,
  "match_rect": { "x": 102, "y": 202, "w": 76, "h": 36 },
  "crop_file": "crops/rgn-0001-f005-tmpl.png"
}
```

---

### `state_snapshot`

Full modeled application state at a checkpoint.

```json
{
  "type": "state_snapshot",
  "frame": 10,
  "ts": "2026-01-15T14:23:00.333Z",
  "state": {
    "screen": "Login",
    "field_focused": "username",
    "error_visible": false
  }
}
```

State values are strings, booleans, or numbers. No nested objects.

---

### `transition`

Modeled state change. Records the fields that changed.

```json
{
  "type": "transition",
  "frame": 11,
  "ts": "2026-01-15T14:23:00.366Z",
  "from": { "screen": "Login" },
  "to":   { "screen": "Dashboard" },
  "trigger_region": "rgn-0001"
}
```

---

### `divergence`

Detected mismatch between expected and observed modeled state.

```json
{
  "type": "divergence",
  "frame": 12,
  "ts": "2026-01-15T14:23:00.400Z",
  "severity": "error",
  "expected": { "screen": "Dashboard" },
  "observed": { "screen": "Login" },
  "message": "Expected screen=Dashboard after login button click, got screen=Login",
  "region_id": "rgn-0001"
}
```

`severity` values: `"warning"`, `"error"`, `"fatal"`.

---

### `missing_frame`

Gap in the recording. The producer signals that frame N was not captured.

```json
{
  "type": "missing_frame",
  "frame": 7,
  "ts": "2026-01-15T14:23:00.233Z",
  "reason": "capture_drop",
  "patch": {
    "source": "duplicate_previous",
    "ref_frame": 6
  }
}
```

`patch.source` values: `"duplicate_previous"`, `"interpolated"`,
`"game_export"`, `"none"`.
When `source` is `"none"`, replay must treat the frame as unavailable.

---

### `warning`

Non-fatal diagnostic from the producer or replay engine.

```json
{
  "type": "warning",
  "frame": 3,
  "ts": "2026-01-15T14:23:00.100Z",
  "code": "LOW_CONFIDENCE_OCR",
  "message": "OCR confidence 0.41 below threshold 0.60 for rgn-0002"
}
```

---

### `error`

Fatal or near-fatal diagnostic.

```json
{
  "type": "error",
  "frame": 20,
  "ts": "2026-01-15T14:23:00.666Z",
  "code": "REGION_LOST",
  "message": "Region rgn-0005 lost after 3 consecutive missing frames"
}
```

---

### `replay_checkpoint`

A named point in the event stream where replay can resume.

```json
{
  "type": "replay_checkpoint",
  "frame": 50,
  "ts": "2026-01-15T14:23:01.666Z",
  "name": "after_login",
  "state": { "screen": "Dashboard" }
}
```

---

## Design Requirements Coverage

| Requirement | Mechanism |
|-------------|-----------|
| Raw frames + change events only | `frame` + `change` events; no other types required |
| User annotations added later | `annotation` events appended after initial recording |
| Generated/game exact ground truth | `source_type: "game_export"`, `state_snapshot`, `transition` |
| Game imports GT and patches missing | `missing_frame` + `patch.source: "game_export"` |
| Async OCR/template results | `trigger_frame` field on `ocr`/`template` events |
| Region identity across movement | `region` events with `action: "moved"/"resized"` + `fingerprint` |
| Large image data external | `image_file`, `crop_file`, `fingerprint_file` — all relative paths |
| Backward-compatible evolution | `"schema": 1` integer; new optional fields are ignored by old parsers |

---

## Compact Examples

### Example 1: Minimal Session Header And Frame Event

```json
{
  "schema": 1,
  "producer": { "name": "VisualStateModel", "version": "0.1.0",
                "created_at": "2026-01-15T14:23:00.000Z" },
  "session": {
    "id": "demo-001",
    "source_type": "desktop_recording",
    "frame_width": 1920, "frame_height": 1080,
    "started_at": "2026-01-15T14:23:00.000Z",
    "image_dir": "frames/", "crop_dir": "crops/"
  },
  "events": [
    { "type": "frame", "frame": 0, "ts": "2026-01-15T14:23:00.000Z",
      "image_file": "frames/000000.png" },
    { "type": "frame", "frame": 1, "ts": "2026-01-15T14:23:00.033Z",
      "image_file": "frames/000001.png" },
    { "type": "change", "frame": 1, "ts": "2026-01-15T14:23:00.033Z",
      "regions": [{ "x": 100, "y": 200, "w": 80, "h": 40, "score": 0.91 }] }
  ]
}
```

### Example 2: Changed Region With Hierarchy And Annotation

```json
{
  "events": [
    { "type": "region", "frame": 1, "ts": "2026-01-15T14:23:00.033Z",
      "region_id": "rgn-0001", "action": "created",
      "rect": { "x": 100, "y": 200, "w": 80, "h": 40 },
      "parent_id": "rgn-0000",
      "fingerprint": "sha1:a3f8c1d2e" },
    { "type": "annotation", "frame": 5, "ts": "2026-01-15T14:23:00.166Z",
      "region_id": "rgn-0001",
      "label": "LoginButton",
      "properties": { "ocr_enabled": true },
      "author": "human" },
    { "type": "ocr", "frame": 5, "ts": "2026-01-15T14:23:00.500Z",
      "region_id": "rgn-0001", "trigger_frame": 5,
      "text": "Log In", "confidence": 0.97, "engine": "tesseract" }
  ]
}
```

### Example 3: Divergence Record

```json
{
  "events": [
    { "type": "state_snapshot", "frame": 10, "ts": "2026-01-15T14:23:00.333Z",
      "state": { "screen": "Login", "field_focused": "username" } },
    { "type": "transition", "frame": 11, "ts": "2026-01-15T14:23:00.366Z",
      "from": { "screen": "Login" }, "to": { "screen": "Dashboard" },
      "trigger_region": "rgn-0001" },
    { "type": "divergence", "frame": 12, "ts": "2026-01-15T14:23:00.400Z",
      "severity": "error",
      "expected": { "screen": "Dashboard" },
      "observed": { "screen": "Login" },
      "message": "Expected screen=Dashboard after login click, got screen=Login" }
  ]
}
```

---

## Schema Evolution

- Adding new optional top-level fields to `producer` or `session`: backward-compatible, no schema bump.
- Adding a new event `type`: backward-compatible; parsers must skip unknown types.
- Adding optional fields to existing event types: backward-compatible.
- Renaming or removing fields: requires schema bump.
- Changing the meaning of existing fields: requires schema bump.

---

## File Naming Convention

| File | Convention |
|------|-----------|
| Ground truth JSON | `<session-id>.vsm.json` |
| Frame images | `frames/<NNNNNN>.png` (zero-padded 6 digits) |
| Crop images | `crops/<region-id>-f<NNNNNN>-<suffix>.png` |
| Fingerprint buffer | `crops/<region-id>-f<NNNNNN>.fp.bin` |
| Template images | `templates/<name>.png` |
