Task 0003 - Geometry base classes and event model

Checklist
- Identify SoftHMD math/glue classes to move into Geometry (Fusion, tracking glue, event types).
- Define virtual interfaces in Geometry for: visual tracker, IMU tracker, fusion, relocalization.
- Define GeomEvent types for VR controller + HMD pose + camera frames.
- Decide ownership and data flow between Geometry (interfaces) and SoftHMD (impl).
- Use interface naming without Base suffix; implementations are prefixed (e.g., SoftHmd*).
- Note GeomEvent constraints (trivial/union padding if required).

Deliverables
- A proposed class hierarchy + event spec, with target files/headers.
