# VideoFrameWindowDetector Agent Notes

Scope: this file applies to `reference/VideoFrameWindowDetector`.

## Purpose

Headless reference tool for detecting desktop windows from captured
VideoServer frames and dumping crops/overlays for VisualStateModel M11.

## Rules

- Build with `bin\build.exe -m MSVS22x64 .\reference\VideoFrameWindowDetector\VideoFrameWindowDetector.upp`.
- Do not use `script/build.py`.
- Keep heuristics explicit and diagnostic-heavy; this is expected to evolve
  through regression failures.
- Source files include `VideoFrameWindowDetector.h` first.

