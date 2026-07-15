# VideoRegressionAssert Agent Notes

Scope: this file applies to `reference/VideoRegressionAssert`.

## Purpose

Validate live video regression artifacts with explicit pass/fail stdout
diagnostics for agents and scripted runs.

## Rules

- Build with `bin\build.exe -m MSVS22x64 .\reference\VideoRegressionAssert\VideoRegressionAssert.upp`.
- Do not use `script/build.py`.
- Keep this tool assertion-only; recording/tracking/OCR belong to their own tools.
- Source files include `VideoRegressionAssert.h` first.

