# VideoChangedRegionReview Agent Notes

This package is a headless review helper. It reads a changed-region manifest,
groups crops by a deterministic normalized 8x8 RGB signature, and writes JSON
and a standalone HTML gallery. Keep the package limited to Core/Draw/plugin-jpg
and include the package main header first in source files.
