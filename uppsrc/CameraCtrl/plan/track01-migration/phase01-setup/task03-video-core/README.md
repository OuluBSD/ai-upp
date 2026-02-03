# Task 03 - Draw/Video Core Package

Status: pending

## Goal
Define and extract shared headless video/camera interfaces into `uppsrc/Draw/Video`, and migrate relevant code from `uppsrc/api/Media`.

## Scope
- Introduce `uppsrc/Draw/Video` as the shared base for CameraDraw and ComputerVision.
- Move or adapt reusable capture/device/core types from `uppsrc/api/Media/*`.
- Keep UI out of Draw/Video.

## Acceptance
- CameraDraw and ComputerVision can share base interfaces without circular deps.
- api/Media content is migrated or proxied, with clear ownership.

## Notes
- Focus on performance, modularity, and debuggability.
