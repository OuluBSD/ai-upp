# VideoTableQuality Agent Notes

Scope: this file applies to `reference/VideoTableQuality`.

## Purpose

Assess whether detected poker table windows contain usable table content, as
opposed to being obscured by lobby windows, login dialogs, or other overlays.

## Rules

- Build with `bin\build.exe -m MSVS22x64 .\reference\VideoTableQuality\VideoTableQuality.upp`.
- Do not use `script/build.py`.
- Keep this as a post-processing tool over tracker/OCR artifacts.
- Source files include `VideoTableQuality.h` first.

