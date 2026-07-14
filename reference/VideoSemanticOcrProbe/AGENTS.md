# VideoSemanticOcrProbe Agent Notes

Scope: this file applies to `reference/VideoSemanticOcrProbe`.

## Purpose

Run a repeatable OCR smoke probe over selected `VideoWindowTracker` semantic
crops using the installed Tesseract executable.

## Rules

- Build with `bin\build.exe -m MSVS22x64 .\reference\VideoSemanticOcrProbe\VideoSemanticOcrProbe.upp`.
- Do not use `script/build.py`.
- Keep this as diagnostics only; poker-state interpretation belongs elsewhere.
- Source files include `VideoSemanticOcrProbe.h` first.

