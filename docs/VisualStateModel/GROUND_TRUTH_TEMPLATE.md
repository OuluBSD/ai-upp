# VisualStateModel — Ground Truth Template Generator

## Overview

`VsmGroundTruthTemplateGenerator` produces a skeleton `VsmGroundTruthSession`
JSON file from an existing recorded or imported session. This is useful for
authoring ground truth: instead of hand-writing JSON from scratch, you start
with a valid template that has the correct session metadata, frame count, and
a human-friendly example divergence entry showing the expected structure.

The template contains:
- The session's original `session_id` and metadata (frame dimensions, source type, etc.)
- A frame reference for each recorded frame
- An empty `divergences` array, plus exactly one example entry with the message
  `"EXAMPLE - replace or remove"` so a human opening the file immediately
  understands what to fill in

---

## API

### Options

```cpp
struct VsmGroundTruthTemplateOptions {
    String session_dir;   // existing recorded/imported session to read
    String output_path;   // where to write the template JSON
};
```

### Result

```cpp
struct VsmGroundTruthTemplateResult {
    bool   success = false;
    int    frame_count = 0;
    String session_id;
    String output_path;
};
```

### Generator

```cpp
VsmGroundTruthTemplateGenerator generator;
generator.SetLog(&log);
VsmGroundTruthTemplateResult result = generator.Generate(opts);

if(result.success) {
    Cout() << "Generated template with " << result.frame_count << " frames\n";
}
```

---

## Usage

### Typical Workflow

1. Record or import a session (creates `session/` directory with `manifest.json`).
2. Run the template generator:
   ```cpp
   VsmGroundTruthTemplateOptions opts;
   opts.session_dir = "session/";
   opts.output_path = "ground_truth.json";
   
   VsmGroundTruthTemplateGenerator generator;
   generator.Generate(opts);
   ```
3. Open `ground_truth.json` in your editor.
4. Replace the example divergence entry(ies) with actual ground truth expectations.
5. Load the file in your pipeline or test suite via `VsmGroundTruthLoader`.

### Example Output

The generated `ground_truth.json` will look like:

```json
{
  "schema": 1,
  "session": {
    "id": "recorded-session-001",
    "source_type": "recorded",
    "frame_width": 1920,
    "frame_height": 1080,
    "started_at": "2026-07-03T12:34:56.000Z",
    "ended_at": "2026-07-03T12:34:56.000Z",
    "image_dir": "frames/",
    "crop_dir": "crops/"
  },
  "events": [
    {
      "type": "frame",
      "frame": 0,
      "ts": "2026-07-03T12:34:56.000Z",
      "image_file": "frames/00000000.vsm"
    },
    {
      "type": "frame",
      "frame": 1,
      "ts": "2026-07-03T12:34:56.000Z",
      "image_file": "frames/00000001.vsm"
    },
    ...
    {
      "type": "divergence",
      "frame": 0,
      "ts": "2026-07-03T12:34:56.000Z",
      "severity": "warning",
      "message": "EXAMPLE - replace or remove",
      "region_id": "",
      "expected": {},
      "observed": {}
    }
  ]
}
```

Human workflow:
1. Delete or replace the example divergence entry.
2. Add real divergences based on what the pipeline should expect at each frame.
3. Use this file as the source of truth for regression testing.

---

## Non-Goals

- Does **not** infer expected state from OCR/pipeline output — this generates
  structural skeleton only, not a guess at what the ground truth should say.
- Does **not** add interactive/wizard CLI prompt flow — it is a single headless
  function.
- Does **not** validate the session directory — assumes `manifest.json` exists
  and is readable.

---

## Reference Tool

`reference/VisualStateGroundTruthInit/` demonstrates:

1. Creates a synthetic 3-frame session directory in temp.
2. Runs `VsmGroundTruthTemplateGenerator::Generate()` on it.
3. Loads the generated JSON back to verify it parses correctly.
4. Asserts `frame_count == 3`.
5. Asserts exactly one example divergence entry is present.
6. Prints the result.

```sh
bin/build.exe -m 7 -j12 VisualStateGroundTruthInit
bin\VisualStateGroundTruthInit.exe
```

Expected output:

```
=== VisualStateModel Ground Truth Template Generator Demo ===

Creating synthetic 3-frame session: <temp path>
  Created session with 3 frames

Generating template from session...
  Success: gt-template-demo
  Frame count: 3
  Output path: <temp path>

Loading and verifying generated template...
OK: frame_count == 3
OK: exactly 1 example divergence
OK: example divergence message is correct

--- Generated Template ---
Session ID: gt-template-demo
Frame count: 3
Divergences: 1
  [0] Frame 0 (warning): EXAMPLE - replace or remove

--- Acceptance Checks ---
OK: session_id == "gt-template-demo"
OK: frame dimensions 48x48

All ground truth template checks passed.
```

---

## Design Notes

- Generation is **read-only** — no files in the source session are modified.
- Logging is optional via `SetLog()`.
- The example divergence is always at frame 0 with severity "warning".
- All frame references come directly from the session manifest.
- The template preserves the original session metadata (session_id, frame
  dimensions, source type, created_at timestamp).

---

## Integration

Use this tool as part of a ground-truth authoring workflow:

1. **Record or import** a scenario (e.g., via `VisualStateRecordSession`).
2. **Generate a template** from that session.
3. **Author ground truth** by editing the template JSON (delete/replace example,
   add real divergences).
4. **Test** by loading the ground truth file and running divergence detection
   against the replay or pipeline.
5. **Commit** the ground truth file to version control as your regression test
   baseline.
