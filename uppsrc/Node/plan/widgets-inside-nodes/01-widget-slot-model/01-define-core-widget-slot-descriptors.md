# Define Core Widget Slot Descriptors

## Purpose
Define Core-side widget slot descriptors so node documents can describe embedded-widget locations and roles without depending on Ctrl classes.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define slot ID anchor rect policy and type metadata fields
- Define slot-to-node binding and lifecycle semantics in document model
- Define slot participation in scene descriptors and hit geometry

## Out of Scope
- Implementing actual child Ctrl hosting
- Focus event handling
- Widget toolkit selection

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-model-document/03-core-model-implementation/01-implement-core-document-entities.md`
- `./uppsrc/Node/plan/core-scene-render/02-scene-descriptor-model/01-design-scene-descriptor-types.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Keep slot descriptors purely declarative and serializable
- Do not reference Ctrl pointers or widget instances in Core
- Plan descriptor evolution for future widget families
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.
- Separate responsibilities explicitly: Core owns slot/descriptor model; Ctrl owns concrete child-Ctrl hosting and parenting.

## Acceptance Criteria
- [ ] Core slot model supports multiple slots per node with stable IDs
- [ ] Slot descriptors are independent from Ctrl runtime objects
- [ ] Design clearly separates slot model from host implementation

## Suggested Validation
- unit tests for descriptor integrity
- compile checks
