# Task 0100 - Create Operations

## Status
- Done (2026-02-19)

## Goal
Bind Create tab actions to real object creation flows.

## Scope
- Primitive generation (cube/sphere/cylinder/cone/plane).
- Camera, lights, skybox, particle system, and 2D overlay items.
- Import static/animated mesh from assets.
- Dialog-driven creation for primitives and complex items (terrain, room, water, camera types) with defaults from spec.

## Acceptance
- Each Create tab button spawns the expected node in the scene tree.
- Created nodes appear in the viewport and can be selected.
- Dialogs appear where required and apply parameters to created objects.

## Notes
- Create ribbon actions are bound to primitive/object creation and asset import flows.
- Dialog-driven creation is wired for camera/terrain/room/water and primitive variants.
