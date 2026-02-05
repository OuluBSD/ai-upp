# Task 0101 - Effect Stack UI for Pointcloud

## Goal
Provide a proper effect/addon stack for pointcloud nodes with per-effect visibility and lock controls.

## Scope
- Add effect stack UI in the Properties panel for pointcloud nodes.
- Effects should be toggled independently (visibility/lock) without hiding the base pointcloud.
- Effects should be serializable and stored as components/subnodes (not only UI state).

## Implementation Notes
- Use VfsValueExt components for effects (e.g., LocalizationEffect).
- Pointcloud render path should apply enabled effects in order.
- Visibility toggle should disable the effect but keep base pointcloud visible.

## Success Criteria
- Effect stack shows for pointcloud nodes with per-effect eye/lock toggles.
- Toggling effects changes rendering but does not hide base pointcloud.
- Effects survive save/load.

## Status
- Pending
