# Task 0702 - 3D Editor: Sculpt From Sphere

## Goal
Add a sculpting tool that deforms geometry using a spherical brush.

## Scope
- Sphere brush with radius/strength controls.
- Modes: push/pull, smooth, flatten.
- Apply to meshes or pointcloud surface samples.

## Implementation Notes
- Use falloff curve (linear or gaussian) over radius.
- Support real-time preview while dragging.
- Respect read/write toggles and keyframe recording.

## Success Criteria
- Sculpt tool modifies geometry smoothly with live feedback.
- Radius/strength controls behave consistently.

## Status
- Done

## Notes
- Added sculpt mode with add/subtract, radius, and strength controls.
- Sculpt uses ray hit + falloff on editable mesh points.
