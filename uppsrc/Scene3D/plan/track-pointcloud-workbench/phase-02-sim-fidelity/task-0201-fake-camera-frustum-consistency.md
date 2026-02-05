# Task 0201 - Fake Camera Frustum Consistency

## Goal
Make the fake HMD camera orientation and frustum math match the rendered camera orientation.

## Scope
- Align simulated camera orientation with render camera orientation (no inverted Z).
- Validate frustum test with visual overlay (controllers in view should be inside frustum).

## Implementation Notes
- Compare math-space axes with render-space axes and normalize to a single convention.
- Add a simple frustum overlay/visual test for the fake camera.

## Success Criteria
- Controllers placed via "Simulate HMD Observation" are inside the fake camera frustum.
- Frustum math matches what is displayed in the viewport.

## Status
- Pending
