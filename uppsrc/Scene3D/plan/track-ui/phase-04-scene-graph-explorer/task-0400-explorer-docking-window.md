# Task 0400 - Scene Graph Explorer Docking Window

## Status
- Done (2026-02-17)

## Goal
Add a SceneCraft Explorer docking window with a TreeArrayCtrl-based scene graph, driven by scene_graph_explorer.xml.

## Scope
- New docking window (left) with TreeArrayCtrl.
- Root node binds to active scene, display name from scene_name.
- Preserve all current TreeArrayCtrl features (columns, toggles, focus, filters).
- Support preferred tree arrow control if present.

## Acceptance
- Explorer dock appears and mirrors current scene graph.
- Existing tree features still work as before.
- Root node label matches the active scene name.

## Implementation Notes
- SceneCraft Explorer is docked on the left and uses the existing `TreeArrayCtrl` with columns/toggles/focus behavior preserved.
- Root label is bound to active scene name with fallback scene index naming.
