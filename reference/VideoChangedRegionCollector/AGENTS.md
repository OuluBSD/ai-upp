# VideoChangedRegionCollector Agent Notes

Scope: this file applies to `reference/VideoChangedRegionCollector`.

This is a headless collector for the changed-region-first video workflow. It
reads `VideoWindowTracker` summaries, preserves every occurrence, and creates
a reviewable unique-region gallery. It must not infer cards from fixed seat or
board crops.

Build with `bin\\build.exe -m MSVS22x64 .\\reference\\VideoChangedRegionCollector\\VideoChangedRegionCollector.upp`.
Source files include `VideoChangedRegionCollector.h` first.
