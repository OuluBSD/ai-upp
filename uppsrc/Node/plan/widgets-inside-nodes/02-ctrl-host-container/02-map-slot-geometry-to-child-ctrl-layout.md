# Map Slot Geometry to Child Ctrl Layout

## Purpose
Implement the geometry adapter that transforms Core slot rectangles into Ctrl child layout positions and clipping behavior.

This task is a Ctrl integration task that consumes a Core contract and must not move domain logic into Ctrl.

## Scope
- Convert world/view slot rects to Ctrl pixel bounds
- Apply viewport scroll/zoom updates to child layouts
- Handle visibility and clipping of off-screen slot widgets

## Out of Scope
- Computing slot geometry in Core
- Widget value binding semantics
- Focus arbitration

## Package Ownership
- `Node/Ctrl`

## Depends On
- `./uppsrc/Node/plan/widgets-inside-nodes/02-ctrl-host-container/01-implement-node-widget-host-container.md`
- `./uppsrc/Node/plan/core-scene-render/01-coordinate-system-spec/01-freeze-coordinate-space-contract.md`

## Touches
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`

## Implementation Notes
- All geometry math source-of-truth remains Core transforms
- Ctrl only applies transform results to child layout
- Avoid duplicated routing/hit geometry logic in Ctrl
- Consume Core contract: `Core Scene Coordinate Contract v1 and Core widget slot descriptor API`.
- Do not place graph model, persistence, routing, hit testing, or render-path generation in Ctrl; keep those in Node/Core.
- Separate responsibilities explicitly: Core owns slot/descriptor model; Ctrl owns concrete child-Ctrl hosting and parenting.

## Acceptance Criteria
- [ ] Child Ctrl bounds track slot geometry correctly through zoom/pan
- [ ] Off-screen slot widgets are clipped/hidden predictably
- [ ] No Core domain logic is reimplemented in Ctrl

## Suggested Validation
- manual interaction checks
- compile checks
