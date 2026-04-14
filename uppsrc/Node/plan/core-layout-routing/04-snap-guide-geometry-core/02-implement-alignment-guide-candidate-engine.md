# Implement Alignment Guide Candidate Engine

## Purpose
Implement Core computations that produce alignment-guide candidates from scene/document geometry for Ctrl to display.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Compute horizontal and vertical guide candidates near moving entities
- Define scoring and tie-break rules for guide selection
- Expose candidate metadata for optional rendering and snapping

## Out of Scope
- Rendering guide lines in Ctrl
- UI toggles for guides
- Drag event handling

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-layout-routing/04-snap-guide-geometry-core/01-implement-grid-snap-kernel.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Keep guide generation as headless geometry service in Core
- Ctrl should only render candidate guides and forward user actions
- Document complexity limits for large graphs
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Guide candidate API returns stable ordered candidates
- [ ] Candidates can drive snapping without Ctrl-side geometry recompute
- [ ] Core/Ctrl split is preserved

## Suggested Validation
- unit tests for candidate ranking
- compile checks
