# Add Widget Value Roundtrip Tests

## Purpose
Add Core-side test coverage ensuring widget-bound values survive command updates and document persistence roundtrip.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Add fixtures for multiple widget value types and default values
- Test command-driven updates then save/load roundtrip behavior
- Test backward compatibility for documents without widget values

## Out of Scope
- Ctrl widget parenting tests
- Focus interaction tests
- Visual rendering verification

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/widgets-inside-nodes/04-widget-persistence-hooks/01-define-widget-value-binding-contract.md`
- `./uppsrc/Node/plan/core-model-document/04-persistence-implementation/02-add-roundtrip-and-compat-fixtures.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`

## Implementation Notes
- Keep test data and checks in Core to verify headless persistence behavior
- Do not rely on Ctrl event simulation for value persistence tests
- Use deterministic fixture updates for reproducibility
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.
- Separate responsibilities explicitly: Core owns slot/descriptor model; Ctrl owns concrete child-Ctrl hosting and parenting.

## Acceptance Criteria
- [ ] Widget value channels persist correctly across roundtrip
- [ ] Command updates are reflected in serialized output
- [ ] No Ctrl dependency is required for these tests

## Suggested Validation
- unit tests
- golden file tests
- compile checks
