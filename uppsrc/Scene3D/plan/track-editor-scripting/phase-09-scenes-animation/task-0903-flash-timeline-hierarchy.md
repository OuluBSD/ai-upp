# Task 0903 - Flash-Style Timeline Hierarchy

## Goal
Show scene/object/component hierarchy in the timeline, similar to Flash layers.

## Scope
- Timeline rows become a tree: Scene -> Object -> Component tracks.
- Collapse/expand object tracks.
- Row context menu supports: Make Active Timeline, Add/Remove Keyframe, Lock/Mute/Solo (if applicable).

## Implementation Notes
- Use a tree-like row model inside TimelineCtrl (indentation + disclosure).
- Keep row visuals compact to mimic Flash layer list.

## Success Criteria
- Hierarchical rows render correctly.
- Collapse/expand works.
- Active timeline selection is clear.

## Status
- Pending
