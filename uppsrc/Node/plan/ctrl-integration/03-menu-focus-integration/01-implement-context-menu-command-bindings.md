# Implement Context Menu Command Bindings

## Purpose
Implement context menu integration in Ctrl where menu actions invoke Core commands instead of direct model changes.

This task is a Ctrl integration task that consumes a Core contract and must not move domain logic into Ctrl.

## Scope
- Build node pin edge group and background menu sections
- Resolve menu availability from Core pick/selection state snapshots
- Map every menu action to Core command IDs and payloads

## Out of Scope
- Core command handler implementation
- Document persistence schema
- Rendering algorithms

## Package Ownership
- `Node/Ctrl`

## Depends On
- `./uppsrc/Node/plan/ctrl-integration/02-input-mapping-layer/01-map-pointer-events-to-core-commands.md`

## Touches
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Use GraphLib menu coverage as migration reference while cleaning direct mutation patterns
- Keep menu code purely presentational and dispatch-oriented
- Do not encode selection semantics in menu layer
- Consume Core contract: `Core Command Contract v1 and pick query API`.
- Do not place graph model, persistence, routing, hit testing, or render-path generation in Ctrl; keep those in Node/Core.

## Acceptance Criteria
- [ ] All menu actions dispatch Core commands
- [ ] No direct graph mutations remain in menu callbacks
- [ ] Menu visibility logic is testable from state snapshots

## Suggested Validation
- manual interaction checks
- compile checks
- menu dispatch tests
