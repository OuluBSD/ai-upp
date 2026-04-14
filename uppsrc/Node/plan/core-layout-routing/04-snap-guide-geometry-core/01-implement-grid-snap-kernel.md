# Implement Grid Snap Kernel

## Purpose
Implement Core-side grid snapping computations so Ctrl can request snapped positions without owning snap math.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define snap query inputs including candidate point and grid settings
- Implement deterministic snapped output with tolerance handling
- Expose APIs usable by move and placement commands

## Out of Scope
- Ctrl drag gesture UI
- Alignment guide visualization
- Persistence of grid UI preferences

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-scene-render/01-coordinate-system-spec/02-define-viewport-transform-api.md`
- `./uppsrc/Node/plan/core-editor-commands/02-command-protocol/02-implement-command-dispatch-kernel.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/Snap.h`

## Implementation Notes
- Replace stale Snap.h concept with real Core implementation
- Keep snap calculations independent of Ctrl event sampling rates
- Avoid introducing UI settings classes into Core
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Grid snap kernel returns deterministic outputs for tested inputs
- [ ] API is callable from command handlers without Ctrl coupling
- [ ] No drawing logic is moved into Ctrl

## Suggested Validation
- unit tests for snap cases
- compile checks
