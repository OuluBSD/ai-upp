# Freeze Command Interface Contract

## Purpose
Freeze the command interface used by Ctrl and tests to invoke all document/editor mutations through Core.

This task freezes a cross-package interface so both Core and Ctrl can evolve independently without boundary drift.

## Scope
- Define command envelope and typed payload conventions
- Define execution result and diagnostics contract
- Define undo token and transaction identifiers

## Out of Scope
- Implementing concrete command handlers
- Ctrl keybinding and menus
- Performance optimizations

## Package Ownership
- `Boundary`

## Depends On
- `./uppsrc/Node/plan/core-editor-commands/01-editor-state-model/01-define-editor-runtime-state.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Contract should support future batching and remote automation
- Avoid exposing document internals as mutable pointers to Ctrl
- Keep command API stable and versionable
- Freeze exact interface contract: `Core Command Contract v1 (command payload, execution context, undo token, and result envelope)`.
- Keep the contract minimal and stable so Core and Ctrl can compile and test independently.

## Acceptance Criteria
- [ ] Command contract document is complete and versioned
- [ ] Both Core and Ctrl can compile against the same interfaces
- [ ] No boundary ambiguity remains for mutation ownership

## Suggested Validation
- contract conformance tests
- compile checks
