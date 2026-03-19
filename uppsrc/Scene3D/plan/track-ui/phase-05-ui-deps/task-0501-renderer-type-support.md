# Task 0501 - Renderer Support for New Scene Types

## Status
- Done (2026-02-19)

## Goal
Add Render/Exec support for new scene object types introduced for UI actions.

## Scope
- Lights: point + directional (ensure both influence lighting).
- Skybox: bind display/specular/irradiance in render path.
- Billboards: render as camera-facing quads.
- 2D overlays: render in screen-space; touchscreen overlay hooks.
- Particle systems: visualize with basic sprite emitter.
- 3D sound: pass positions to runtime/audio system.
- Paths/path nodes: draw debug spline + node markers.

## Acceptance
- Scene objects created via UI render or drive at runtime in at least one renderer.
- Rendering code paths are documented (Scene3D Render/Exec vs api/Graphics).

## Notes
- Implemented typed overlay/gizmo rendering in `Scene3D/Render/Renderer.cpp` via `DrawUiTypeOverlay(...)`, used from:
  - `EditRendererV1::PaintObject(...)`
  - `EditRendererV2_Ogl::Paint(...)` post-pass overlay path
- Implemented V2 lighting integration for typed UI lights in `Scene3D/Render/Renderer.cpp`:
  - `CollectUiLights(...)` gathers `light.point` and `light.directional`
  - `EvaluateUiLighting(...)` feeds both interactive V2 and headless V2 shading paths
- Added Exec type recognition for typed scene objects in `Scene3D/Exec/Exec.cpp`:
  - `NodeTypeMatches(...)` now matches `ui_type` (with aliases like `light`, `overlay`, `pathnode`)
  - `DisplayObjectProxy::GetAttr("type")` returns `ui_type` when present
- Coverage delivered in Scene3D software renderers:
  - point/directional light gizmos
  - skybox bounds marker + binding indicator
  - billboard and vertical billboard camera-facing quads
  - overlay2d/touch2d screen-space badges
  - particle-system emitter preview
  - sound3d icon marker
  - path/path-node debug links and node markers
