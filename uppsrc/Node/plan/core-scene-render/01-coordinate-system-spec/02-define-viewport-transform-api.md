# Define Viewport Transform API

## Purpose
Define and stub the Core API that exposes viewport transforms and conversion helpers to rendering and interaction subsystems.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define transform object types and helper operations
- Define conversion APIs for points rects and path anchors
- Define deterministic behavior under extreme zoom levels

## Out of Scope
- Ctrl widget scrolling implementation
- Scene cache invalidation
- Hit test indexing

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-scene-render/01-coordinate-system-spec/01-freeze-coordinate-space-contract.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphLib.h`

## Implementation Notes
- Keep transform operations pure and side-effect free
- Provide API that Ctrl can consume without reimplementing math
- Include guards for overflow and precision drift
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.
- Keep rendering logic (scene assembly, style resolution, path generation) in Node/Core; Ctrl may only perform final `Ctrl::Paint` bridging.

## Acceptance Criteria
- [ ] API supports all required world-view-screen conversions
- [ ] Extreme zoom cases are specified and tested
- [ ] No Ctrl dependencies are added

## Suggested Validation
- unit tests for transform math
- compile checks
