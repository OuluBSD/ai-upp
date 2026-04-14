# Implement Scene Dirty Region Tracking

## Purpose
Implement Core-side dirty-region tracking to minimize full-scene rebuild costs after localized edits.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define invalidation graph for document/editor changes
- Track affected scene descriptor ranges and bounding regions
- Expose dirty-region metadata for Ctrl bridge consumption

## Out of Scope
- Ctrl repaint policy implementation details
- Spatial index optimization
- Layout incremental updates

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-scene-render/02-scene-descriptor-model/02-implement-scene-builder-baseline.md`
- `./uppsrc/Node/plan/performance-scaling/01-baseline-metrics-budgets/02-build-benchmark-harness.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`

## Implementation Notes
- Keep invalidation ownership in Core where scene semantics are known
- Provide simple read-only dirty outputs to Ctrl
- Avoid hidden global caches with implicit invalidation
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Dirty regions update correctly for localized edits
- [ ] Unchanged regions are not rebuilt unnecessarily
- [ ] Interface remains Ctrl-agnostic

## Suggested Validation
- unit tests for invalidation scenarios
- benchmark comparisons
- compile checks
