# Task 0300 - Active Selection Render Sync

## Goal
Ensure selection changes in TreeArrayCtrl immediately update viewport rendering and active frustum visualization.

## Scope
- Fix any temporal mismatch where the previous selection is rendered instead of current.
- Ensure Tree selection triggers a renderer refresh after state updates.

## Implementation Notes
- Confirm selection pipeline order: state update -> refresh.
- Avoid stale focus ids when selection changes.

## Success Criteria
- Active object/frustum matches current selection every time.
- No stale selection renders in viewport.

## Status
- Pending
