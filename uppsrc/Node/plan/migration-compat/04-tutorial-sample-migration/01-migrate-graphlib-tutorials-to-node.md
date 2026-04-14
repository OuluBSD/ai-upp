# Migrate GraphLib Tutorials to Node

## Purpose
Migrate tutorial coverage from GraphLib1-4 to new Node packages to validate compatibility and expose boundary regressions.

This task is a Ctrl integration task that consumes a Core contract and must not move domain logic into Ctrl.

## Scope
- Create updated tutorial samples using Node/Core and Node/Ctrl APIs
- Preserve key scenarios: tree layout ordered dependencies spring layout and Dijkstra highlighting
- Add notes highlighting intentional behavior differences

## Out of Scope
- Implementing brand-new tutorial features beyond legacy scope
- Performance profiling
- Widget hosting demos unless explicitly required

## Package Ownership
- `Node/Ctrl`

## Depends On
- `./uppsrc/Node/plan/migration-compat/03-facade-shim-layer/02-implement-transitional-ctrl-facade.md`
- `./uppsrc/Node/plan/core-layout-routing/01-legacy-algorithm-audit-port/02-port-tree-and-shortestpath-algorithms.md`
- `./uppsrc/Node/plan/migration-compat/01-compatibility-inventory-freeze/02-define-compat-regression-fixtures.md`

## Touches
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./tutorial/GraphLib1/...`
- `legacy reference only: ./tutorial/GraphLib4/...`

## Implementation Notes
- Treat tutorials as integration regression assets not product features
- Keep domain logic in Core and ensure tutorials call through Ctrl/Core contracts only
- Avoid hidden fallback to legacy GraphLib runtime
- Consume Core contract: `Transitional Core/Ctrl Facade APIs`.
- Do not place graph model, persistence, routing, hit testing, or render-path generation in Ctrl; keep those in Node/Core.

## Acceptance Criteria
- [ ] Migrated tutorial set builds and runs against Node packages
- [ ] Tutorial parity check explicitly covers `GraphLib1` through `GraphLib4` behavior expectations
- [ ] Core/Ctrl boundary is demonstrably preserved in sample code
- [ ] Compatibility matrix coverage improves

## Suggested Validation
- tutorial/sample checks
- compile checks
- manual interaction checks
