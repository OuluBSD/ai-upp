# Implement Spatial Index Structures

## Purpose
Implement spatial index structures for fast hit-query operations on scene geometry at larger graph sizes.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Select and implement index structures for node pin and edge geometries
- Define index build/update APIs aligned with scene descriptor lifecycle
- Expose query APIs compatible with existing hit-query contract

## Out of Scope
- Ctrl pointer event handling
- Selection command semantics
- Cache eviction policies

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-scene-render/04-hit-geometry-core/01-define-hit-query-api.md`
- `./uppsrc/Node/plan/performance-scaling/01-baseline-metrics-budgets/02-build-benchmark-harness.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`

## Implementation Notes
- Keep hit semantics unchanged while replacing underlying search complexity
- Avoid any Ctrl coordinate policy embedded in index code
- Document update complexity trade-offs
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Spatial index APIs support current hit-query contract
- [ ] Index query performance improves on large fixtures
- [ ] No Ctrl dependency is introduced

## Suggested Validation
- unit tests
- benchmark runs
- compile checks
