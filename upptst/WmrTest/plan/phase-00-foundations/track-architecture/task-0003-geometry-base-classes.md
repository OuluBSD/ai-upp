Task 0003 - Geometry base classes and event model [DONE]

Checklist
- [x] Identify SoftHMD math/glue classes to move into Geometry (Fusion, tracking glue, event types).
- [x] Define virtual interfaces in Geometry for: visual tracker, IMU tracker, fusion, relocalization.
- [x] Define GeomEvent types for VR controller + HMD pose + camera frames.
- [x] Decide ownership and data flow between Geometry (interfaces) and SoftHMD (impl).
- [x] Use interface naming without Base suffix; implementations are prefixed (e.g., SoftHmd*).
- [x] Note GeomEvent constraints (trivial/union padding if required).

Interface naming (proposal)
- Abstract interfaces in Geometry:
  - VisualTracker (pure virtual)
  - ImuTracker (pure virtual)
  - Fusion (pure virtual)
  - Relocalizer (pure virtual)
  - StereoCalibration (pure virtual)
- Concrete implementations in SoftHMD:
  - SoftHmdVisualTracker
  - SoftHmdImuTracker
  - SoftHmdFusion
  - SoftHmdRelocalizer

GeomEvent extensions (proposal)
- Extend GeomEvent with VR/HMD events:
  - GeomEvent::HmdPose
  - GeomEvent::ControllerState
  - GeomEvent::CameraFrame
  - GeomEvent::ImuSample
- Ensure event payload fits GeomEvent constraints (check whether union/trivial is required).
  - If GeomEvent must be trivially copyable, use a tagged union or fixed-size payload + external buffers.

Ownership and dataflow
- Geometry owns math/data structures and interfaces.
- SoftHMD owns device capture and pushes packets into Geometry interfaces.
- WmrTest consumes GeomEvent stream and state matrices for visualization.

Deliverables
- A proposed class hierarchy + event spec, with target files/headers.

Findings: GeomEvent constraints (from codebase)
- GeomEvent is Moveable and stored in Vector<GeomEvent>, passed by value through Event systems.
- sizeof(GeomEvent) is used by Vfs/Ecs samples; treat GeomEvent as trivially movable and memcpy-safe.
- Current payload is pointer-only; no ownership or destructors are assumed.
- Avoid adding dynamic containers (String, Vector, Array, etc.) to GeomEvent payload.

Decision (per direction)
- Prefer an in-place union payload in GeomEvent (type-tagged), not owning pointers.
- Each union member is a POD payload struct with explicit padding so all variants match size.
- GeomEvent remains trivially copyable/movable; payload is value-only.

Proposed union payload layout (Geometry-level)
- Add `GeomEventPayload` union with fixed-size POD structs:
  - HmdPosePayload (pose + fov/eye_dist + timestamp + flags).
  - ControllerPayload (buttons/axes + pose + timestamp + flags).
  - ImuSamplePayload (accel/gyro/mag + timestamp + flags).
  - CameraFramePayload (size/format/stride + pointer to raw buffer + timestamp + flags).
  - FusionPayload (fused pose + quality/covariance + timestamp + flags).
- Each payload struct is padded to `GEOM_EVENT_PAYLOAD_BYTES` (single constant).
- GeomEvent::type selects which payload is valid; scalar fields (value/n/pt/pt3) remain for legacy.

Notes
- CameraFramePayload likely still needs a raw buffer pointer; keep it explicitly non-owning.
- If pointer payload is used (e.g., buffer or shared state), document lifetime and ownership clearly.
