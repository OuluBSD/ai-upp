# Task 0801 - Weight Painting

## Goal
Support weighting polygons/vertices to skeleton bones.

## Scope
- Weight painting UI (brush with radius/strength).
- Assign weights to selected bones.
- Normalize weights per vertex.

## Implementation Notes
- Store weights in mesh data (per-vertex influences).
- Provide visual heatmap for weights.

## Success Criteria
- Users can paint weights and see live heatmap updates.
- Vertex weights persist across save/load.

## Status
- Done

## Notes
- Added skin weight component and weight paint mode using bone selection.
- Viewport shows weight colors for the active bone.
