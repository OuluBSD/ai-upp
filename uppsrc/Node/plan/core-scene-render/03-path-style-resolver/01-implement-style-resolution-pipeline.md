# Implement Style Resolution Pipeline

## Purpose
Implement style resolution in Core so visual defaults, overrides, and themes are resolved before Ctrl paint bridging.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define style source precedence rules
- Implement resolved style cache keys for scene items
- Support GraphLib-equivalent node edge group style attributes

## Out of Scope
- Theme editor UI
- Final performance tuning
- OS-specific font handling

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-scene-render/02-scene-descriptor-model/02-implement-scene-builder-baseline.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/Graph.h`

## Implementation Notes
- Keep style policy pure core logic and avoid Ctrl theme objects
- Expose resolved style outputs through scene descriptors only
- Document fallback behavior for missing style properties
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.
- Keep rendering logic (scene assembly, style resolution, path generation) in Node/Core; Ctrl may only perform final `Ctrl::Paint` bridging.

## Acceptance Criteria
- [ ] Resolved styles are deterministic for same inputs
- [ ] Override precedence is covered by tests
- [ ] No Ctrl-owned style classes are introduced

## Suggested Validation
- unit tests for style precedence
- golden descriptor tests
- compile checks
