# Add Command Merge and Transaction Rules

## Purpose
Define and implement command coalescing and transaction grouping rules for interactive editing workloads.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define merge rules for drag move and repeated edits
- Define explicit transaction begin commit rollback APIs
- Add tests for grouped undo behavior

## Out of Scope
- UI gesture recognition details
- Performance benchmarks
- Clipboard payload formatting

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-editor-commands/03-undo-redo-engine/01-implement-history-stack-core.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Keep grouping semantics in Core command layer not in Ctrl event code
- Document non-mergeable command classes
- Prevent partial rollback states
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Grouped operations undo/redo as single units where expected
- [ ] Non-mergeable commands remain isolated
- [ ] Tests cover drag-like repetitive command series

## Suggested Validation
- unit tests
- manual interaction checks with mock command streams
- compile checks
