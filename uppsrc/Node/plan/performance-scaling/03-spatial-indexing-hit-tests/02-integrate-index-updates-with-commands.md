# Integrate Index Updates with Commands

## Purpose
Integrate spatial index maintenance with command execution so hit-query acceleration remains correct after edits.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Hook index updates into command commit and rollback flows
- Define batch update behavior for transactions
- Add consistency checks between scene data and index contents

## Out of Scope
- Ctrl input mapping
- Menu behavior
- Rendering bridge playback

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/performance-scaling/03-spatial-indexing-hit-tests/01-implement-spatial-index-structures.md`
- `./uppsrc/Node/plan/core-editor-commands/02-command-protocol/02-implement-command-dispatch-kernel.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`

## Implementation Notes
- Keep integration within Core command pipeline and avoid Ctrl-triggered ad-hoc index rebuilds
- Ensure rollback paths restore index consistency
- Add debug assertions for divergence detection
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Index remains consistent after add/remove/move/connect commands and undo/redo
- [ ] No manual Ctrl rebuild calls are required
- [ ] Consistency checks pass in test scenarios

## Suggested Validation
- unit tests
- benchmark runs
- compile checks
