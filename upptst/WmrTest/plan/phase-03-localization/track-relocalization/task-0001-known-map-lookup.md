Task 0001 - Known map lookup

Checklist
- Define map format for stored pointcloud + feature descriptors.
- Define relocalization pipeline (PnP/RANSAC or stereo constraints).
- Define map selection and scoring.

Deliverables
- A relocalization spec and map storage format.

Relocalization spec
- Map format:
  - Keyframes with pose + descriptors + 3D points.
  - Descriptor index (hash/LSH) for quick candidate retrieval.
- Pipeline:
  1) Extract features from current stereo frame.
  2) Retrieve candidate keyframes (hash hits).
  3) PnP/RANSAC against 3D map points to estimate pose.
  4) Validate with reprojection + inlier ratio; update Map<-Odom.
- Scoring:
  - Inlier count, reprojection error, and coverage.
  - Pick best candidate above threshold; otherwise keep odom.

Map storage format
- JSON for metadata + binary blob for descriptors/points.
- Versioned schema to allow upgrades.
