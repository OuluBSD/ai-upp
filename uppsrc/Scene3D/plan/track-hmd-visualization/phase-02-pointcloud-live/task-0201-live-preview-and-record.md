Task 0201 - Live preview + record hook

Checklist
- [x] Provide HMD stereo preview viewport in ModelerApp grid.
- [x] Add menu toggle to start/stop pointcloud recording.
- [x] Ensure HMD camera + pointcloud objects exist in test scene.
- [x] Bind tracker octree to Scene3D pointcloud object via pointer.
- [x] Override program camera pose from HMD while recording.
- [ ] Runtime verify with HMD attached.

Notes
- Uses SoftHMD fusion + tracker for pointcloud data.
- Scene3D remains backend-agnostic; HMD logic stays in ModelerApp.
