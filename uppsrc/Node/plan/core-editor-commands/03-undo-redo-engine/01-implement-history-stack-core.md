# Implement History Stack Core

## Purpose
Implement undo/redo history data structures and policies in Core, independent from Ctrl shortcuts.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Implement push/pop and cursor behavior for undo redo
- Implement transaction boundaries and clear-on-branch behavior
- Implement history capacity policy hooks

## Out of Scope
- Ctrl keybinding integration
- Command merge heuristics
- Persistence of history state

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-editor-commands/02-command-protocol/02-implement-command-dispatch-kernel.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- History engine should operate on command results/tokens from Core dispatch
- Do not allow Ctrl to mutate history internals directly
- Expose narrow API for invoke undo redo
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Undo/redo stack operations are deterministic
- [ ] Branching behavior is documented and tested
- [ ] No Ctrl references exist in history module

## Suggested Validation
- unit tests for history semantics
- compile checks
