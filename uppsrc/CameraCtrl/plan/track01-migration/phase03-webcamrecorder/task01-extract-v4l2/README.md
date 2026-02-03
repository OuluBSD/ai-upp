# Task 01 - Extract V4L2 Backend into CameraCtrl

Status: pending

## Goal
Move V4L2 capture logic (and any reusable webcam capture utilities) into CameraDraw.

## Scope
- Identify V4L2 capture classes and move into CameraDraw as a backend.
- Provide a generic frame delivery interface consistent with SoftHMD backend.
- Keep device selection/configuration hooks.

## Acceptance
- WebcamRecorder builds using CameraDraw V4L2 backend.
- CameraDraw can switch between SoftHMD and V4L2 sources via API.
