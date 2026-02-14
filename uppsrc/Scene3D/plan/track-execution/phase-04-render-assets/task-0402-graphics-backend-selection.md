# Task 0402 - Graphics Backend Selection

## Goal
Render execution projects using uppsrc/api/Graphics backends.

## Scope
- Use Graphics API for executor rendering (OpenGL or software).
- Improve Graphics package as needed for execution runtime.
- Keep backend selectable and configurable.

## Success Criteria
- Executor renders scenes through Graphics API.
- Backend selection documented and working.
- Graphics improvements scoped to execution needs.

## Implementation Notes
- ModelMediaShell accepts backend aliases `gfx_sw` and `gfx_ogl` via `--renderer`.
- Keep `v1`, `v2`, and `v2_ogl` options available for comparison and fallback.

## Status
- Planned
