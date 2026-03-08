Task 0202 - Find location in pointcloud (stub)

Goal
- Add a pointcloud localization stub that estimates pose from observed points.

Checklist
- [x] Define inputs/outputs for pointcloud localization (observed points + reference map -> pose).
- [x] Add a stub interface in Scene3D/IO or Scene3D/Core with clear data contracts.
- [x] Add a minimal caller in ModelerApp for future wiring (no real math yet).

Notes
- Keep Scene3D backend-agnostic; actual tracking stays in app/adapters.
