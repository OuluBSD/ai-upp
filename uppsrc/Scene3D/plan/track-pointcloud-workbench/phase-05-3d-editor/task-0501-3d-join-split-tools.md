# Task 0501 - 3D Editor: Join/Split Tools

## Goal
Enable joining and splitting of primitives (points, edges, faces) for mesh editing.

## Scope
- Join selected points into a line/edge.
- Join edges into faces/polygons.
- Split edges/faces at a picked location.
- Merge points (weld) within a threshold.

## Implementation Notes
- Provide topology-safe operations (avoid duplicate edges).
- Display preview of join/split operation before commit.

## Success Criteria
- Join/split operations work on basic meshes without corrupting topology.
- Weld threshold behaves predictably.

## Status
- Pending
