# Implement Compatibility Warning Catalog

## Purpose
Implement a standardized warning/error catalog for migration adapters and compatibility shims.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define warning codes severities and message templates
- Integrate warnings into import and compatibility validation pipelines
- Define machine-readable warning payload for Ctrl display

## Out of Scope
- UI wording customization
- OS notification systems
- Performance benchmark tooling

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/migration-compat/02-data-migration-adapters/01-implement-graphlib-document-import-adapter.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`

## Implementation Notes
- Keep warning taxonomy in Core so all frontends display consistent diagnostics
- Do not embed UI formatting in Core warning payload
- Maintain stable codes for automation and tests
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Warning catalog covers known compatibility deviations
- [ ] Warnings are emitted deterministically for test fixtures
- [ ] No Ctrl dependency is introduced

## Suggested Validation
- unit tests
- golden warning output tests
- compile checks
