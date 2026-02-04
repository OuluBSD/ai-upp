Task 0102 - Record HMD pointcloud in ModelerApp

Checklist
- [x] Add Record Pointcloud toggle to ModelerApp menu.
- [x] Create/ensure HMD camera + pointcloud objects in test scene.
- [x] Bind HMD tracker octree to Scene3D octree object via pointer.
- [x] Override program camera pose from HMD while recording (no keypoints).
- [ ] Runtime verify pointcloud updates + pose override.

Notes
- Stereo calibration uses SoftHMD SetWmrDefaults (loads share/calibration/*.stcal).
- Future: evaluate PCL keypoints/OpenCV calibration path when we hook HMD frames into Scene3D capture pipeline.
