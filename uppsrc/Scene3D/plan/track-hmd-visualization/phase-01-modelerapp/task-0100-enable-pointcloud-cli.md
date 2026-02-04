Task 0100 - Enable ModelerApp pointcloud CLI

Checklist
- [x] Allow `-n/--pointcloud` path in ModelerApp main without remote mode.
- [x] Keep remote code gated (disabled) while allowing pointcloud mode.
- [ ] Verify ModelerApp loads WmrTest pointcloud directory at runtime.

Notes
- Remote client code stays behind `#if 0` for now.
- Pointcloud mode uses `Edit3D::LoadWmrStereoPointcloud`.
