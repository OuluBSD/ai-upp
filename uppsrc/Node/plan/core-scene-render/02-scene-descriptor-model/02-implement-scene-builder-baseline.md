# Implement Scene Builder Baseline

## Purpose
Implement the baseline scene builder that converts core documents plus editor/runtime state into scene descriptors.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Implement document-to-scene traversal for nodes pins edges groups
- Implement baseline style assignment and label placement hooks
- Emit stable descriptor ordering for deterministic rendering and tests

## Out of Scope
- Bezier routing refinement
- Spatial indexing
- Ctrl input dispatch

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-scene-render/02-scene-descriptor-model/01-design-scene-descriptor-types.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/Renderer.cpp`

## Implementation Notes
- Use GraphLib rendering output as migration reference, not direct structure copy
- Keep builder stateless where possible to aid testing and caching
- Do not call Draw APIs directly from this builder
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.
- Keep rendering logic (scene assembly, style resolution, path generation) in Node/Core; Ctrl may only perform final `Ctrl::Paint` bridging.

## Acceptance Criteria
- [ ] Scene builder emits complete descriptors for representative graphs
- [ ] Output order is deterministic
- [ ] No Ctrl APIs are invoked

## Suggested Validation
- unit tests with golden descriptor snapshots
- compile checks
