# Define Versioning and Migration Rules

## Purpose
Define forward/backward compatibility policy for document versions and migration behavior so future format changes are controlled.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define version markers and compatibility guarantees
- Define migration hook contract for V1-to-future upgrades
- Define failure policy for unsupported versions

## Out of Scope
- Writing concrete migrator code
- Implementing UI error dialogs
- Adding new runtime features

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-model-document/02-document-schema-v1/01-specify-document-schema-v1.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Keep migration policy in Core to avoid Ctrl ownership leaks
- Specify deterministic behavior for unknown fields and partial documents
- Add compatibility notes for GraphLib save format import adapters
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Versioning policy includes read/write guarantees
- [ ] Migration hook signatures are frozen for V1
- [ ] Unsupported-version behavior is explicitly specified

## Suggested Validation
- unit tests for version dispatch
- golden file tests for compatibility scenarios
