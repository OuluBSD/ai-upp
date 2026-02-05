# Task 0201 - DisplayObject Proxy

## Goal
Expose ActionScript-style DisplayObject semantics for Scene3D nodes.

## Scope
- Proxy object with `.x/.y/.z`, `.rotation`, `.scale`, `.visible`.
- Methods: `addChild`, `remove`, `find`.
- Backed by GeomTransform + visibility fields.

## Success Criteria
- Script can read/write DisplayObject properties and see updates in viewport.

## Status
- Done (2026-02-05)
  - Added DisplayObject proxy with x/y/z, rotation, scale, visible, and child ops.
