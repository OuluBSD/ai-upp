# Add Roundtrip and Compatibility Fixtures

## Purpose
Create reusable fixtures that exercise persistence roundtrip and compatibility assumptions needed by later migration tasks.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Add minimal medium and large graph fixtures including pins groups and styles
- Add fixtures mapped from GraphLib tutorial scenarios
- Add compatibility fixture placeholders for legacy import adapters

## Out of Scope
- Implementing legacy adapters themselves
- Ctrl integration tests
- Performance tuning

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-model-document/04-persistence-implementation/01-implement-document-reader-writer.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./tutorial/GraphLib1/main.cpp`
- `legacy reference only: ./tutorial/GraphLib4/main.cpp`

## Implementation Notes
- Use deterministic fixture generation to keep diffs reviewable
- Tag fixtures by schema version and scenario intent
- Keep fixtures usable by both unit and golden tests
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Fixture set covers all core entity relationships
- [ ] Roundtrip tests are reproducible and deterministic
- [ ] Fixtures remain independent of Ctrl

## Suggested Validation
- golden file tests
- unit tests
- compile checks
