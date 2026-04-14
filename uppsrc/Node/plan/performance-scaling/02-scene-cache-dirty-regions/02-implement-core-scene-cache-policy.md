# Implement Core Scene Cache Policy

## Purpose
Implement scene caching policy in Core using dirty-region signals and stable cache keys.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define cache keying for document state editor state and viewport-relevant inputs
- Implement cache eviction and memory guard rules
- Integrate cache hits/misses into benchmark reporting

## Out of Scope
- Ctrl draw command playback optimization
- Hit-test indexing
- Layout algorithms

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/performance-scaling/02-scene-cache-dirty-regions/01-implement-scene-dirty-region-tracking.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`

## Implementation Notes
- Cache behavior must be deterministic and diagnosable
- Keep cache internals in Core to prevent duplication in Ctrl
- Avoid storing Ctrl-owned objects in cache entries
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Cache returns valid scene snapshots for unchanged inputs
- [ ] Cache invalidates correctly on relevant edits
- [ ] Benchmarks show measurable improvement

## Suggested Validation
- unit tests
- benchmark runs
- compile checks
