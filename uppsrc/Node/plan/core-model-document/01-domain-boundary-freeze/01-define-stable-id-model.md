# Define Stable ID Model

## Purpose
Define the canonical identifier model for graph, node, pin, edge, and group entities so future serialization, commands, and migration code can rely on stable keys.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define ID syntax and normalization rules
- Define uniqueness scope per document and cross-document import behavior
- Define legacy GraphLib ID mapping including integer edge/node examples from tutorials

## Out of Scope
- Implementing serializer code paths
- Ctrl event plumbing and menus
- Rendering implementation details

## Package Ownership
- `Node/Core`

## Depends On
None

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/Graph.h`
- `legacy reference only: ./tutorial/GraphLib1/main.cpp`

## Implementation Notes
- Use GraphLib string IDs as migration baseline and document divergences
- Specify collision policy for paste/import workflows
- Record reserved namespaces for system-generated IDs
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] ID rules cover all entity kinds used in GraphLib
- [ ] ID collision and import scenarios are documented with deterministic outcomes
- [ ] No Ctrl types or UI concerns are introduced

## Suggested Validation
- unit tests for ID generation/validation
- compile checks for Node/Core
