# Task 01 - Move V4L2 Backends to Draw/Camera

Status: completed

## Goal
Move V4L2 capture + device enumeration out of Draw/Video into Draw/Camera.

## Scope
- `V4L2Capture.*` and `V4L2DeviceManager.*` moved to `uppsrc/Draw/Camera`.
- Remove Draw/Video includes/entries for V4L2.

## Acceptance
- Draw/Video remains backend-agnostic.
- Draw/Camera exposes V4L2 helpers for Linux.
