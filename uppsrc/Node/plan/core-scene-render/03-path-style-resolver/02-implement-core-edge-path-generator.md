# Implement Core Edge Path Generator

## Purpose
Implement edge path generation in Core using routing policies and coordinate transforms, replacing ad-hoc drawing logic.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Implement default bezier-like path generation with anchor selection
- Implement label anchor and arrowhead geometry outputs
- Expose route metadata for hit testing and debug inspection

## Out of Scope
- Ctrl paint stroke calls
- Orthogonal routing policy implementation
- Edge interaction UI

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-scene-render/03-path-style-resolver/01-implement-style-resolution-pipeline.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/Renderer.cpp`

## Implementation Notes
- Migrate geometry intent from GraphLib Renderer while removing Draw-time coupling
- Return path primitives in scene descriptor format
- Keep all path math in Core and never in Ctrl
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.
- Keep rendering logic (scene assembly, style resolution, path generation) in Node/Core; Ctrl may only perform final `Ctrl::Paint` bridging.

## Acceptance Criteria
- [ ] Edge paths and arrow geometry are produced entirely by Core
- [ ] Generated paths are consumable by paint bridge without extra geometry logic
- [ ] Path output is testable via golden fixtures

## Suggested Validation
- golden file tests for path output
- unit tests for anchor logic
- compile checks
