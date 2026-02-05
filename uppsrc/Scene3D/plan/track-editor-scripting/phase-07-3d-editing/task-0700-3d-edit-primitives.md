# Task 0700 - 3D Editor: Edit/Draw/Erase Primitives

## Goal
Add interactive edit tools for points, lines, faces, and polygons in the 3D editor.

## Scope
- Point tool: create, select, move, delete points.
- Line tool: create segments, join existing points, edit endpoints.
- Face/Polygon tool: create faces from selected points/edges, edit vertices.
- Erase tool: delete selected primitives.

## Implementation Notes
- Use a single editable mesh/geometry component in Scene3D/Core for primitive storage.
- Provide top-level tool state machine: select, draw, erase, join.
- Integrate with timeline/keyframe and read/write toggles.

## Success Criteria
- User can create, edit, and delete points/lines/polygons in viewport.
- Edits are undoable and persist in save/load.

## Status
- Pending
