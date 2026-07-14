# VideoServerFrameRecorder Agent Notes

Scope: this file applies to `reference/VideoServerFrameRecorder`.

## Purpose

Small headless reference tool for pulling frames from an already-running
`VideoServer` TCP endpoint and saving regression frame artifacts.

## Rules

- Build with `bin\build.exe -m MSVS22x64 .\reference\VideoServerFrameRecorder\VideoServerFrameRecorder.upp`.
- Do not use `script/build.py`.
- Keep the tool focused on capture/diagnostics; move shared production logic to
  `uppsrc` only after the workflow stabilizes.
- Source files include `VideoServerFrameRecorder.h` first.

