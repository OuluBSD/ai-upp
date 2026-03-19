# Task 0101 - Polygon Tools Ribbon Mapping

## Status
- Done (2026-02-19)

## Goal
Connect Polygon Editing tab actions to mesh edit tools and selection modes.

## Scope
- Select/rectangle select, move/rotate/scale tools.
- UV modify entry point.
- Edit mode dropdown (triangle/point) mapping.
- Hook Tools button to mesh operations menu.

## Acceptance
- Polygon Editing tab matches mesh tool behavior.
- Dropdown and tool state reflect active selection mode.

## Notes
- Ribbon polygon actions now map to concrete mesh-selection behavior:
  - select / rectangle-select route to mesh select tool
  - move routes to mesh-select transform flow
  - rotate/scale apply direct selection transforms
  - UV modify switches to vertex selection mode as entry point
- `poly_tools` now opens the mesh-operations menu (loop/ring/expand/contract/extrude/inset/spin/screw).
- Edit-mode dropdown keeps triangle/point mode synchronized with current mesh selection mode.
