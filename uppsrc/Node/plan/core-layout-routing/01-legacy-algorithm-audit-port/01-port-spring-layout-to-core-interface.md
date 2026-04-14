# Port Spring Layout to Core Interface

## Purpose
Port the existing GraphLib spring layout algorithm behind the new Core layout interface with deterministic inputs and outputs.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define layout request/response structures for spring algorithm
- Port force-directed logic from GraphLib Spring with normalized units
- Add deterministic seed/control options for reproducible tests

## Out of Scope
- Implementing Ctrl interaction
- Routing policy interfaces
- Performance acceleration

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-model-document/03-core-model-implementation/01-implement-core-document-entities.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/Spring.cpp`

## Implementation Notes
- Preserve useful behavior while removing legacy state coupling
- Keep layout algorithm independent from scene paint concerns
- Avoid hidden dependencies on Ctrl timers or events
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Spring layout compiles and runs via new Core layout interface
- [ ] Results are reproducible under fixed seed/config
- [ ] No Ctrl dependency is introduced

## Suggested Validation
- unit tests for layout outputs
- golden layout fixtures
- compile checks
