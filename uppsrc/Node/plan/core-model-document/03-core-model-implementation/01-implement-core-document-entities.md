# Implement Core Document Entities

## Purpose
Implement the normalized document entity types and containers in Node/Core according to the frozen model/schema decisions.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Create entity structs/classes for graph node pin edge group with stable IDs
- Implement document container APIs for add remove update and lookup
- Implement invariant checks for references and ownership

## Out of Scope
- Serialization readers/writers
- Ctrl control classes
- Layout/routing algorithm logic

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-model-document/01-domain-boundary-freeze/02-separate-persistent-vs-runtime-state.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/Graph.cpp`

## Implementation Notes
- Prefer composition over inheritance patterns used by GraphLayout<T>
- Avoid embedding selection or hover state in persistent entities
- Keep APIs deterministic and suitable for unit testing
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Core document entities compile without Ctrl dependencies
- [ ] Reference integrity checks exist for key mutations
- [ ] API supports all entity kinds required by migrated GraphLib features

## Suggested Validation
- unit tests for CRUD and invariants
- compile checks for Node/Core
