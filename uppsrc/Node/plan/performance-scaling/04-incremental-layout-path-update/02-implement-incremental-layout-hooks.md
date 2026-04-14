# Implement Incremental Layout Hooks

## Purpose
Implement hooks for incremental layout updates so layout engines can avoid global recomputation when command impact is local.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define impact classification API from commands to layout domains
- Implement optional incremental entrypoints in layout engines
- Add fallback path to full layout when impact is broad

## Out of Scope
- Ctrl-side layout controls
- Routing policy design
- Persistence schema changes

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-layout-routing/03-layered-dag-generalization/02-implement-layered-layout-and-fixtures.md`
- `./uppsrc/Node/plan/performance-scaling/04-incremental-layout-path-update/01-implement-incremental-path-recompute.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`

## Implementation Notes
- Incremental layout must preserve deterministic outcomes where possible
- Keep all layout decisions in Core and expose only results to Ctrl
- Document fallback thresholds and correctness trade-offs
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Incremental hooks operate for targeted edit classes
- [ ] Fallback to full layout remains correct and explicit
- [ ] Core/Ctrl boundary is maintained

## Suggested Validation
- unit tests
- benchmark runs
- compile checks
