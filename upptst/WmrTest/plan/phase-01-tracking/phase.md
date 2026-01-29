Phase 01 - Tracking

Goals
- Feature detection, matching, and 3D triangulation.
- Building a sparse pointcloud from stereo matches.

Status
- [x] High-performance camera capture with producer/consumer pattern.
- [x] Feature pipeline integrated with ComputerVision::OrbSystem.
- [x] Sparse pointcloud visualization via Octree.
- [x] Thread-safe data flow between capture and processing.

Exit Criteria
- Tracking maintains a consistent set of points across frames.
