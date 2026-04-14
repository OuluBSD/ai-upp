# Freeze Coordinate Space Contract

## Purpose
Freeze the coordinate-space contract used by scene building, hit testing, and Ctrl paint/input adapters so all tracks share identical transform semantics.

This task freezes a cross-package interface so both Core and Ctrl can evolve independently without boundary drift.

## Scope
- Define world view screen spaces and transform direction
- Define origin axis and scaling conventions including zoom and pan
- Define precision and rounding policy for paint and hit tests

## Out of Scope
- Implementing renderer internals
- Ctrl event handlers
- Layout algorithm updates

## Package Ownership
- `Boundary`

## Depends On
- `./uppsrc/Node/plan/core-model-document/01-domain-boundary-freeze/01-define-stable-id-model.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphLib.h`

## Implementation Notes
- Document transform invariants as contract tests
- Map current GraphLib ToImage/FromImage behavior into explicit rules
- Guard against Ctrl-side duplicate transform logic
- Freeze exact interface contract: `Core Scene Coordinate Contract v1 (world/view/screen transforms and origin conventions)`.
- Keep the contract minimal and stable so Core and Ctrl can compile and test independently.
- Keep rendering logic (scene assembly, style resolution, path generation) in Node/Core; Ctrl may only perform final `Ctrl::Paint` bridging.

## Acceptance Criteria
- [ ] Contract includes formulas and edge-case rules
- [ ] Core and Ctrl agree on one transform definition
- [ ] No unresolved ambiguity remains for zoom/pan math

## Suggested Validation
- contract tests
- compile checks for both packages
