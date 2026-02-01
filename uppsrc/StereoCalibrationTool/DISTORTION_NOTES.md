# Stage A Distortion Model - Design Notes

## Current Problems (as of facd7ebf3)

1. **Unstable parameter range**: barrel_strength outside [-3, +3] causes corruption/crashes
2. **Confusing semantics**: Negative values seem to "straighten more" but meaning is unclear
3. **Polynomial instability**: Current model uses LensPoly with polynomial inversion that can fail
4. **No visual validation**: No way to verify distortion correction actually straightens lines

## Proposed Solution

### 1. Normalized Radial Distortion Model

Use simple radial distortion in normalized coordinates:
```
r_d = r_u * (1 + k1 * r_u^2)
```

Where:
- `r_u` = undistorted radius in normalized coords
- `r_d` = distorted radius
- `k1` = radial distortion coefficient

### 2. UI Semantics

- Replace "barrel_strength" with "undistort_strength"
- Range: [0, 3] (0 = no correction, higher = more correction)
- Internally: `k1 = -undistort_strength * scale_factor`
- Increasing slider → lines get straighter

### 3. Implementation Strategy

**Undistortion (preview with intrinsics ON)**:
- For each output pixel (u_out, v_out):
  1. Normalize: `(x_u, y_u) = ((u_out - cx)/f, (v_out - cy)/f)`
  2. Compute `r_u = sqrt(x_u^2 + y_u^2)`
  3. Apply forward distortion: `r_d = r_u * (1 + k1 * r_u^2)`
  4. Scale back: `(x_d, y_d) = (x_u, y_u) * (r_d / r_u)`
  5. Denormalize: `(u_in, v_in) = (x_d * f + cx, y_d * f + cy)`
  6. Sample input image at (u_in, v_in)

**Safety**:
- Clamp `r_u` to max based on FOV
- Guard all intermediate values for NaN/inf
- Fallback to identity if any step fails

### 4. Validation

Add CLI command:
```bash
bin/StereoCalibrationTool --stagea_distortion_selfcheck [--strength 0..3]
```

Generates synthetic grid, applies distortion+undistortion, measures straightness metric.

## Migration Plan

1. ✅ Document current issues
2. ⏳ Implement new simple radial model alongside old LensPoly
3. ⏳ Add distortion selfcheck CLI
4. ⏳ Update Stage A UI to use new model
5. ⏳ Deprecate/guard old unstable polynomial path
6. ⏳ Update regression tests to verify stability

## References

- Brown's distortion model: r_d = r_u * (1 + k1*r_u^2 + k2*r_u^4 + ...)
- OpenCV undistort: Uses iterative refinement for inversion
- Our approach: Use forward model in inverse mapping (no iteration needed)
