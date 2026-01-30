# Calibration Debug Guide

## Coordinate Conventions

- **Pixel origin**: (0, 0) is the top-left of each **sub-image**.
- **Right image coordinates**: always in the right sub-image coordinate system (0..width-1), not combined side-by-side coordinates.
- **Principal point**: `cx, cy` are in pixels.
- **Pixel to ray**:
  - `dx = u - cx`, `dy = v - cy` (v grows downward).
  - `roll = atan2(dy, dx)`.
  - Direction uses `x = sin(theta) * cos(roll)`, `y = -sin(theta) * sin(roll)`, `z = ±cos(theta)` (sign depends on forward axis).
- **Right camera pose**:
  - Right camera rotation is `yaw (outward) + pitch + roll` relative to the left.
  - Baseline length is fixed by `eye_dist`.

## Diagnostics

After each solve, the report includes:

- **Reprojection RMS per camera** (px).
- **Distance RMS per camera** (mm).
- **Points behind camera** (% with Z < 0).
- **Baseline** (fixed).
- **Disparity–depth consistency** (RMS %).
- **Top residual offenders** (largest combined reprojection + distance errors).

If **distance errors stay >50%** while reprojection is small, the solver prints a warning that coordinate mapping/sign conventions are likely wrong.

## Common Failure Modes

- **Combined coordinates**: right points stored in full side-by-side coordinates instead of the right sub-image.
- **atan2 swap**: using `atan2(dx, dy)` instead of `atan2(dy, dx)`.
- **Y sign flip**: forgetting that image Y grows downward.
- **Principal point mismatch**: assuming center when the camera is off-center.
- **Under-parameterized extrinsics**: solving only yaw when pitch/roll is present.
