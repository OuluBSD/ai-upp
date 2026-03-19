# Task 0403 - Modify Selection Actions

## Status
- Done (2026-02-19)

## Goal
Add modify-selection actions from the Scene Graph Explorer context menu.

## Scope
- Implement mesh selection ops (UV planar, flip faces, center pivot, normalize normals, vertex colors, recalc normals/tangents).
- Clone as static animated mesh.
- Distribute over terrain and bake textures into one.
- Mesh reload from disk.
- Behaviors copy/paste stubs where needed.

## Acceptance
- Commands apply to current selection without breaking existing tools.
- Disabled items remain visible and gated until implemented.

## Current State
- Modify Selection submenu now executes concrete mesh operations for model selections:
  - planar UV projection
  - face flip
  - center pivot
  - freeze scale + normalize normals
  - material color reset/set handlers for vertex-color parity commands
  - recalculate normals/tangents
- Mesh reload from disk now resolves asset paths and reloads through `ModelLoader`.
- `Clone as static animated mesh` creates a clone and seeds a static mesh keyframe.
- Behavior copy/paste now copies `GeomScript` entries between compatible nodes.
- Unsupported heavy operations (terrain distribution, texture bake-to-one) remain visible but disabled/gated with explicit messaging.

## Remaining
- None for this task scope.
