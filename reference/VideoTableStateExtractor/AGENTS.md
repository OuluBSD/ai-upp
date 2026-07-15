# VideoTableStateExtractor Agent Notes

Scope: this file applies to `reference/VideoTableStateExtractor`.

## Purpose

Build the first compact per-table game-state JSON from existing live video
pipeline artifacts.

## Rules

- Build with `bin\build.exe -m MSVS22x64 .\reference\VideoTableStateExtractor\VideoTableStateExtractor.upp`.
- Do not use `script/build.py`.
- Keep this tool headless and diagnostic-heavy.
- This is an early state extractor: prefer explicit confidence/reason fields
  over silent poker-logic guesses.
- Source files include `VideoTableStateExtractor.h` first.
