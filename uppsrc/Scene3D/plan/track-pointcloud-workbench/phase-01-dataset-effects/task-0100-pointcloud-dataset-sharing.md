# Task 0100 - Pointcloud Dataset Sharing

## Goal
Introduce shared pointcloud datasets so multiple nodes can reference the same data without duplication.

## Scope
- Add a dataset reference field for pointcloud nodes (e.g., `pointcloud_ref` or dedicated dataset id).
- Provide a shared dataset registry in Scene3D/Core so multiple GeomObjects can reference the same Octree/Point set.
- Ensure property panel exposes dataset selection and current binding.

## Implementation Notes
- Use a lightweight shared storage (e.g., map of id -> Octree/Pointcloud buffer).
- Keep serialization compatible: store dataset id in GeomObject and keep external data in `data_dir`.
- Avoid copying Octree when duplicating nodes.

## Success Criteria
- Two pointcloud nodes can point to the same dataset id and render identical points.
- Switching dataset id in properties updates rendering without data duplication.
- Save/load preserves dataset references.

## Status
- Pending
