# Task 0906: Transform Sub-Keyframes + Props Keyframe Controls

## Goals
- Split transform keyframes into sub-keyframes for position and orientation.
- Show position/orientation sub-rows under Transform in the timeline hierarchy.
- Add keyframe controls in the Properties panel for transform position/orientation.

## Requirements
- Timeline rows:
  - Object row expands to Transform row.
  - Transform row shows Position and Orientation sub-rows.
  - Each sub-row shows only its own keypoints.
- Props panel:
  - Add a rightmost keyframe column with 3 buttons: prev, keyframe toggle, next.
  - Prev/next always works; if no keypoints exist, jump to first/last frame.
  - Keyframe toggle sets/clears keypoint for that property only.
- Runtime:
  - Position and orientation interpolation must respect property flags.
  - If only one keypoint exists for a property, hold its value.

## Notes
- Maintain compatibility with existing timelines (default flags = on).
- Keep scene/object timeline row union behavior.
