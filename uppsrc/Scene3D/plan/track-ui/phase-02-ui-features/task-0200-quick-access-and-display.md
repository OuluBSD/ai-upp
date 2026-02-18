# Task 0200 - Quick Access + Display Modes

## Status
- Done (2026-02-17)

## Goal
Provide ribbon display modes and a customizable quick access toolbar.

## Scope
- Auto-hide / tabs-only / always-show switching.
- QAT position (top/bottom) and basic customization UI.
- Persist ribbon UI settings in config.

## Acceptance
- Ribbon display toggles match Windows behavior.
- QAT edits persist across restarts.

## Implementation Notes
- Display mode switching is implemented (`always`, `tabs`, `auto-hide`).
- QAT position switching (`top` / `bottom`) is implemented.
- Ribbon UI settings persist in ModelerApp config (`ribbon_display_mode`, `ribbon_qat_pos`).
