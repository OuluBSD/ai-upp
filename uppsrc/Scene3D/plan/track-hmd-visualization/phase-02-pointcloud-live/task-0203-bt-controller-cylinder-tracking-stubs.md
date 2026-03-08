Task 0203 - BT controller cylinder tracking stubs

Goal
- Stub data flow for two bluetooth controllers with dotted-cylinder patterns visible in dark frames.

Checklist
- [x] Define controller pattern model (two dot circles on cylinder, with unique dot layout).
- [x] Define stub tracker interface: detect pattern -> controller pose (relative to HMD).
- [x] Define stub fusion interface: merge pattern pose with bluetooth IMU/controller data.
- [x] Define ephemeral pointcloud integration: add temporary controller points to render.

Notes
- Keep detection algorithm as placeholder; wire data contracts only.
