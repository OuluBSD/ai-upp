# VideoLiveRegressionRunner Agent Notes

Scope: this file applies to `reference/VideoLiveRegressionRunner`.

## Purpose

Small reference wrapper for running the current live VideoServer regression
pipeline: record frames, then run the window tracker on the recording.

## Rules

- Build with `bin\build.exe -m MSVS22x64 .\reference\VideoLiveRegressionRunner\VideoLiveRegressionRunner.upp`.
- Do not use `script/build.py`.
- This is a reference orchestration tool; keep it thin and diagnostic-heavy.
- Source files include `VideoLiveRegressionRunner.h` first.

