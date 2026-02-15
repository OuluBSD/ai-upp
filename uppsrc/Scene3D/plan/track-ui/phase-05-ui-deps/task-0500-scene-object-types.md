# Task 0500 - Scene Object Types for UI Actions

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
