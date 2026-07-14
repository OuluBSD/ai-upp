# VideoWindowTracker Agent Notes

Scope: this file applies to `reference/VideoWindowTracker`.

## Purpose

Headless reference tool for tracking poker table windows across recorded
VideoServer frame sequences and dumping per-window changed-region diagnostics.

## Rules

- Build with `bin\build.exe -m MSVS22x64 .\reference\VideoWindowTracker\VideoWindowTracker.upp`.
- Do not use `script/build.py`.
- Keep output artifact-heavy: crops, overlays, and JSON are part of the
  verification contract.
- Source files include `VideoWindowTracker.h` first.

