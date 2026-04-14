# Implement Command Dispatch Kernel

## Purpose
Implement the Core command dispatch kernel that routes commands to handlers and applies validated mutations.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Implement dispatch table and command registration policy
- Implement execution context assembly with validation hooks
- Implement standard error/result reporting

## Out of Scope
- Undo history storage internals
- Ctrl shortcut mapping
- Layout recomputation policy

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-editor-commands/02-command-protocol/01-freeze-command-interface-contract.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Keep mutation logic in handlers invoked by Core dispatch only
- Do not expose ad-hoc mutators to Ctrl
- Provide deterministic handler ordering and diagnostics
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Dispatch kernel executes known commands and rejects unknown ones cleanly
- [ ] Validation hooks run before mutation commit
- [ ] No Ctrl dependencies are present

## Suggested Validation
- unit tests for dispatch behavior
- compile checks
