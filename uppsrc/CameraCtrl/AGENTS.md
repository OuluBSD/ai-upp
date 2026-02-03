AGENTS

Scope
- Applies to `uppsrc/CameraCtrl`.

Purpose
- UI controls for camera preview and overlays.
- Depends on `CameraDraw` for capture backends and data types.

Guidelines
- Keep backend capture logic out of CameraCtrl.
- UI should remain generic; app-specific menus stay in consuming apps.
