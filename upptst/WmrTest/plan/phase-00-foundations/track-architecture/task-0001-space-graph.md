Task 0001 - Coordinate spaces and transforms

Checklist
- List all spaces: camera-left/right, stereo-camera, IMU body, fusion/odom, map/world, UI view.
- Define handedness, axes orientation, and units.
- Define transform chain and ownership (who updates which transform).
- Decide whether ORB matching runs in image space only or a dedicated "feature space".

Space graph (proposal)
- ImgL / ImgR (pixel space)
  - Units: pixels.
  - Origin: top-left or image center (explicit per algorithm).
  - Used for: keypoint detection, descriptor extraction.
- CamL / CamR (optical space, normalized)
  - Units: normalized image plane; undistorted.
  - Origin: principal point; +x right, +y down (confirm), +z forward.
  - Used for: stereo geometry, triangulation.
- Rig (stereo camera rig)
  - Units: meters.
  - Origin: midpoint between left/right camera centers.
  - Geometry default is RHS with negative Z forward (IS_NEGATIVE_Z=1); convert from camera +Z.
  - +x to right camera; +y up or down (confirm).
  - Used for: 3D points in camera-local frame.
- IMU (body)
  - Units: meters, radians.
  - Origin: IMU sensor frame.
  - Requires extrinsic T_imu_rig.
- Odom / Fusion
  - Units: meters, radians.
  - Origin: arbitrary at boot.
  - Pose output of VIO filter.
- Map / World
  - Units: meters, radians.
  - Origin: persistent map frame; set by relocalization.
- UI / View
  - Units: pixels.
  - Used for rendering overlays and 3D views.

Transform chain (ownership)
- Camera intrinsics: Img -> Cam (Geometry calibration data).
- Stereo extrinsics: CamL <-> CamR (Geometry calibration data).
- Rig extrinsics: Rig <-> CamL/CamR (Geometry calibration data).
- IMU extrinsics: Rig <-> IMU (Geometry calibration data; solved or fixed).
- Fusion: Odom pose from IMU + camera constraints (Geometry VIO).
- Map alignment: Map <- Odom (Relocalizer output).

Dataflow (high level)
1) ImgL/ImgR keypoints/ORB descriptors in Img space.
2) Undistort -> Cam space (using calibration).
3) Stereo match + triangulate -> Rig 3D points.
4) Temporal tracking -> constraints between frames.
5) Fusion updates -> Odom pose.
6) Map alignment -> Map pose (relocalization).

Notes
- ORB matching stays in Img space; it is not a separate physical space.
- Debug overlay should visualize transforms (e.g., Rig axes in Odom and Map).
- Explicitly document the camera->world handedness conversion when wiring tracking code.

Deliverables
- A diagram and a short written spec for transforms and ownership.
