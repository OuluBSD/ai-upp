# VideoEventAuditReport Agent Notes

Scope: this file applies to `reference/VideoEventAuditReport`.

## Purpose

Generate a compact markdown audit report from a `VideoWindowTracker` output
directory so agents and users can inspect event candidates visually.

## Rules

- Build with `bin\build.exe -m MSVS22x64 .\reference\VideoEventAuditReport\VideoEventAuditReport.upp`.
- Do not use `script/build.py`.
- Keep this as a thin reporting tool; event detection belongs in `VideoWindowTracker`.
- Source files include `VideoEventAuditReport.h` first.

