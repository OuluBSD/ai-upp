# Task 0200 - Localization Application in Synthetic Sim

## Goal
Ensure localization transforms are applied to the correct pointcloud (HMD/localized) and that matching accuracy is visible.

## Scope
- Apply localization transform to `hmd_pointcloud` (not `sim_observation`).
- `sim_observation` should remain raw camera-space.
- Ensure localized pointcloud matches reference pointcloud at ~99.9% for the synthetic case.

## Implementation Notes
- Confirm transform order: raw -> localization -> world.
- Provide a debug metric for mismatch (mean/median distance).
- Ensure effects visibility toggles only the transform, not base data.

## Success Criteria
- After "Run localization", `hmd_pointcloud` visually overlaps `sim_pointcloud`.
- Toggling localization effect on/off changes only the localized render.
- Automated sanity check logs accuracy >= 99.9%.

## Status
- Pending
