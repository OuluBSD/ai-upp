Phase 02 - Live HMD pointcloud capture in ModelerApp

Goals
- Provide a live stereo camera preview viewport while capturing.
- Record a pointcloud in-memory from HMD stereo input with pose override.
- Keep Scene3D independent from SoftHMD (ModelerApp acts as adapter).

Deliverables
- ModelerApp: HMD preview viewport wired and recording toggle in menu.
- ModelerApp: scene setup includes HMD camera + pointcloud object.
- ModelerApp: program camera pose follows HMD while recording (no keypoints).
- Scene3D plan updated to reflect live capture pipeline work.

Notes
- PCL/OpenCV integration is deferred to a later phase after live capture is stable.
