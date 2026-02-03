# Task 02 - Adapt WebcamRecorder to Ctrl/Camera

Status: pending

## Goal
Refactor reference/WebcamRecorder to use Draw/Camera + Ctrl/Camera for capture and preview.

## Scope
- Replace internal capture loop with Draw/Camera backend.
- Use Ctrl/Camera for preview UI.
- Keep recorder-specific encode/output logic local to WebcamRecorder.

## Acceptance
- WebcamRecorder compiles and captures at least as fast as before.
- Preview path uses Ctrl/Camera UI components.
