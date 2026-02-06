# Task 0907: Props Playback Sync

## Goals
- Properties panel values reflect the timeline playback state.
- When the time cursor changes (scrub/play), props show the evaluated values.

## Requirements
- On timeline cursor change, update props with the evaluated value at that frame.
- Support interpolation (lerp/easing) consistent with runtime playback.
- If no keyframes exist for a property, keep the current static value.
- Avoid infinite loops: prop refresh should not re-trigger keyframe writes.

## Notes
- Start with Transform (position/orientation), then extend to other animated components.
- Should work for scene timelines and object timelines.
