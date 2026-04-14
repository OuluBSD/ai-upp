# Implement Document Reader/Writer

## Purpose
Implement schema-compliant reader and writer services for Node document persistence in Node/Core.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Implement encode/decode paths for schema V1 entities
- Integrate validation and version dispatch during load
- Define deterministic ordering rules for stable output

## Out of Scope
- Ctrl file dialog integration
- Legacy GraphLib adapter code
- Undo/redo command history persistence

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-model-document/03-core-model-implementation/02-implement-document-validation-services.md`
- `./uppsrc/Node/plan/core-model-document/02-document-schema-v1/02-define-versioning-and-migration-rules.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Reader/writer APIs should be stream-based and headless
- Keep IO error handling in Core with structured diagnostics
- Do not serialize transient editor or Ctrl state
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Round-trip load-save-load preserves document semantics
- [ ] Invalid documents return structured errors
- [ ] Module compiles without Ctrl dependencies

## Suggested Validation
- unit tests for round-trip
- golden file tests
- compile checks
