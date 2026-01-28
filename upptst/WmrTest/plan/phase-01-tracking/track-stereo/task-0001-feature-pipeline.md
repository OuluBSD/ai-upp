Task 0001 - Feature pipeline rewrite plan

Checklist
- Evaluate OrbSystem + DescriptorImage data flow.
- Define where to store per-feature metadata (score, scale, orientation, stereo pair id).
- Decide matching strategy: brute-force Hamming, grid-indexed Hamming, or FLANN-like.
- Define a consistent descriptor format between ComputerVision and Geometry.

Deliverables
- A concrete feature pipeline spec and module boundaries.

Feature pipeline spec
- Inputs: rectified L/R grayscale frames + calibration.
- Detection: FAST/ORB keypoints per eye (ComputerVision::OrbSystem).
- Description: Descriptor32 (256-bit) with x,y,angle and optional scale/score.
- Matching:
  - Stereo matching: row-constrained (epipolar) Hamming with disparity window.
  - Temporal matching: grid-indexed Hamming with ratio test.
- Output:
  - Stereo pairs -> triangulated 3D points in Rig space.
  - Temporal tracks -> constraints for fusion/pointcloud update.

Metadata storage
- Add a lightweight `FeatureMeta` struct in Geometry:
  - score, scale, octave, id, stereo_id, track_id.
- Keep Descriptor32 in Draw/Extensions (shared); store meta alongside descriptor vectors.

Module boundaries
- ComputerVision: keypoint detection + descriptor generation.
- Geometry: stereo matching, triangulation, point tracking, spatial structures.
- SoftHMD: device capture and delivery into Geometry interfaces.
