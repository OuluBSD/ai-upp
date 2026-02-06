# Task 1301 - Keyframe Drag & Move

## Goal
Allow direct manipulation of keyframes in the timeline.

## Scope
- Drag keyframe to move in time.
- Shift-drag snaps to nearest second or grid.
- Dragging a range moves all keyframes inside the range.
- Collision handling (merge or offset policy).

## Implementation Notes
- Use a lightweight hit-test on keyframe markers.
- Update model only on drag end (preview while dragging).

## Success Criteria
- Keyframes can be moved without jitter.
- Snapping is predictable and optional.
