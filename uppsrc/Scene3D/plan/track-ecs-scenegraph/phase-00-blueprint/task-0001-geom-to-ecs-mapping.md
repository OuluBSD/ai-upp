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

## Mapping Table

### Core Scene
| Geom* type | ECS equivalent | Notes |
|---|---|---|
| GeomProject | ECS Scene/World root | Project metadata stored in Scene root component or project settings node. |
| GeomScene | ECS Scene node (entity) | Scene becomes root entity with children. |
| GeomDirectory | Entity (folder node) | Pure hierarchy node with Transform (optional). |
| GeomObject | Entity | All objects become ECS entities. |
| GeomObjectState | transient runtime state | Derived from Transform + animation. No persistent type. |
| GeomWorldState | ECS WorldState | Holds active scene, focus entity, program camera, selection. |

### Transform + Camera
| Geom* type | ECS equivalent | Notes |
|---|---|---|
| GeomTransform | Transform component | Local space; parent-child composition handled by ECS. |
| GeomCamera | CameraBase + Viewport + Viewable | Camera entity uses these components; fov/scale on CameraBase. |
| GeomCamera (focus/program) | ECS camera entities | Focus/program selection stored in WorldState. |

### Rendering / Visuals
| Geom* type | ECS equivalent | Notes |
|---|---|---|
| GeomObject::O_MODEL + Model | ModelComponent | ModelComponent owns mesh asset ref + material bindings. |
| GeomEditableMesh | EditableMesh component | Mesh editing data stored on component. |
| Geom2DLayer | Overlay2D component | 2D shapes rendered in 3D plane or screen space. |
| GeomBillboard | Billboard component | Vertical flag maps to Billboard axis policy. |
| GeomSkybox | Skybox component | Bound on a scene entity (singleton per scene). |
| GeomLight | Light component | Types: directional/point. |
| GeomParticleSystem | ParticleSystem component | Emitter settings in component. |

### Audio + Interaction
| Geom* type | ECS equivalent | Notes |
|---|---|---|
| GeomSound3D | Sound3D component | Audio resource ref + radius/volume. |
| GeomOverlay2D (touchscreen_input) | InputOverlay component | Touchscreen overlays become input component variant. |

### Animation / Timeline
| Geom* type | ECS equivalent | Notes |
|---|---|---|
| GeomTimeline | TimelineTrack component | Track per component/property, hierarchical in UI. |
| GeomSceneTimeline | SceneTimeline component | Scene-level timeline entity (global). |
| GeomAnim | AnimationSystem runtime | Playback state lives in system + per-scene timeline component. |

### Paths
| Geom* type | ECS equivalent | Notes |
|---|---|---|
| GeomPath | Path component | Spline points stored on component. |
| GeomPathNode | PathNode component | Optional; for UI editing nodes or linked entities. |

### Pointcloud / Dataset
| Geom* type | ECS equivalent | Notes |
|---|---|---|
| GeomPointcloudDataset | PointcloudDataset component or resource | Stored in scene resource registry. |
| GeomPointcloudEffectTransform | PointcloudEffect component | Effect stack stored on dataset or pointcloud entity. |
| GeomObject::O_OCTREE | PointcloudEntity + Octree component | Octree data attached to entity; dataset binding via ref. |

## Migration Notes
- **IDs**: Geom `name` fields map to ECS entity IDs; duplicates get suffixed.
- **Visibility/Lock/Read/Write**: map to generic `NodeState` component or flags on entity.
- **DynamicProperties**: replaced by explicit components; remaining dynamic fields become `Metadata` component.
- **Selection**: selection model stores `(entity_id, component_type, component_instance)`.
- **Execution**: ByteVM paths resolve to ECS entity/component paths only.
- **Legacy**: Geom* wrappers remain read-through over ECS until Phase 06 completion.
