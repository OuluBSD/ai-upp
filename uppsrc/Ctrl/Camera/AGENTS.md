AGENTS

Scope
- Applies to `uppsrc/Ctrl/Camera`.

Purpose
- UI controls for camera preview and overlays.
- Depends on `Draw/Camera` for capture backends and data types.

Guidelines
- Keep backend capture logic out of Ctrl/Camera.
- UI should remain generic; app-specific menus stay in consuming apps.
