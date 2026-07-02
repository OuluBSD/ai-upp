# VisualStateModel — Image Sequence Import

## Overview

`VsmImageSequenceImporter` converts a directory of numbered `.vsm` frame files
into a VisualStateModel session directory, ready for replay through
`VsmObservationPipeline` or `VsmSessionStoreSource`.

---

## Supported Format

| Format | Headless | Notes |
|---|---|---|
| `.vsm` (VSM1 binary) | Yes | Primary format |
| JPEG / PNG | No | Requires Draw/GUI — deferred follow-up |

---

## API

### Options

```cpp
VsmImageSequenceImportOptions opts;
opts.source_dir = "/path/to/frames";   // directory with .vsm files
opts.output_dir = "/path/to/session";  // destination session directory
opts.session_id = "my-session-001";    // leave empty to auto-generate
opts.fps        = 30;                  // used for timestamp assignment
```

### Import

```cpp
VsmImageSequenceImporter importer;
importer.SetLog(&log);
VsmImageSequenceImportResult res = importer.Import(opts);
if(!res.success) { /* handle error */ }
```

### Result

```cpp
res.success         // true if import completed without fatal error
res.frames_scanned  // number of .vsm files found
res.frames_imported // number of frames written to output session
res.frames_skipped  // number of files skipped (unreadable / dimension mismatch)
res.session_id      // final session ID
res.output_dir      // final output path
res.warnings        // per-file warnings (filename + message)
```

---

## Sorting

Files are sorted by their leading numeric prefix (`00000003.vsm` < `00000010.vsm`).
Non-numeric filenames are sorted to the front (prefix = 0).

---

## Reference Tool

`reference/VisualStateImportSequence/` is a buildable CLI that:

1. Generates 4 synthetic `.vsm` frames in a temp source directory.
2. Imports them into a new session.
3. Verifies the imported session opens via `VsmSessionStoreSource`.
4. Reads back all frames and confirms the count.

```sh
bin/build.exe -m 7 -j12 VisualStateImportSequence
bin\VisualStateImportSequence.exe
```

Expected:
```
All import checks passed.
```

---

## Opening the Imported Session

After import, open with `VsmSessionStoreSource`:

```cpp
VsmSessionStoreSource src;
src.Open(res.output_dir);
VsmImageBuffer frame; int64 ts_ms;
while(src.ReadFrame(frame, ts_ms)) { /* process */ }
```

Or run the observation pipeline directly:

```cpp
VsmObservationPipeline pipe;
// ... configure pipeline ...
VsmPipelineRunSummary sum = pipe.RunFromSource(src);
```

---

## Limitations

- Only `.vsm` files are imported. JPEG/PNG requires `Draw` (GUI) dependency.
- All frames must have the same dimensions as the first readable file.
- No timestamp extraction from filenames; timestamps are assigned as
  `frame_index / fps` seconds.
