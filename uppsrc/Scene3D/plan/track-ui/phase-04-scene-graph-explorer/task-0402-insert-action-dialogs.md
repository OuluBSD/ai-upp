# Task 0402 - Insert Action Dialogs

## Status
- Partial / Open (updated 2026-02-17)

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

## Remaining
- Verify strict dialog field parity against `scene_graph_explorer.xml` for edge cases and unexposed options.
