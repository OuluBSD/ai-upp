# Freeze Core Paint Command Contract

## Purpose
Freeze the contract used by Core to emit paint commands and by Ctrl to render them, preventing geometry logic from leaking into Ctrl.

This task freezes a cross-package interface so both Core and Ctrl can evolve independently without boundary drift.

## Scope
- Define command set for fills strokes text images and clipping primitives
- Define command payload precision and ordering guarantees
- Define versioning and extension points for future commands

## Out of Scope
- Implementing concrete Ctrl paint adapter code
- Platform-specific draw optimization
- Widget hosting overlays

## Package Ownership
- `Boundary`

## Depends On
- `./uppsrc/Node/plan/core-scene-render/03-path-style-resolver/02-implement-core-edge-path-generator.md`
- `./uppsrc/Node/plan/core-scene-render/04-hit-geometry-core/01-define-hit-query-api.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphLib.h`

## Implementation Notes
- Contract must be minimal yet sufficient for migrated GraphLib visuals
- All edge path and style decisions must already be resolved in Core output
- Avoid introducing callback-based backchannels into Ctrl
- Freeze exact interface contract: `Core Paint Command Contract v1 (scene draw command schema consumed by Ctrl::Paint bridge)`.
- Keep the contract minimal and stable so Core and Ctrl can compile and test independently.
- Keep rendering logic (scene assembly, style resolution, path generation) in Node/Core; Ctrl may only perform final `Ctrl::Paint` bridging.

## Acceptance Criteria
- [ ] Contract document lists all commands and invariants
- [ ] Core and Ctrl compile against same contract types
- [ ] No unresolved responsibility overlap remains

## Suggested Validation
- contract conformance tests
- compile checks for both packages
