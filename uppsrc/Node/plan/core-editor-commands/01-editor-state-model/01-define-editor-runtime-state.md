# Define Editor Runtime State

## Purpose
Define headless editor runtime state containers for selection hover tools and viewport metadata in Core.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define editor state structures independent from document entities
- Define state transitions for selection hover drag-link and marquee modes
- Define serialization exclusion rules for runtime-only fields

## Out of Scope
- Ctrl event wiring
- Undo command implementation
- Paint bridge integration

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-model-document/03-core-model-implementation/01-implement-core-document-entities.md`
- `./uppsrc/Node/plan/core-scene-render/01-coordinate-system-spec/01-freeze-coordinate-space-contract.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.h`

## Implementation Notes
- Extract behavior intent from GraphNodeCtrl without carrying Ctrl-specific fields
- Keep runtime state deterministic and test-friendly
- Ensure document objects remain free of editor-only mutation flags
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Editor state model covers all planned interaction modes
- [ ] Runtime state has no direct Ctrl dependencies
- [ ] Transitions are documented with invariant rules

## Suggested Validation
- unit tests for state transitions
- compile checks
