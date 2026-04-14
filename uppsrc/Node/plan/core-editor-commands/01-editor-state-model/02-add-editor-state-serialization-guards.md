# Add Editor State Serialization Guards

## Purpose
Add safeguards ensuring runtime editor state is never persisted through document IO accidentally.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Add compile-time and runtime guardrails in persistence APIs
- Add explicit whitelist of persistable model fields
- Add tests that reject accidental runtime-state writes

## Out of Scope
- Designing command stack APIs
- Implementing Ctrl clipboard behavior
- Schema evolution features

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-editor-commands/01-editor-state-model/01-define-editor-runtime-state.md`
- `./uppsrc/Node/plan/core-model-document/04-persistence-implementation/01-implement-document-reader-writer.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Treat this as a regression wall against GraphLib mixed-state pitfalls
- Surface clear diagnostics when runtime fields leak into persistence path
- Keep guard logic in Core IO layer
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Persistence tests prove runtime editor state is excluded
- [ ] Guard violations produce deterministic errors
- [ ] No Ctrl dependencies are introduced

## Suggested Validation
- unit tests
- golden file tests
- compile checks
