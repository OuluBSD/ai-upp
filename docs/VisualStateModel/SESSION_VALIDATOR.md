# VisualStateModel — Session Validator

## Overview

`VsmSessionValidator` checks a session directory for structural integrity,
validating that the manifest is well-formed and all referenced asset files
exist on disk.

---

## API

### Validator

```cpp
VsmSessionValidator validator;
validator.SetLog(&log);
VsmValidationResult result = validator.Validate("/path/to/session");
```

### Result

```cpp
result.ok                // true if validation passed (no errors)
result.frames_checked    // number of frames in manifest
result.crops_checked     // number of crops in manifest
result.issues            // Vector<VsmValidationIssue>
```

### Issues

Each `VsmValidationIssue` has:

```cpp
issue.severity  // "error" or "warning"
issue.message   // human-readable description
```

---

## Validation Checks

### 1. Manifest File Exists and Parses

- **Check**: `manifest.json` is present and readable.
- **Check**: JSON parses successfully into `VsmSessionManifest`.
- **Severity**: Error if missing or unparseable.
- **Stops further checks**: Yes (no frames/crops checked).

### 2. Frame Asset Files Exist

- **Check**: For each frame entry in the manifest, the file at
  `session_dir/relative_path` exists on disk.
- **Severity**: Error per missing file.
- **Continues scanning**: Yes (other files are checked even if one is missing).

### 3. Crop Asset Files Exist

- **Check**: For each crop entry in the manifest, the file at
  `session_dir/relative_path` exists on disk.
- **Severity**: Error per missing file.
- **Continues scanning**: Yes.

### 4. Frame Indexes Are Unique

- **Check**: No two frame entries have the same `frame_index`.
- **Severity**: Warning per duplicate.

### 5. Session ID Is Non-Empty

- **Check**: `session_id` field in the manifest is not empty.
- **Severity**: Warning if empty.

---

## Result Interpretation

```cpp
if(result.ok) {
    // No errors; session is structurally valid.
    // May still have warnings.
} else {
    // One or more errors; session is broken.
    // Check `result.issues` for details.
}

for(const VsmValidationIssue& issue : result.issues) {
    if(issue.severity == "error") {
        // Critical: something is missing or malformed.
    } else if(issue.severity == "warning") {
        // Suspicious but not fatal.
    }
}
```

---

## Non-Goals

This validator **does not** check:

- Annotation layers, divergences.json, or comparison_result.json content.
- Asset file formats (only existence).
- Image dimensions or consistency.
- Consistency between frames and crops (e.g., crop regions within frame bounds).

---

## Reference Tool

`reference/VisualStateSessionValidate/` demonstrates:

1. Creates a valid session with 3 frames and 1 crop.
2. Creates a broken session and deletes a frame asset file.
3. Validates both sessions.
4. Asserts the valid session passes and the broken session reports errors.

```sh
bin/build.exe -m 7 -j12 VisualStateSessionValidate
bin\VisualStateSessionValidate.exe
```

Expected output:

```
=== VisualStateModel Session Validator Demo ===
...
OK: Valid session passes validation
OK: Broken session correctly reports missing frame asset error
All validation checks passed
```

---

## Design Notes

- Validation is **read-only** — no manifest repairs or file modifications.
- Logging is optional via `SetLog()`.
- JSON parsing uses existing `VsmSessionManifest` structure from
  `SessionStorage.h` — no separate manifest schema.
- Missing manifest is fatal; missing individual files are errors but do not
  stop the scan.
