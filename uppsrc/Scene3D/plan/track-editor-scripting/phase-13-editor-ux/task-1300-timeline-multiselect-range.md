# Task 1300 - Timeline Multi-Row Selection & Range Editing

## Goal
Enable multi-row selection and batch operations across the timeline.

## Scope
- Multi-row select (Shift/Ctrl) in timeline rows.
- Range selection across rows with box-drag.
- Apply keyframe add/remove to all selected rows.
- Bulk delete keyframes in range.
- Range copy/paste (row-relative).

## Implementation Notes
- TimelineCtrl should expose selected rows + range(s).
- Provide a compact selection overlay and status line.
- Add "Apply to Selected Rows" in timeline row menu.

## Success Criteria
- Multi-row selection is stable and visible.
- Batch operations work on Transform/Position/Orientation rows.
