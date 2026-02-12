# Task 0101 - Compile + Export Pipeline

## Goal
Define and implement the compiled project pipeline for execution.

## Scope
- Compiled project file format for executor (ByteVM-friendly).
- Export modes:
  - Full export: bundle assets into an export directory.
  - Lightweight export: keep assets in place, store references.
- Projects stored under share/ and loaded via ShareDirFile().
- Executor can run ByteVM-only payloads (no full editor state).

## Success Criteria
- ModelerApp can compile and export a project in both modes.
- Executor can load compiled artifacts for either mode.
- Exported layout is documented and consistent.

## Status
- Done
