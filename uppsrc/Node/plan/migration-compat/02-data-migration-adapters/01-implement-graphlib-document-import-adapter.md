# Implement GraphLib Document Import Adapter

## Purpose
Implement Core adapter logic that imports legacy GraphLib document data into the new Node/Core document model.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Parse legacy GraphLib save structures and map entities to new model
- Map mixed legacy fields into persistent/runtime split with documented defaults
- Emit structured warnings for lossy conversions

## Out of Scope
- Ctrl file picker UX
- Legacy runtime interaction semantics
- New feature persistence outside legacy scope

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-model-document/04-persistence-implementation/01-implement-document-reader-writer.md`
- `./uppsrc/Node/plan/migration-compat/01-compatibility-inventory-freeze/02-define-compat-regression-fixtures.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Keep import in Core so Ctrl only supplies data streams
- Use compatibility matrix to decide preserve/adapt/drop behavior
- Add robust diagnostics for unsupported legacy constructs
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Legacy sample documents import into valid Core documents
- [ ] Lossy mappings are reported with warning categories
- [ ] Adapter remains independent from Ctrl

## Suggested Validation
- unit tests
- golden import tests
- compile checks
