# GPGPU Plan - ORB AMP Path

## Objective
Route ORB descriptor generation through backend dispatch and prepare the AMP path for incremental acceleration work.

## Current Status
- Implemented: `Orb::Describe` dispatch by selected backend.
- Implemented: CPU path preserved as baseline (`DescribeCpu`).
- Implemented: AMP and OpenGL entrypoints currently route to CPU for deterministic behavior.

## Next Tasks
1. Implement AMP-specific descriptor loop in `DescribeAmp`.
2. Add performance counters for CPU vs AMP path in ORB demo workloads.
3. Add parity test vectors to verify bit-identical descriptors between CPU and AMP paths.
4. Keep OpenGL entrypoint as stub until shader backend track starts.

## Acceptance Criteria
- No behavior regression in ORB matching outputs for existing demos.
- AMP path can be toggled live from WebcamCV backend menu.
- Fallback from unsupported backend is explicit and logged.
