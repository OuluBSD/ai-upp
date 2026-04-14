# Implement Selection Command Set

## Purpose
Implement selection commands in Core for single multi and marquee selection over nodes pins edges and groups.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Implement select add remove toggle clear commands
- Implement modifier-aware semantics as command parameters not Ctrl logic
- Integrate with hit query results and deterministic precedence rules

## Out of Scope
- Ctrl input mapping details
- Menu UI commands
- Hover-only visual rendering

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-editor-commands/02-command-protocol/02-implement-command-dispatch-kernel.md`
- `./uppsrc/Node/plan/core-scene-render/04-hit-geometry-core/02-add-hit-geometry-fixtures.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Use GraphLib behavior as baseline but formalize edge/group semantics
- Keep selection state in Core editor runtime state
- Avoid direct selection mutation from Ctrl
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Selection commands support all entity types
- [ ] Precedence and toggle semantics are test-covered
- [ ] Core API remains UI-agnostic

## Suggested Validation
- unit tests
- golden state transition tests
- compile checks
