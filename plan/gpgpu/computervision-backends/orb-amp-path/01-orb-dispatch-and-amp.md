# GPGPU Plan - ORB AMP Path

## Objective
Route ORB descriptor generation through backend dispatch and prepare the AMP path for incremental acceleration work.

## Current Status
- Implemented: `Orb::Describe` dispatch by selected backend.
- Implemented: CPU path preserved as baseline (`DescribeCpu`).
- Implemented: AMP-specific descriptor path via `parallel_for_each` in `DescribeAmp`.
- Implemented: OpenGL entrypoint remains stub and routes to CPU for deterministic behavior.
- Implemented: explicit backend fallback resolution + one-time log when unsupported backend is requested.
- Implemented: ORB backend-local descriptor timing counters (`last_describe_ms` and per-backend last time).

## Next Tasks
1. Add parity test vectors to verify bit-identical descriptors between CPU and AMP paths.
2. Expose ORB timing counters in a dedicated stats structure (for stable external API use).
3. Keep OpenGL entrypoint as stub until shader backend track starts.

## Acceptance Criteria
- No behavior regression in ORB matching outputs for existing demos.
- AMP path can be toggled live from WebcamCV backend menu.
- Fallback from unsupported backend is explicit and logged.
