# Task 0002 - Timeline Strategy (Flash-like)

## Goal
Define timeline behavior for ECS components, including script-driven keyframes.

## Scope
- Per-entity timeline and per-component subtracks.
- Keyframe data model for Transform, materials, mesh, and 2D layers.
- Script execution model (single keyframe with code, per-frame hooks, etc.).

## Deliverables
- Timeline strategy spec with sample workflows.

## Strategy

### Timeline Model
- **Scene Timeline**: one per scene, drives global time and playback settings.
- **Entity Timeline**: optional per-entity timeline nodes for visibility/activation.
- **Component Tracks**: each component can expose tracks (Transform, Material, Mesh, 2D Layer, Camera).
- **Track Rows**: UI shows rows for entity, then component, then sub-tracks (properties).

### Keyframe Types
- **Value keyframes**: store scalar/vector/quaternion values with interpolation.
- **Event keyframes**: call script or emit event without numeric value.
- **Script keyframes**: store code blobs executed when the frame is reached.

### Interpolation Rules
- Default interpolation: linear for scalars/vectors, slerp for quaternions.
- Per-keyframe easing modes: linear, smooth, hold, custom curve.
- Mesh/shape animation uses discrete frames unless explicitly set to interpolate.

### Script Execution Model
- Scripts can be triggered by:
  - **Keyframe enter** (one-shot event).
  - **Keyframe range** (enter/exit events).
  - **Per-frame hooks** (`onFrame`/`enterFrame`).
- **Single-keyframe scripts** are allowed and may drive full animation procedurally.
- Script execution order: scene scripts → entity scripts → component scripts.

### Evaluation Order (per frame)
1. Scene timeline advances.
2. Component tracks evaluate values.
3. Script keyframes execute (if frame crossed).
4. Procedural scripts can override component values (with write-enabled flag).

### Write/Read Flags
- **Read**: component track reads from timeline.
- **Write**: procedural/script changes write back to keyframes (auto-key).
- Write requires Read enabled (mirrors Flash-like auto-key rules).

### Sample Workflows
1) **Classic keyframe animation**
   - Set Transform keyframes at frames 0, 30, 60.
   - Scrub timeline: values interpolate.

2) **Script-driven animation**
   - Single keyframe at frame 0 with script:
     ```python
     if input.isKeyDown('W'):
         transform.position += vec3(0,0,1) * dt
     ```
   - No other keyframes needed.

3) **Hybrid**
   - Keyframe initial pose at frame 0.
   - Script in keyframe 1 drives physics/controls and writes back to keyframes if auto-key.

### Data Storage
- Tracks store values per component property path, e.g.:
  - `Transform.position.x`
  - `Transform.orientation`
  - `Material.baseColor`
- Script keyframes store file ref or inline code blob.

### UI Expectations
- Timeline rows are hierarchical:
  - Entity row
    - Transform row
      - position.x / position.y / position.z
      - orientation.w/x/y/z
    - Material row
      - baseColor.r/g/b
- Empty rows show faint placeholders; keyframes show filled markers.
