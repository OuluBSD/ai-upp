# Port Tree and Shortest-Path Algorithms

## Purpose
Port ordered/tournament/topological and Dijkstra helper algorithms into cohesive Core algorithm modules.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Port OrderedTree TournamentTree TopologicalSort and Dijkstra into new namespaces
- Define shared graph-algorithm utility interfaces
- Add parity fixtures based on GraphLib tutorials

## Out of Scope
- Ctrl menu integration
- Scene rendering implementation
- Undo stack coupling

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-layout-routing/01-legacy-algorithm-audit-port/01-port-spring-layout-to-core-interface.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/OrderedTree.cpp`
- `legacy reference only: ./uppsrc/GraphLib/TournamentTree.cpp`
- `legacy reference only: ./uppsrc/GraphLib/Dijkstra.cpp`

## Implementation Notes
- Treat algorithms as headless services callable by both rendering and editor overlays
- Document known algorithmic assumptions inherited from GraphLib
- Avoid storing algorithm artifacts inside persistent document fields
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.
- Keep rendering logic (scene assembly, style resolution, path generation) in Node/Core; Ctrl may only perform final `Ctrl::Paint` bridging.

## Acceptance Criteria
- [ ] Ported algorithms run through new Core interfaces
- [ ] Tutorial-derived parity fixtures pass
- [ ] Algorithms remain independent from Ctrl and rendering bridge code

## Suggested Validation
- unit tests
- golden file tests
- compile checks
