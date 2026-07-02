# VisualStateModel — Capture Sink

## Overview

`VsmCaptureSink` records frames from any `VsmFrameSource` into a new
VisualStateModel session directory. The recorded session is a normal VSM
session that can be opened by `VsmSessionStoreSource` and run through
`VsmObservationPipeline`.

---

## API

### Options

```cpp
VsmCaptureSinkOptions opts;
opts.output_dir = "/path/to/output/session";
opts.session_id = "my-capture-001";   // leave empty to auto-generate
opts.max_frames = -1;                 // -1 = unlimited; positive = cap
```

### Recording

```cpp
VsmCaptureSink sink;
sink.SetLog(&log);
VsmCaptureSummary summary = sink.Record(src, opts);
```

`src` is any object implementing `VsmFrameSource` — typically
`VsmSessionStoreSource` (replay) or a future live adapter.

### Summary

```cpp
summary.success          // true if recording completed
summary.frames_recorded  // frames written to output session
summary.frames_dropped   // frames that failed to write
summary.error_count      // total write errors
summary.session_id       // final session ID
summary.output_dir       // output path
summary.source_info      // source.GetSourceInfo() value at end
```

---

## Replaying a Recorded Session

```cpp
VsmSessionStoreSource replay;
replay.Open(summary.output_dir);

VsmImageBuffer frame; int64 ts_ms;
while(replay.ReadFrame(frame, ts_ms)) { /* process */ }
```

Or run the pipeline directly:

```cpp
VsmObservationPipeline pipe;
// configure rules...
VsmPipelineRunSummary run = pipe.RunFromSource(replay);
```

---

## Reference Tool

`reference/VisualStateRecordSession/` demonstrates:

1. Creates a 3-frame source session.
2. Opens it with `VsmSessionStoreSource`.
3. Records it into a new session with `VsmCaptureSink`.
4. Verifies the recorded session with `VsmSessionStoreSource`.
5. Runs the observation pipeline on the recorded session.

```sh
bin/build.exe -m 7 -j12 VisualStateRecordSession
bin\VisualStateRecordSession.exe
```

Expected:
```
All record/replay checks passed.
```

---

## Design Notes

- No threading — frame pull is synchronous.
- Frame timestamps from the source are preserved where nonzero.
- Dropped frames (write errors) are counted but do not abort the recording.
- `GetStore()` exposes the output `VsmSessionStore` for pipeline integration.
- `detect_changes` option is reserved for future use (change detection
  while recording without a separate replay pass).
