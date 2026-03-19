# Task 0402 - Insert Action Dialogs

## Status
- Done (2026-02-19)

## Goal
Implement insert action dialogs and behaviors described in scene_graph_explorer.xml.

## Scope
- Dialogs for cube/sphere/cylinder/cone/plane/camera/terrain/room/water.
- File dialogs for static/animated mesh import.
- Direct-create behaviors (lights, skybox, particles, overlays, etc.).
- Context-dependent creation for path nodes.

## Acceptance
- Each insert action matches dialog fields and default values.
- Creation integrates with current scene graph and properties.
- Direct-create actions do not prompt unnecessarily.

## Current State
- Dialog-based insert actions are wired for primitives, camera, terrain, room mesh, water, and mesh imports.
- Direct-create actions are wired for lights/skybox/particles/overlays/path-related objects.
- Context-dependent creation for `create_path_node` is now enforced in both menu availability and action handling.
- Directional light now follows single-instance-per-scene behavior.
- Parity refinements:
  - `create_tree` now opens the expected "Generate new tree" dialog before creation.
  - Static mesh import dialog title now matches XML wording ("Please select a mesh to load").
