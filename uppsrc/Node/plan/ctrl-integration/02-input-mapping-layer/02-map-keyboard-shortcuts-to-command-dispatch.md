# Map Keyboard Shortcuts to Command Dispatch

## Purpose
Implement keyboard shortcut binding in Ctrl as a thin translation layer to Core command and history APIs.

This task is a Ctrl integration task that consumes a Core contract and must not move domain logic into Ctrl.

## Scope
- Define shortcut table for select all delete undo redo copy paste focus
- Route key events into Core command dispatch and history APIs
- Provide configurable binding map extension points

## Out of Scope
- Adding command business logic to Ctrl
- OS clipboard implementation details
- Menu rendering

## Package Ownership
- `Node/Ctrl`

## Depends On
- `./uppsrc/Node/plan/core-editor-commands/03-undo-redo-engine/01-implement-history-stack-core.md`
- `./uppsrc/Node/plan/ctrl-integration/02-input-mapping-layer/01-map-pointer-events-to-core-commands.md`

## Touches
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Shortcut handling must never mutate model directly in Ctrl
- Keep command IDs and payload building consistent with Core protocol
- Document unsupported key combos and conflict policy
- Consume Core contract: `Core Command Contract v1 and Core undo/redo API`.
- Do not place graph model, persistence, routing, hit testing, or render-path generation in Ctrl; keep those in Node/Core.

## Acceptance Criteria
- [ ] Shortcut table executes expected Core commands
- [ ] Undo/redo shortcuts call Core history API only
- [ ] No domain logic is introduced in Ctrl

## Suggested Validation
- manual interaction checks
- compile checks
- shortcut dispatch tests
