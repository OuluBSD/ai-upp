# Task 01 - Extract WmrTest Core into Ctrl/Camera

Status: pending

## Goal
Move generic camera capture core into Draw/Camera and camera preview + overlay UI into Ctrl/Camera.

## Scope
- Extract headless capture + tracking data interfaces into Draw/Camera.
- Extract camera UI widgets (camera view, overlays, stats display) into Ctrl/Camera.
- Define Draw/Camera public API for: frames, overlays, stats, callbacks.
- Keep SoftHMD-specific logic behind a backend interface in Draw/Camera.

## Out of Scope
- No refactor of SoftHMD tracking core or OpenHMD code.
- No StereoCalibrationTool changes.

## Acceptance
- WmrTest builds and runs using Draw/Camera + Ctrl/Camera APIs.
- Draw/Camera contains no UI code.
- Ctrl/Camera contains no backend capture logic.
