# Implement Incremental Path Recompute

## Purpose
Implement incremental edge path recomputation in Core so only affected routes are regenerated after local edits.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define dependency tracking from node/pin changes to impacted edges
- Implement selective path invalidation and recompute
- Add correctness checks against full recompute baseline

## Out of Scope
- Ctrl repaint logic
- Layout algorithm generalization
- Widget hosting

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-layout-routing/02-routing-policy-layer/02-implement-default-bezier-routing-policy.md`
- `./uppsrc/Node/plan/performance-scaling/02-scene-cache-dirty-regions/01-implement-scene-dirty-region-tracking.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`

## Implementation Notes
- Keep path recompute policy in Core with routing ownership
- Expose affected-path sets to scene cache/bridge layers
- Avoid path generation fallback in Ctrl
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.
- Keep rendering logic (scene assembly, style resolution, path generation) in Node/Core; Ctrl may only perform final `Ctrl::Paint` bridging.

## Acceptance Criteria
- [ ] Localized edits trigger partial path recompute only
- [ ] Incremental results match full recompute outputs
- [ ] Performance improves on benchmark scenarios

## Suggested Validation
- unit tests
- golden path comparisons
- benchmark runs
