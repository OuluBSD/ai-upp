# Task 0001 - Ribbon Action Stubs

## Goal
Wire ribbon action ids to existing ModelerApp commands and provide stubs for unimplemented actions.

## Scope
- Map file/undo/redo and view mode actions to existing handlers.
- Map selection/transform tools to Edit3D tools.
- Add placeholder handlers for create/import/light/publish actions.
- Log unhandled action ids for follow-up work.

## Acceptance
- Common actions (New/Open/Save/Undo/Redo/View/Move/Rotate) work from ribbon.
- Unhandled actions are logged once per action id.
