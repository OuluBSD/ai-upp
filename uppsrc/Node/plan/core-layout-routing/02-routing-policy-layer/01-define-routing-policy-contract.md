# Define Routing Policy Contract

## Purpose
Freeze the routing policy interface used by scene/path generation so multiple routers can be plugged in without Ctrl changes.

This task freezes a cross-package interface so both Core and Ctrl can evolve independently without boundary drift.

## Scope
- Define routing input model including anchors obstacles and style constraints
- Define output primitives and metadata contract for labels and arrows
- Define policy registration and selection rules in Core

## Out of Scope
- Implementing specific orthogonal router internals
- Ctrl interaction gestures
- Performance indexing

## Package Ownership
- `Boundary`

## Depends On
- `./uppsrc/Node/plan/core-scene-render/02-scene-descriptor-model/01-design-scene-descriptor-types.md`
- `./uppsrc/Node/plan/core-layout-routing/01-legacy-algorithm-audit-port/02-port-tree-and-shortestpath-algorithms.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/Renderer.cpp`

## Implementation Notes
- Route contract must keep all path semantics in Core and expose only ready-to-draw primitives
- Avoid embedding Draw callbacks or Ctrl pointers in routing APIs
- Include extension points for future routing families
- Freeze exact interface contract: `Core Routing Policy Contract v1 (path request, constraints, and path primitive response)`.
- Keep the contract minimal and stable so Core and Ctrl can compile and test independently.

## Acceptance Criteria
- [ ] Routing contract is documented and versioned
- [ ] Edge-path generator can consume policy outputs directly
- [ ] No Ctrl responsibilities are added

## Suggested Validation
- contract tests
- compile checks
