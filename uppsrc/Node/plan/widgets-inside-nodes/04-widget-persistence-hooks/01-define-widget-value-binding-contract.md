# Define Widget Value Binding Contract

## Purpose
Freeze how widget values map to Core document/runtime channels and how updates become undoable Core commands.

This task freezes a cross-package interface so both Core and Ctrl can evolve independently without boundary drift.

## Scope
- Define binding identifiers and typed value channel schema
- Define update flow from widget edits to Core command dispatch
- Define default value and validation error behavior

## Out of Scope
- Implementing specific widget controls
- UI styling of widgets
- Performance optimization

## Package Ownership
- `Boundary`

## Depends On
- `./uppsrc/Node/plan/widgets-inside-nodes/01-widget-slot-model/02-add-slot-serialization-rules.md`
- `./uppsrc/Node/plan/core-editor-commands/02-command-protocol/01-freeze-command-interface-contract.md`
- `./uppsrc/Node/plan/core-model-document/04-persistence-implementation/01-implement-document-reader-writer.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`

## Implementation Notes
- Contract must keep values and validation in Core-owned channels
- Ctrl widgets act as adapters only and must not own business rules
- Ensure all widget edits are command-based for undo/redo compatibility
- Freeze exact interface contract: `Widget Value Binding Contract v1 (Core value channels, change commands, and Ctrl widget adapters)`.
- Keep the contract minimal and stable so Core and Ctrl can compile and test independently.
- Separate responsibilities explicitly: Core owns slot/descriptor model; Ctrl owns concrete child-Ctrl hosting and parenting.

## Acceptance Criteria
- [ ] Binding contract includes type rules and update lifecycle
- [ ] Core and Ctrl integration points are unambiguous
- [ ] Undoable update path is defined

## Suggested Validation
- contract tests
- compile checks
