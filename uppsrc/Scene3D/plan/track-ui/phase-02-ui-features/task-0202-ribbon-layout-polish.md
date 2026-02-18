# Task 0202 - Ribbon Layout Polish

## Status
- Done (2026-02-17)

## Goal
Refine ribbon layout and sizing to match Windows-style ribbons.

## Scope
- Normalize group heights across tabs.
- Ensure list groups and form groups size correctly with content.
- Balance padding/margins for large and list buttons.
- Improve visibility of empty groups (placeholders).

## Acceptance
- Switching tabs does not change ribbon height.
- Groups align consistently with correct padding.
- Empty groups are visually distinct without collapsing layout.

## Implementation Notes
- Ribbon content height now uses a stable baseline computed from visible tabs, so switching tabs does not resize the ribbon frame.
- Empty ribbon groups no longer collapse and now draw a visible placeholder border.
- Group min-size handling was adjusted for list/form content so groups stay readable and aligned.
