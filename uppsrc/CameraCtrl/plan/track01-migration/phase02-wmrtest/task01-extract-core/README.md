# Task 01 - Extract WmrTest Core into CameraCtrl

Status: pending

## Goal
Move generic camera capture core into CameraDraw and camera preview + overlay UI into CameraCtrl.

## Scope
- Extract headless capture + tracking data interfaces into CameraDraw.
- Extract camera UI widgets (camera view, overlays, stats display) into CameraCtrl.
- Define CameraDraw public API for: frames, overlays, stats, callbacks.
- Keep SoftHMD-specific logic behind a backend interface in CameraDraw.

## Out of Scope
- No refactor of SoftHMD tracking core or OpenHMD code.
- No StereoCalibrationTool changes.

## Acceptance
- WmrTest builds and runs using CameraDraw + CameraCtrl APIs.
- CameraDraw contains no UI code.
- CameraCtrl contains no backend capture logic.
