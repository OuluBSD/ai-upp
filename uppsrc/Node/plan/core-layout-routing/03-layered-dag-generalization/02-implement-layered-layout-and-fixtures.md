# Implement Layered Layout and Fixtures

## Purpose
Implement the generalized layered layout and add fixtures that prove behavior on varied DAG structures.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Implement layout pipeline stages based on approved design
- Add fixtures for sparse dense and branching DAG topologies
- Add regression fixtures for legacy ordered-tree samples

## Out of Scope
- Ctrl layout configuration UI
- Orthogonal edge routing
- Undo command integration

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-layout-routing/03-layered-dag-generalization/01-design-generalized-layered-layout.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./tutorial/GraphLib2/main.cpp`

## Implementation Notes
- Keep all coordinate decisions in Core and expose through layout result objects
- Do not perform any drawing or event handling in layout module
- Use deterministic ordering to stabilize fixture outputs
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Layered layout works on non-tree DAG fixtures
- [ ] Legacy sample parity expectations are documented
- [ ] Module remains independent from Ctrl

## Suggested Validation
- unit tests
- golden layout fixtures
- compile checks
