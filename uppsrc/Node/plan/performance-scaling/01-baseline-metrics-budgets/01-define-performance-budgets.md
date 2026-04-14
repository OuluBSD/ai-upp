# Define Performance Budgets

## Purpose
Define measurable performance budgets spanning Core scene work and Ctrl bridge playback before optimization tasks start.

This task freezes a cross-package interface so both Core and Ctrl can evolve independently without boundary drift.

## Scope
- Define target budgets for scene build paint replay hit tests and edit commands
- Define benchmark graph sizes and scenario classes
- Define reporting format and pass/fail thresholds

## Out of Scope
- Implementing benchmark harness code
- Optimization algorithms
- Feature additions

## Package Ownership
- `Boundary`

## Depends On
- `./uppsrc/Node/plan/ctrl-integration/01-viewport-ctrl-skeleton/02-bridge-core-scene-to-ctrl-paint.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`

## Implementation Notes
- Budget contract should avoid platform-specific overfitting while remaining actionable
- Include both median and worst-case targets
- Keep ownership clear: Core optimizes algorithms, Ctrl optimizes bridge overhead
- Freeze exact interface contract: `Performance Budget Contract v1 (scene build, paint bridge, hit query, and command latency targets)`.
- Keep the contract minimal and stable so Core and Ctrl can compile and test independently.

## Acceptance Criteria
- [ ] Budgets are documented and accepted by maintainers
- [ ] Scenario set covers expected graph scales
- [ ] Metrics are tied to concrete validation tasks

## Suggested Validation
- benchmark plan review
- compile checks
