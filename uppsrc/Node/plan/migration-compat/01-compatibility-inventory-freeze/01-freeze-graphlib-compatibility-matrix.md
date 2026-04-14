# Freeze GraphLib Compatibility Matrix

## Purpose
Freeze the migration compatibility matrix that defines what legacy GraphLib behavior is carried forward, adapted, or explicitly dropped.

This task freezes a cross-package interface so both Core and Ctrl can evolve independently without boundary drift.

## Scope
- Enumerate GraphLib APIs and behaviors across tutorials and package code
- Classify each item as preserve adapt deprecate or drop
- Define parity test targets and acceptable deviations

## Out of Scope
- Implementing adapters
- Ctrl facade code
- Feature redesign beyond skeleton

## Package Ownership
- `Boundary`

## Depends On
- `./uppsrc/Node/plan/core-model-document/01-domain-boundary-freeze/02-separate-persistent-vs-runtime-state.md`

## Touches
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./tutorial/GraphLib1/main.cpp`
- `legacy reference only: ./tutorial/GraphLib4/main.cpp`
- `legacy reference only: ./uppsrc/GraphLib/...`

## Implementation Notes
- Use current GraphLib code as evidence rather than assumptions
- Keep matrix versioned for future deprecation updates
- Ensure matrix aligns with Core/Ctrl ownership rules
- Freeze exact interface contract: `GraphLib Compatibility Matrix Contract v1 (supported APIs, behaviors, and sample parity targets)`.
- Keep the contract minimal and stable so Core and Ctrl can compile and test independently.

## Acceptance Criteria
- [ ] Compatibility matrix covers tutorials and major GraphLib subsystems
- [ ] Each item has explicit migration status
- [ ] Contract is accepted as migration baseline

## Suggested Validation
- compatibility review
- manual checklist validation
