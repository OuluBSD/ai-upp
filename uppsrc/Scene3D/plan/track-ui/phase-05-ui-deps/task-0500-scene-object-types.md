# Task 0500 - Scene Object Types for UI Actions

## Status
- Done (2026-02-19)

## Goal
Provide concrete Scene3D object types backing UI create actions for lights, skybox, particles, 3D sound, and billboard/overlay items.

## Scope
- Define Scene3D extensions or properties for:
  - Point and directional lights.
  - Skybox object/cubemap binding.
  - Particle system object.
  - 3D sound source object.
  - Billboard / vertical billboard.
  - 2D overlay + touchscreen items.
  - Path/path node.
- Update render/runtime to respect these types.
- Replace dynamic-property tags with real types.

## Acceptance
- Create actions instantiate real typed nodes (not just tagged models).
- Renderer and runtime recognize and render/drive these objects.

## Progress
- Added built-in `GeomObject::ui_type` field in Scene3D core (`GeomObject` serialization updated).
- Create flows now populate `ui_type` alongside legacy dynamic `type` tags for compatibility.
- Path-node and directional-light checks now read typed object data (`ui_type`) first, then fall back to legacy dynamic props.

## Completed
- Modeler create-actions now instantiate typed scene objects via `GeomObject::ui_type` and editor logic reads typed data for path/light behavior.
- Legacy dynamic `type` and `light_type` write/check paths were removed from new-create flows.
- Render/Exec consumers were updated to prefer typed behavior (`ui_type`) with compatibility fallback reads for older scene data.
