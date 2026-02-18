# Task 0201 - Context Tabs + Key Tips

## Status
- Done (2026-02-18)

## Goal
Add contextual tabs and ALT key keytips for ribbon navigation.

## Scope
- Contextual tabs for object selection (e.g., camera, mesh, material).
- ALT key shows keytips for tab and control activation.
- Keyboard navigation for ribbon items.

## Acceptance
- Context tabs appear only for matching selection.
- Keytips allow full ribbon navigation without mouse.

## Implementation Notes
- ModelerApp context coverage expanded beyond camera to `object`, `mesh`, and `material` contextual tabs with selection-based visibility updates.
- ALT keytips now enumerate visible tabs and actionable controls in the active ribbon page.
- Keyboard keytip activation is wired: tab switching by keytip token and control activation via keytip token (with Esc to cancel keytip mode).
