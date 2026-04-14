# Add Slot Serialization Rules

## Purpose
Extend Core document schema rules to persist widget slot descriptors and related metadata safely.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define schema fields for slot descriptors and defaults
- Define versioning behavior for unknown slot properties
- Add roundtrip fixture expectations for slot-bearing nodes

## Out of Scope
- Ctrl child widget state persistence
- Focus policy
- Widget rendering bridge

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/widgets-inside-nodes/01-widget-slot-model/01-define-core-widget-slot-descriptors.md`
- `./uppsrc/Node/plan/core-model-document/04-persistence-implementation/01-implement-document-reader-writer.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`

## Implementation Notes
- Keep persisted data limited to slot descriptors and serializable values
- Do not persist Ctrl object identity or layout cache artifacts
- Maintain backward compatibility with documents without slots
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.
- Separate responsibilities explicitly: Core owns slot/descriptor model; Ctrl owns concrete child-Ctrl hosting and parenting.

## Acceptance Criteria
- [ ] Documents with slot descriptors roundtrip correctly
- [ ] Older documents load with defaults
- [ ] Core schema remains Ctrl-agnostic

## Suggested Validation
- golden file tests
- unit tests
- compile checks
