# Map Pointer Events to Core Commands

## Purpose
Implement pointer input mapping in Ctrl that converts mouse gestures into Core command intents and execution calls.

This task is a Ctrl integration task that consumes a Core contract and must not move domain logic into Ctrl.

## Scope
- Map click drag marquee and link-creation gestures to Core command requests
- Route modifiers and pointer positions through Core transform APIs
- Keep transient Ctrl-only cursor state minimal and explicit

## Out of Scope
- Implementing selection policy itself in Ctrl
- Hit geometry calculation in Ctrl
- Undo stack internals

## Package Ownership
- `Node/Ctrl`

## Depends On
- `./uppsrc/Node/plan/core-editor-commands/04-selection-picking-integration/02-wire-pick-results-to-core-commands.md`
- `./uppsrc/Node/plan/ctrl-integration/01-viewport-ctrl-skeleton/01-create-viewport-ctrl-shell.md`

## Touches
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Ctrl may gather raw events but must ask Core for picking and mutation semantics
- Keep gesture-to-command mapping table declarative for maintainability
- Avoid direct Graph model writes in event handlers
- Consume Core contract: `Core Command Contract v1 plus pick-to-command mapping API`.
- Do not place graph model, persistence, routing, hit testing, or render-path generation in Ctrl; keep those in Node/Core.

## Acceptance Criteria
- [ ] Primary pointer workflows invoke Core commands only
- [ ] No geometry or selection semantics are reimplemented in Ctrl
- [ ] Interaction traces are reproducible in tests

## Suggested Validation
- manual interaction checks
- compile checks
- mock command-dispatch tests
