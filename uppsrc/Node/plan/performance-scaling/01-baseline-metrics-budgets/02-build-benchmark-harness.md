# Build Benchmark Harness

## Purpose
Implement baseline benchmark harnesses in Core to measure scene generation layout hit testing and command throughput.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Implement repeatable benchmark entrypoints and fixture loading
- Capture timing/memory metrics with stable reporting format
- Add benchmark scenarios derived from migrated GraphLib graphs

## Out of Scope
- Ctrl visual profiling UI
- Optimization implementation
- OS-specific profiler integration

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/performance-scaling/01-baseline-metrics-budgets/01-define-performance-budgets.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./tutorial/GraphLib3/main.cpp`

## Implementation Notes
- Keep harness headless and scriptable for CI integration
- Separate measurement collection from optimization policy
- Do not add Ctrl dependencies to benchmark core
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Harness runs baseline scenarios and outputs normalized metrics
- [ ] Results are reproducible across runs with fixed seed/config
- [ ] No Ctrl linkage is required

## Suggested Validation
- benchmark runs
- compile checks
