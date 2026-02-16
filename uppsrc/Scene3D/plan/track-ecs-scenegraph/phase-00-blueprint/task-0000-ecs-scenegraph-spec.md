# Task 0000 - ECS Scene Graph Spec

## Goal
Define the ECS-first scene graph structure used by ModelerApp and Eon09.

## Scope
- Entity naming, IDs, and hierarchy rules.
- Component categories for SceneGraph Explorer.
- Required components for a valid renderable entity.
- Rules for selection, focus, and active camera.

## Deliverables
- Written spec with examples and allowed component order.

## Spec
### Entities and IDs
- Entities are the primary scene graph nodes.
- Each entity has a stable string ID (unique within scene) used for UI paths, scripts, and IO.
- IDs are path-safe: `[A-Za-z0-9_]+`, no spaces, no slashes. Display names are separate fields.
- Default entity ID is derived from type + index (e.g., `camera_0`, `cube_1`).

### Hierarchy
- Entities form a tree (parent/child).
- Parent owns transform space; child transform is local to parent.
- Scene root is a virtual entity (`scene_root`) that is not exposed in IO but is present in runtime.

### Component Model
- Components are attached to entities; each component has a type and optional instance name.
- SceneGraph Explorer shows:
  - Entity node
    - Component nodes (Transform, Model, Camera, Light, Script, etc.)
    - Subcomponents under a component (e.g., Timeline -> Tracks)
- Component ordering is deterministic and stable:
  1) Transform
  2) Visual/Render components (Model, Mesh, Billboard, Overlay)
  3) Camera/Viewport components
  4) Light/Skybox/Particle/Sound
  5) Animation/Timeline components
  6) Script/Logic components

### Required Components
- Renderable entity: Transform + Model (or Mesh/Overlay) component.
- Camera entity: Transform + CameraBase + Viewport + Viewable.
- Light entity: Transform + Light component.

### Selection and Focus
- Selection uses entity ID + component ID.
- Focus camera is an ECS camera entity (not a GeomCamera wrapper).
- Active camera source is resolved from:
  1) Explicit focus camera entity
  2) Program camera entity
  3) First camera found

### Paths and Addressing
- UI path: `scene/<entity_id>/<component_type>[/<sub_id>]`
- Script path: `/scene/<entity_id>/<component_type>`
- Components are addressable by type; if multiple instances exist, use instance name suffix.

### Examples
- `scene/cube_0/Transform`
- `scene/camera_0/CameraBase`
- `scene/sphere_2/Model`
