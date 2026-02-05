# Task 0102 - Script Runtime Hooks

## Goal
Provide stable runtime hooks (on_load/on_frame) with project-relative script resolution and safe reload behavior.

## Scope
- Ensure scripts are reloaded when file changes.
- Provide on_start/on_frame hooks with `__dt__` and `__project_dir__` globals.
- Avoid repeated re-parsing unless file timestamp changes.

## Success Criteria
- Scripts run on load and per-frame when enabled.
- File changes reload automatically without restart.

## Status
- Done (2026-02-05)
  - Added on_load hook support and reload-safe execution.
  - `__project_dir__` and `__dt__` globals available (frame uses `__dt__`).
