# Task 0001 - Geom* to ECS Mapping

## Goal
Create a definitive mapping from Geom* types to ECS entities/components.

## Scope
- GeomObject -> Entity
- GeomTransform -> Transform component
- GeomCamera -> CameraBase + Viewport + Viewable
- GeomTimeline/GeomAnim -> ECS timeline component(s)
- GeomLight/Skybox/Particle/Overlay/Billboard/Sound/Path -> ECS components

## Deliverables
- Mapping table with data fields and migration notes.
