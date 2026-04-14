# Wire Pick Results to Core Commands

## Purpose
Implement adapter logic in Core that transforms hit-query outputs into selection command intents for later Ctrl consumption.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define pick-to-intent mapping helpers for click and marquee cases
- Implement utility functions for modifier translation inputs
- Expose stable APIs for Ctrl input layer to call

## Out of Scope
- Ctrl event-loop wiring
- Menu command binding
- OS-specific input translation

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-editor-commands/04-selection-picking-integration/01-implement-selection-command-set.md`
- `./uppsrc/Node/plan/core-scene-render/04-hit-geometry-core/01-define-hit-query-api.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Keep this mapping in Core to avoid duplicated pick policies in Ctrl
- Ensure mapping APIs are deterministic and side-effect free
- Document assumptions for modifier masks and selection modes
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Pick-to-command mapping is fully covered by tests
- [ ] Ctrl can call mapping APIs without additional geometry logic
- [ ] No Ctrl types are referenced

## Suggested Validation
- unit tests for mapping tables
- compile checks
