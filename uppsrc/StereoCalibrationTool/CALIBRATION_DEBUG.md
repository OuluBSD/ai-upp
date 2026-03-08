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

## Math Trace Logging

The solver now includes comprehensive trace logging to diagnose failures. Enable it via the "Verbose Math Log" checkbox in the UI, or it will be automatically enabled when the solver fails.

### How to Read the Trace

#### 1. Configuration Section

Shows input parameters:
- **Matches**: Number of match pairs
- **Eye distance**: Baseline in mm
- **Distance weight**: Relative weight of distance constraints vs reprojection (default 0.1)
- **Huber thresholds**: Outlier rejection thresholds for reprojection (px) and distance (mm)
- **Lock distortion**: Whether stage 1 (locked) or stage 2 (unlocked) distortion parameters

#### 2. Initial Parameters

Shows starting values for:
- **Distortion coefficients**: a, b, c, d (polynomial angle-to-pixel mapping)
- **Principal point**: cx, cy (optical center in pixels)
- **Right camera rotation**: yaw (outward angle), pitch, roll in radians

#### 3. Preflight Match Analysis

For each match, shows detailed pixel→ray→triangulation breakdown:

**Input pixels:**
```
Match #1:
  Input pixels: L=(320.5, 240.3), R=(310.2, 238.7)
  Image size: 640x480
  Measured distances: L=450.0 mm, R=455.0 mm
```

**Pixel to ray conversion:**
```
  Left eye:
    dx=0.50, dy=0.30 (relative to principal point)
    r_pix=0.583, roll=0.540 rad (30.96 deg)
    theta=0.0012 rad (0.07 deg) OK
```
- `dx, dy`: Pixel offset from principal point (cx, cy)
- `r_pix`: Radial distance from principal point
- `roll`: Angle around the principal point (atan2(dy, dx))
- `theta`: Polar angle from forward axis (computed via Newton iteration on distortion polynomial)
- **OK/FAILED**: Whether PixelToAngle Newton solver converged

**Ray directions:**
```
  Ray directions (camera frame):
    Left:  (0.001200, -0.000648, 0.999999)
    Right: (0.001150, -0.000620, 0.999999) (after rotation yaw=0.3491, pitch=-0.0873, roll=0.5236)
```
- Shows 3D ray direction vectors in camera coordinate frames
- Right ray includes rotation by yaw/pitch/roll parameters

**Triangulation:**
```
  Triangulation:
    3D point: (-1.2, 3.5, 452.3) mm
    Ray-ray distance: 0.05 mm
    Calculated distances: L=450.1 mm, R=454.8 mm
    Distance error L: 0.1 mm (measured 450.0 mm)
    Distance error R: -0.2 mm (measured 455.0 mm)
```
- **3D point**: Triangulated world position
- **Ray-ray distance**: Closest approach between rays (should be small; large values indicate poor geometry)
- **Calculated distances**: Distance from 3D point to camera centers
- **Distance error**: Difference from measured distance constraints

**Validation:**
```
  WARNING: Match #5 has unreasonable Z=5123.8 mm, marking invalid
  INVALID: Match #7 failed preflight checks
```

**Preflight summary:**
```
Preflight summary: 18/20 matches valid
```

#### 4. Optimization Iterations

Shows parameter evolution during Levenberg-Marquardt:

```
Iteration 1:
  Distortion: a=814.324, b=0.000, c=0.000, d=0.000 (locked)
  Principal point: cx=320.00, cy=240.00
  Right rotation: yaw=0.349066, pitch=-0.087266, roll=0.523599
  Total cost: 1234.567

Iteration 10:
  ...
  Total cost: 45.678
```

**Warnings during optimization:**
```
  WARNING: 3 matches produced non-finite residuals
```
- Indicates NaN or Inf values, usually from invalid parameters or degenerate geometry

#### 5. Optimization Result

```
========================================
Optimization Succeeded
========================================

Total iterations: 42
```

Or on failure:

```
========================================
OPTIMIZATION FAILED
========================================

Reason: Max function evaluations reached (2000)
Total iterations: 2000
Had non-finite residuals: yes
```

#### 6. Final Diagnostics

Shows solved parameters and quality metrics:

