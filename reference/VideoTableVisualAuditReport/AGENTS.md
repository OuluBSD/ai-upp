# VideoTableVisualAuditReport Agent Notes

Scope: this file applies to `reference/VideoTableVisualAuditReport`.

## Purpose

Generate a compact HTML visual audit report from a `VideoWindowTracker` output
directory so agents and users can inspect table-state parser crops and JSON
summaries side-by-side.

## Rules

- Build with `bin\build.exe -m MSVS22x64 .\reference\VideoTableVisualAuditReport\VideoTableVisualAuditReport.upp`.
- Do not use `script/build.py`.
- Keep this as a thin reporting tool; parsing/detection belongs in tracker and extractor packages.
- Source files include `VideoTableVisualAuditReport.h` first.
