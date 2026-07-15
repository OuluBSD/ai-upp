# VideoEventCorrelator Agent Notes

Scope: this file applies to `reference/VideoEventCorrelator`.

## Purpose

Convert raw frame-local video event candidates into coarser correlated
transitions for audit and agent consumption.

## Rules

- Build with `bin\build.exe -m MSVS22x64 .\reference\VideoEventCorrelator\VideoEventCorrelator.upp`.
- Do not use `script/build.py`.
- Keep this as a post-processing tool; raw detection belongs to `VideoWindowTracker`.
- Source files include `VideoEventCorrelator.h` first.

