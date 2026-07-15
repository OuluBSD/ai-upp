# VideoRecorder Agent Notes

Scope: this file applies to `uppsrc/VideoRecorder`.

## Purpose

Record a running `VideoServer` stream into reusable regression video files and
sidecar manifests.

## Build

- Use `bin\build.exe -m MSVS22x64 .\uppsrc\VideoRecorder\VideoRecorder.upp`.
- Do not use `script/build.py`.

## Notes

- The initial implementation intentionally records JPEG frames first and then
  invokes `ffmpeg` as an external encoder. This keeps the workflow debuggable
  before moving to direct libav/FFmpeg APIs.
- Keep stdout diagnostics explicit; long captures must show progress.
- Source files include `VideoRecorder.h` first.

