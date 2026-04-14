# Implement Transitional Ctrl Facade

## Purpose
Implement thin Ctrl-side compatibility facade classes that map legacy interaction entry points onto new Core/Ctrl architecture.

This task is a Ctrl integration task that consumes a Core contract and must not move domain logic into Ctrl.

## Scope
- Create wrapper methods compatible with priority GraphLib control usage
- Map wrapper calls to Core command and scene APIs
- Document unsupported legacy behavior with explicit diagnostics

## Out of Scope
- Implementing Core model/routing logic in Ctrl
- Long-term API design freeze
- Performance tuning

## Package Ownership
- `Node/Ctrl`

## Depends On
- `./uppsrc/Node/plan/migration-compat/03-facade-shim-layer/01-implement-transitional-core-facade.md`
- `./uppsrc/Node/plan/ctrl-integration/03-menu-focus-integration/01-implement-context-menu-command-bindings.md`

## Touches
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.h`

## Implementation Notes
- Consume Core facade/contracts only and keep wrappers thin
- Do not add domain algorithms in Ctrl wrappers
- Keep wrappers temporary and clearly marked
- Consume Core contract: `Transitional Core Facade API and Core Paint Command Contract v1`.
- Do not place graph model, persistence, routing, hit testing, or render-path generation in Ctrl; keep those in Node/Core.

## Acceptance Criteria
- [ ] Ctrl facade compiles and forwards supported operations to Core
- [ ] Unsupported operations return clear diagnostics
- [ ] No domain logic migration into Ctrl occurs

## Suggested Validation
- compile checks
- manual interaction checks
