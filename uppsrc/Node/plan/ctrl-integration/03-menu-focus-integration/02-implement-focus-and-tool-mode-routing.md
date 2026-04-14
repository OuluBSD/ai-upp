# Implement Focus and Tool Mode Routing

## Purpose
Implement Ctrl-side focus and tool-mode routing that cooperates with Core editor state and command dispatch.

This task is a Ctrl integration task that consumes a Core contract and must not move domain logic into Ctrl.

## Scope
- Route focus enter/leave and active tool hints into Core state APIs
- Prevent shortcut conflicts when focus is outside the editor viewport
- Expose tool mode indicators without adding domain logic in Ctrl

## Out of Scope
- Widget hosting internals
- OS-level focus edge cases
- Scene rendering logic

## Package Ownership
- `Node/Ctrl`

## Depends On
- `./uppsrc/Node/plan/core-editor-commands/01-editor-state-model/01-define-editor-runtime-state.md`
- `./uppsrc/Node/plan/ctrl-integration/02-input-mapping-layer/02-map-keyboard-shortcuts-to-command-dispatch.md`

## Touches
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.h`

## Implementation Notes
- Keep focus policy in Ctrl but tool semantics in Core state contracts
- Avoid hidden Ctrl-only state that drifts from Core
- Document fallback behavior on focus loss
- Consume Core contract: `Core editor runtime state API`.
- Do not place graph model, persistence, routing, hit testing, or render-path generation in Ctrl; keep those in Node/Core.

## Acceptance Criteria
- [ ] Focus transitions maintain consistent Core editor state
- [ ] Tool mode routing does not bypass command APIs
- [ ] Ctrl remains free of document-domain logic

## Suggested Validation
- manual interaction checks
- compile checks
