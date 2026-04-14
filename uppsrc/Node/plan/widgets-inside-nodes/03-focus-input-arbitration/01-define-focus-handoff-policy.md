# Define Focus Handoff Policy

## Purpose
Freeze the focus arbitration contract between Core editor state and Ctrl widget host so text entry and graph shortcuts do not conflict.

This task freezes a cross-package interface so both Core and Ctrl can evolve independently without boundary drift.

## Scope
- Define ownership states between graph viewport and child widgets
- Define shortcut suppression rules while widget editing is active
- Define transitions for mouse-click and keyboard-tab focus changes

## Out of Scope
- Implementing concrete shortcut handlers
- Widget rendering details
- OS IME-specific handling

## Package Ownership
- `Boundary`

## Depends On
- `./uppsrc/Node/plan/widgets-inside-nodes/02-ctrl-host-container/01-implement-node-widget-host-container.md`
- `./uppsrc/Node/plan/ctrl-integration/03-menu-focus-integration/02-implement-focus-and-tool-mode-routing.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`

## Implementation Notes
- Contract must clearly separate policy ownership and event plumbing responsibilities
- Core decides tool/command eligibility while Ctrl owns physical focus events
- Ensure the contract prevents accidental command dispatch during text input
- Freeze exact interface contract: `Widget Focus Arbitration Contract v1 (Core editor state gates plus Ctrl focus ownership transitions)`.
- Keep the contract minimal and stable so Core and Ctrl can compile and test independently.
- Separate responsibilities explicitly: Core owns slot/descriptor model; Ctrl owns concrete child-Ctrl hosting and parenting.

## Acceptance Criteria
- [ ] Focus contract is explicit and versioned
- [ ] Core and Ctrl responsibilities are unambiguous
- [ ] Conflict scenarios have deterministic expected behavior

## Suggested Validation
- contract tests
- manual interaction checks
- compile checks
