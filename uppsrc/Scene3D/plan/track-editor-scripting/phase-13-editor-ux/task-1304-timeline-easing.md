# Task 1304 - Keyframe Easing & Curves

## Goal
Add interpolation controls for keyframes.

## Scope
- Per-keyframe easing: linear, step, smooth, bezier.
- Tangent handles for position/orientation curves.
- Optional curve editor dock.

## Implementation Notes
- Store easing metadata in keyframe structs.
- Start with position only, expand to orientation.

## Success Criteria
- Easing controls are visible and affect playback.
- Curves persist across save/load.
