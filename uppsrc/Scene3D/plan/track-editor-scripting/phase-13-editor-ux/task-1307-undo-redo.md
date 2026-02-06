# Task 1307 - Undo/Redo Stack

## Goal
Add robust undo/redo for editor actions.

## Scope
- Properties edits (transform, materials, textures).
- Keyframe add/remove/move.
- Object create/delete.
- Timeline row operations.

## Success Criteria
- Undo/redo is deterministic and never corrupts state.
- History size is bounded.
