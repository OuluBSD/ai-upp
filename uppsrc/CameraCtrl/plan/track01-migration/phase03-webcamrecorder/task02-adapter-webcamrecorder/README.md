# Task 02 - Adapt WebcamRecorder to CameraCtrl

Status: pending

## Goal
Refactor reference/WebcamRecorder to use CameraDraw + CameraCtrl for capture and preview.

## Scope
- Replace internal capture loop with CameraDraw backend.
- Use CameraCtrl for preview UI.
- Keep recorder-specific encode/output logic local to WebcamRecorder.

## Acceptance
- WebcamRecorder compiles and captures at least as fast as before.
- Preview path uses CameraCtrl UI components.