```
Final parameters:
  Distortion: a=815.234, b=-0.123, c=0.045, d=-0.002
  Principal point: cx=321.45, cy=239.87
  Right rotation: yaw=0.350123, pitch=-0.088456, roll=0.524789

Summary statistics:
  Reproj RMS: L=0.456 px, R=0.512 px
  Distance RMS: L=2.3 mm (18), R=2.8 mm (18)
  Average ray-ray distance: 0.08 mm
  Behind camera: L=0, R=0

Top 5 matches by total cost:
  Match #3 (total cost=12.345):
    Measured: L=(310.2, 245.6), R=(298.4, 244.1)
    Reproj:   L=(310.8, 245.3), R=(298.1, 244.5)
    Reproj error: L=0.67 px, R=0.51 px
    Distance error: L=3.2 mm, R=-2.8 mm
    3D point: (-5.2, 12.3, 456.7) mm
    Z-depth: L=456.8, R=457.1
```

### Common Failure Signatures

#### PixelToAngle Newton Overshoot / Wrong Branch

```
  WARNING: PixelToAngle did not converge for r=450.23, final theta=3.142
```
- Indicates distortion parameters are unreasonable or initial guess is poor
- Check that `a` is roughly `2 * image_width / π` for ~90° FOV

#### Principal Point Missing or Wrong

```
Match #1:
  Left eye:
    dx=320.50, dy=240.30
    r_pix=397.45, roll=0.643 rad
```
- If all matches have `dx ≈ image_width/2`, principal point is not set (defaulting to center)
- Real cameras often have off-center principal points; check calibration data

#### Combined Side-by-Side Coords Mistakenly Used

```
  WARNING: 15 right-eye coordinates are outside [0..1].
           Ensure right points are in the right sub-image coordinates (not combined).
```
- Right pixel X coordinates > image_width indicate combined coordinates
- Fix by subtracting half-width from right X coordinates

#### Y Sign Inverted

```
Match #1:
  Left eye:
    dx=10.2, dy=-220.3  (should be positive for bottom half of image)
```
- If `dy` signs are opposite to expected, Y axis convention is wrong
- Correct: `dy = v - cy` where v grows downward

#### dist_weight Too Low (Pretty but Wrong)

```
WARNING: Distance errors remain >50% while reprojection is small.
         Coordinate mapping/sign conventions are likely wrong.
```
- Solver minimized reprojection but ignored distance constraints
- Either increase `dist_weight` (default 0.1) or fix coordinate conventions

#### Too Few Valid Matches

```
FAILURE: Too few valid matches after preflight: 3/20 (need at least 5)
```
- Most matches failed PixelToAngle or produced invalid geometry
- Check principal point, distortion parameters, and coordinate conventions

#### Max Evals Reached

```
FAILURE: Max function evaluations reached (2000)
```
- Solver is stuck or diverging
- Try different initial parameters or check for bad matches

### Debugging Workflow

1. **Enable "Verbose Math Log" checkbox** in the UI
2. **Run Solve**
3. **Open Math tab** (automatically selected on failure)
4. **Check Preflight Analysis**:
   - Are dx/dy values reasonable (within ±half_image_size)?
   - Do theta values look sane (< π/2 for typical FOV)?
   - Are ray-ray distances small (< 5mm)?
5. **Check Iteration Progress**:
   - Is cost decreasing?
   - Are parameters staying in reasonable ranges?
6. **Check Final Diagnostics**:
   - Reproj RMS should be < 2 px
   - Distance RMS should be < 10 mm (if distance constraints provided)
   - Behind camera count should be 0
7. **Look for Warnings**:
   - Non-finite residuals → bad parameters
   - PixelToAngle failures → wrong distortion model or principal point
   - High distance errors with low reproj errors → coordinate conventions wrong

### PixelToAngle Newton Iteration Details

When `verbosity >= 3`, shows detailed Newton solver steps:

```
      PixelToAngle: r=123.45, initial theta=0.151679 (r/a)
        Newton it 0: theta=0.151679, f=0.123, df=814.324
        Newton it 1: theta=0.151528, f=0.001, df=814.312
        converged at it 1
```

- `f`: Residual (should approach 0)
- `df`: Derivative (should be non-zero and reasonable)
- Convergence typically takes 2-4 iterations for good parameters
