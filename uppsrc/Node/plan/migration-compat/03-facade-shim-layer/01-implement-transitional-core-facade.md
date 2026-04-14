# Implement Transitional Core Facade

## Purpose
Implement a transitional facade in Core that offers migration-friendly APIs while internally using the new Node architecture.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define facade methods covering key GraphLib model operations
- Route facade calls through Core command/model services
- Tag facade methods with deprecation and migration notes

## Out of Scope
- Ctrl menu wiring
- Legacy tutorial code changes
- Long-term public API finalization

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/migration-compat/02-data-migration-adapters/01-implement-graphlib-document-import-adapter.md`
- `./uppsrc/Node/plan/core-editor-commands/02-command-protocol/01-freeze-command-interface-contract.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/Graph.h`

## Implementation Notes
- Facade should reduce migration friction but must not reintroduce legacy coupling
- Avoid exposing internal mutable structures directly
- Document sunset strategy for shim APIs
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Facade supports prioritized compatibility operations
- [ ] Facade methods preserve Core ownership boundaries
- [ ] Deprecation markers are documented

## Suggested Validation
- unit tests
- compile checks
