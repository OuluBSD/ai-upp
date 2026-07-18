# VsmVideoServerFrameSourceSmoke Agent Notes

Scope: this file applies to `reference/VsmVideoServerFrameSourceSmoke`.

## Purpose

Task 0279's throwaway, real-network verification tool for
`uppsrc/VisualStateModel/VideoServerFrameSource.h/.cpp`
(`VsmVideoServerFrameSource`). Requires an already-running `VideoServer`
(see `uppsrc/VideoServer`) to connect to — it is not a self-contained
CHECK-harness like `upptst/VisualStateModelTests`, it needs a real live TCP
peer and real wall-clock time, so it lives here as a diagnostic tool in the
same spirit as `reference/VideoServerFrameRecorder`.

## Usage

```
bin\VideoServer.exe --source video --video bin\video_record_25min_20260716_203356.mp4 --fps 8 &
bin\build.exe -m MSVS22x64 .\reference\VsmVideoServerFrameSourceSmoke\VsmVideoServerFrameSourceSmoke.upp
bin\VsmVideoServerFrameSourceSmoke.exe --host 127.0.0.1 --port 8082 --seconds 25
```

Reports, to stdout: real frame count, width/height, count of distinct vs.
identical consecutive-frame hashes (proves the stream is actually changing,
not stuck/cached), measured inter-frame timing, then exercises
Close()/reopen and reads a few more frames to confirm clean reconnect.
Saves a few sample JPEGs (first/mid/last) under `--out` (default
`tmp/vsm_video_server_frame_source_smoke_<timestamp>/`) for visual
spot-checking. Exits 1 on any unrecoverable failure.

## Rules

- Build with `bin\build.exe -m MSVS22x64 .\reference\VsmVideoServerFrameSourceSmoke\VsmVideoServerFrameSourceSmoke.upp`.
- Do not use `script/build.py`.
- Keep this tool focused on this one class's real-network verification; it
  is not production code and should not be depended on by anything else.
