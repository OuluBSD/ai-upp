# Define Hit Query API

## Purpose
Define the Core hit-query API that operates on scene geometry and returns deterministic pick results for nodes pins edges and groups.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define pick query inputs including point rect and tolerance
- Define pick result ordering and tie-break rules
- Define APIs for hover and selection workflows

## Out of Scope
- Ctrl mouse event handling
- Spatial index optimization
- Undo/redo command execution

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-scene-render/02-scene-descriptor-model/01-design-scene-descriptor-types.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/Renderer.cpp`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Keep hit semantics in Core so Ctrl cannot diverge behavior by platform
- Codify precedence rules currently implicit in GraphNodeCtrl and Renderer scans
- Prepare API for future acceleration structures
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.
- Keep rendering logic (scene assembly, style resolution, path generation) in Node/Core; Ctrl may only perform final `Ctrl::Paint` bridging.

## Acceptance Criteria
- [ ] Hit API supports point and marquee use-cases
- [ ] Result ordering rules are explicit and testable
- [ ] No Ctrl dependency is present

## Suggested Validation
- unit tests for hit ordering
- compile checks
