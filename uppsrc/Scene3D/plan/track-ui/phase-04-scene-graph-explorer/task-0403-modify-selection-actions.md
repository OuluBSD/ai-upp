# Task 0403 - Modify Selection Actions

## Status
- Partial / Open (updated 2026-02-17)

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
- Modify Selection submenu structure is present in the context menu and mirrors the spec entries.
- Disabled/gated items remain visible where intended.

## Remaining
- Implement concrete mesh operation handlers for the currently stubbed commands (UV/normals/vertex colors/bake/reload/behavior copy-paste).
