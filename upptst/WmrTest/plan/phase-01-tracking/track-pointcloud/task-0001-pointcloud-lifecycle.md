Task 0001 - Pointcloud lifecycle

Checklist
- Define point insertion, update, and pruning rules.
- Decide per-point confidence model and decay.
- Define octree resolution strategy and limits.

Deliverables
- A point lifecycle spec and parameters.

Point lifecycle spec
- Insert:
  - Triangulated stereo matches with sufficient disparity + low reprojection error.
  - Require descriptor match score <= threshold and min parallax.
- Update:
  - Track across frames (temporal) with descriptor + optical flow gate.
  - Re-triangulate when baseline/parallax improves; blend position with Kalman/EMA.
- Prune:
  - Remove if not observed for N frames or confidence below threshold.
  - Drop if reprojection error spikes for M consecutive frames.

Confidence model
- Start with confidence = 1.0 on insertion.
- Increase with consistent reobservations; decrease with missed frames and high error.
- Clamp [0,1]; mark inactive below 0.2 and remove below 0.05.

Octree strategy
- Use metric resolution tiers (e.g., 1cm near, 5cm mid, 20cm far).
- Cap nodes by max points per node; split when > K points.
- Periodically rebuild or lazy-remove dead points to avoid fragmentation.
