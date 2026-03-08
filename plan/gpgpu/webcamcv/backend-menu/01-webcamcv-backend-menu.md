# GPGPU Plan - WebcamCV Backend Menu

## Objective
Expose backend selection in `reference/WebcamCV` so users can switch CPU/AMP/OpenGL(stub) during demo runtime.

## Current Status
- Implemented: MenuBar `Backend` submenu.
- Implemented: backend selection actions for CPU, AMP, OpenGL(stub).
- Implemented: title/status update and stub warning popup for unsupported backend.

## Next Tasks
1. Persist backend selection in app config.
2. Add per-demo backend overrides where needed (e.g. ORB first).
3. Add visible on-screen overlay for active backend and FPS.

## Notes
- OpenGL backend remains non-functional by design in this phase.
