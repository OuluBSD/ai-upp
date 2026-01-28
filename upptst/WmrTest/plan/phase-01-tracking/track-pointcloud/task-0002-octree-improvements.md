Task 0002 - Octree improvements for tracking

Checklist
- Audit Octree for needed features (range queries, nearest neighbors, memory layout).
- Add hooks needed by pointcloud tracking.
- Keep descriptor matching separate from spatial indexing unless proven useful.

Deliverables
- Octree improvement list + target API changes.

Octree improvement list
- Query API:
  - Range query by AABB/Sphere (already exists) + nearest neighbor (kNN).
  - Iterator that returns only live points (skip deleted).
- Maintenance:
  - Soft-delete + periodic compaction/rebuild.
  - Reinsert/update point position without full remove/insert.
- Limits:
  - Max depth and max objects per node (configurable).
  - Node pooling to reduce allocations.
- Metadata:
  - Timestamp or frame id for pruning.
  - Point confidence/quality to gate neighbors.

Target API changes (Geometry)
- `Octree::GetNearest(const vec3& p, int k, float radius)`
- `Octree::UpdateObject(OctreeObject& obj, const vec3& new_pos)`
- `Octree::Compact()` / `Octree::Rebuild()`
