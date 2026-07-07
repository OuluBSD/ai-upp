# Hearts Human Card Slide Animation

**Status**: COMPLETE
**Date**: 2026-07-07

## Goal

Replace the remaining horizontal flip effect in the human hand with real positional easing when cards shift after a play or selection change.

## Notes

- The motion must stay generic to `Ctrl`/`ImageCtrl` behavior, not card-specific special casing.
- The hand should ease in position; it should not rotate or mirror during the shift.
- Verify with build output and the existing Hearts CLI diagnostics.

## Result

- `game/Hearts/HeartsCtrl.cpp` now drives the human hand with `SetRect(...)` easing only, and the image controls no longer use transition flips.
- `python script/build.py -j2 game/Hearts` succeeds.
- `game/Hearts/main.cpp` now starts a game before `--dump-layout`, so the dump reflects the actual playing layout.
- The runtime form load now succeeds; the crash was caused by malformed XML in `game/Hearts/Hearts.form` near `PassButton`, which I repaired.
- Layout verification via `bin/Hearts.exe --dump-layout` now shows the human row bottom-centered and the north row top-centered.
