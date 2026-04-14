# Write Migration Guide and Deprecation Checklist

## Purpose
Produce a practical migration guide and deprecation checklist so implementation teams can execute the transition consistently.

This task freezes a cross-package interface so both Core and Ctrl can evolve independently without boundary drift.

## Scope
- Document API mapping from GraphLib to Node Core/Ctrl
- Document phased deprecation schedule and gating checks
- Document boundary-compliance checklist for code reviews

## Out of Scope
- Implementing runtime code
- Adding new architecture features
- UI redesign decisions

## Package Ownership
- `Boundary`

## Depends On
- `./uppsrc/Node/plan/migration-compat/04-tutorial-sample-migration/01-migrate-graphlib-tutorials-to-node.md`

## Touches
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/...`
- `legacy reference only: ./tutorial/GraphLib1/...`

## Implementation Notes
- Guide must reinforce that rendering/domain logic remains Core-owned
- Include explicit do-not-do examples for Ctrl-side boundary violations
- Keep checklist concise and enforceable in PR review
- Freeze exact interface contract: `Migration Guide Contract v1 (legacy-to-node mapping, deprecation states, and boundary compliance checklist)`.
- Keep the contract minimal and stable so Core and Ctrl can compile and test independently.

## Acceptance Criteria
- [ ] Migration guide maps high-priority legacy APIs and tutorials
- [ ] Deprecation checklist includes concrete gate criteria
- [ ] Boundary rules are explicit and testable

## Suggested Validation
- manual review checklist
- tutorial/sample checks
