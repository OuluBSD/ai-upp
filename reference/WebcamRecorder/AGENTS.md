AGENTS

Scope
- Applies to `reference/WebcamRecorder`.

Purpose
- Demonstration app for webcam capture + preview + recording.
- To be migrated to use Draw/Video backends and Ctrl/Camera UI.

Guidelines
- Use threaded capture pattern matching WmrTest (background thread + UI refresh).
- Keep recorder-specific output logic local to this app.
