# Task 01 - Extract V4L2 Backend into Ctrl/Camera

Status: pending

## Goal
Move V4L2 capture logic (and any reusable webcam capture utilities) into Draw/Camera.

## Scope
- Identify V4L2 capture classes and move into Draw/Camera as a backend.
- Provide a generic frame delivery interface consistent with SoftHMD backend.
- Keep device selection/configuration hooks.

## Acceptance
- WebcamRecorder builds using Draw/Camera V4L2 backend.
- Draw/Camera can switch between SoftHMD and V4L2 sources via API.
