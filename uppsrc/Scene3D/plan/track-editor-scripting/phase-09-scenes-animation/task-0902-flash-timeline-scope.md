# Task 0902 - Flash-Style Timeline Scope

## Goal
Introduce an "active timeline" concept like Flash: Scene timeline at top, then selected object/component timeline as the active edit scope.

## Scope
- Scene timeline is always visible and controls the global playhead.
- Selecting an object activates its timeline scope for edits.
- Component tracks (Transform, Mesh, 2D) inherit the active scope.
- Auto-key when editing within active scope (optional toggle).

## Implementation Notes
- Active scope state stored in editor (scene/object/component).
- Playhead is shared; edits go to the active scope track.
- Highlight active scope row in timeline UI.

## Success Criteria
- Scene row always visible.
- Selecting an object switches the active editing scope.
- Auto-key works for active scope only.

## Status
- Pending
