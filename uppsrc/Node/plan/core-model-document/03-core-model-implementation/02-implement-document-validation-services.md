# Implement Document Validation Services

## Purpose
Implement reusable validation services that verify structural correctness, ID consistency, and reference validity across documents.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Implement validators for missing references and duplicate IDs
- Implement semantic checks for pin-edge compatibility constraints
- Expose validation report structure consumable by migration and IO layers

## Out of Scope
- UI rendering of validation results
- Command undo semantics
- Performance optimization work

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-model-document/03-core-model-implementation/01-implement-core-document-entities.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/Graph.cpp`

## Implementation Notes
- Validation must be pure Core logic with no UI assumptions
- Return rich diagnostics for migration compatibility tasks
- Keep validators independent from Ctrl event lifecycle
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Validation catches invalid edge endpoint references
- [ ] Validation report supports machine-readable error categories
- [ ] No Ctrl types are referenced

## Suggested Validation
- unit tests with invalid graph fixtures
- compile checks
