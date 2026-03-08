Task 0001 - Stereo calibration program and data format [DONE]

Checklist
- [x] Define a calibration capture flow (checkerboard/aruco).
- [x] Define output parameters: intrinsics, distortion, extrinsics, baseline, per-eye FOV, inter-eye angle.
- [x] Decide where calibration data lives and how it is loaded.

Required calibration outputs (explicit)
- Angle between images (if not parallel).
- Distance between eyes (baseline), measured or estimated.
- Fish-eye distortion coefficients (if present).
- Field-of-view angle per eye.

Known/unknown input scenarios
- Known baseline and known distance from camera center to a target.
- Unknown baseline, but known distance between two tracked points in the image.
- Mixed constraints: known baseline but unknown target distance (or vice versa).

Deliverables
- Calibration spec and file format (e.g., JSON, .cfg).

Calibration spec (initial)
- Use a single calibration blob per device with:
  - Intrinsics per eye: fx, fy, cx, cy.
  - Distortion per eye: k1, k2, k3, p1, p2 (support fisheye model as variant).
  - Extrinsics between eyes: R (3x3) and T (3x1), plus baseline.
  - Per-eye FOV (horizontal + vertical).
  - Image size, stride, format.
- Maintain a simplified Geometry-level struct for runtime:
  - Extend/align with `CalibrationData` (Matrix.h): fov, scale, eye_dist, axes, position.
  - Add per-eye lens parameters (LensPoly/LensDistortion) in Geometry for runtime use.

File format proposal
- JSON or .cfg (Key=Value) stored under device-specific path:
  - `share/calibration/<device-id>/stereo.json`
  - Include `device_id`, `timestamp`, and `source` (HMD/USB/video).
- Keep a lightweight "runtime cache" file for quick load: `.bin` or `.cfg` with only runtime fields.

Capture flow (tool)
- Live: show stereo feed, detect checkerboard/aruco, collect samples across depth/angle.
- Offline: import video files, auto-select frames with good coverage, then solve.
- Provide "known baseline" and/or "known target distance" inputs to constrain solve.

Geometry helpers to reuse
- `LookAtStereoAngles`, `AxesStereoMono`, `AxesMonoStereo`, `CalculateStereoTarget` in `Geometry/Util.*`.
- `CalibrationData` in `Geometry/Matrix.h` for fov/eye_dist/scale baseline storage.
