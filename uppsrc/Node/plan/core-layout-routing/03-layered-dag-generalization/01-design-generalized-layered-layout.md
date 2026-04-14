# Design Generalized Layered Layout

## Purpose
Design a generalized layered layout that removes strict tree assumptions present in legacy OrderedTree behavior.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define layered layout stages and constraints for arbitrary DAGs
- Define crossing-minimization and spacing strategy hooks
- Define compatibility mode matching legacy tutorial outputs where needed

## Out of Scope
- Implementing final algorithm code
- Ctrl-side controls for layout settings
- Performance tuning

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-layout-routing/01-legacy-algorithm-audit-port/02-port-tree-and-shortestpath-algorithms.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/OrderedTree.cpp`

## Implementation Notes
- Document trade-offs and deterministic defaults
- Keep layout outputs as Core geometry state consumable by scene builder
- Avoid embedding UI settings structures from Ctrl
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Design document covers DAG not just perfect trees
- [ ] Compatibility mode behavior is explicit
- [ ] Core/Ctrl separation is preserved

## Suggested Validation
- design review
- golden fixture planning
