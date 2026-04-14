# Define Compatibility Regression Fixtures

## Purpose
Define regression fixture set that encodes compatibility targets for migrated GraphLib behavior.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Create fixture catalog tied to compatibility matrix entries
- Define expected outputs for model scene and algorithm parity checks
- Tag fixtures by criticality and migration phase usage

## Out of Scope
- Implementing adapter code
- Ctrl-specific interaction regression scripts
- Performance tuning

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/migration-compat/01-compatibility-inventory-freeze/01-freeze-graphlib-compatibility-matrix.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./tutorial/GraphLib2/main.cpp`
- `legacy reference only: ./tutorial/GraphLib3/main.cpp`

## Implementation Notes
- Fixtures should be consumable by unit and golden tests throughout migration
- Keep fixture semantics in Core-level artifacts
- Avoid embedding Ctrl event traces as first-class compatibility definitions
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Fixture catalog maps cleanly to compatibility matrix items
- [ ] Expected outputs are deterministic
- [ ] Fixtures are reusable across tracks

## Suggested Validation
- golden file tests
- unit tests
