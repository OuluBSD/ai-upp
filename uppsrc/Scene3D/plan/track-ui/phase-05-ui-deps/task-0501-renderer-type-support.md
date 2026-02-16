# Task 0501 - Renderer Support for New Scene Types

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
