# Add Hit Geometry Fixtures

## Purpose
Create deterministic geometry fixtures for hit-query regression tests across overlapping entities and edge proximity cases.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Add fixtures for node-pin overlap and dense edge intersections
- Add tolerance boundary fixtures for edge proximity picking
- Add marquee fixtures for mixed entity selections

## Out of Scope
- Ctrl event simulation
- Spatial index implementation
- Performance benchmark harness

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-scene-render/04-hit-geometry-core/01-define-hit-query-api.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Use fixture naming that maps to interaction scenarios for later command tests
- Capture known GraphLib ambiguities as explicit expected outcomes
- Keep fixtures independent from Ctrl coordinate events
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.
- Keep rendering logic (scene assembly, style resolution, path generation) in Node/Core; Ctrl may only perform final `Ctrl::Paint` bridging.

## Acceptance Criteria
- [ ] Fixture suite covers key pick ambiguity cases
- [ ] Golden expected pick outputs are checked in tests
- [ ] Fixtures are reusable by command integration phases

## Suggested Validation
- golden file tests
- unit tests
- compile checks
