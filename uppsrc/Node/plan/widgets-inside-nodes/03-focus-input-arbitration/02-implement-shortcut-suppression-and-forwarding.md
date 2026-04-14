# Implement Shortcut Suppression and Forwarding

## Purpose
Implement Ctrl-side shortcut suppression/forwarding behavior according to the frozen focus contract.

This task is a Ctrl integration task that consumes a Core contract and must not move domain logic into Ctrl.

## Scope
- Suppress graph shortcuts while compatible widget edit sessions are active
- Forward allowed commands to Core when policy permits
- Emit focus-state updates back to Core editor state APIs

## Out of Scope
- Defining command semantics
- Widget slot persistence
- Scene rendering logic

## Package Ownership
- `Node/Ctrl`

## Depends On
- `./uppsrc/Node/plan/widgets-inside-nodes/03-focus-input-arbitration/01-define-focus-handoff-policy.md`

## Touches
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`

## Implementation Notes
- Consume focus gating decisions from Core contract and avoid local policy forks
- Do not mutate Core model directly from widget callbacks
- Keep forwarding logic auditable with trace logs
- Consume Core contract: `Widget Focus Arbitration Contract v1 and Core command dispatch API`.
- Do not place graph model, persistence, routing, hit testing, or render-path generation in Ctrl; keep those in Node/Core.
- Separate responsibilities explicitly: Core owns slot/descriptor model; Ctrl owns concrete child-Ctrl hosting and parenting.

## Acceptance Criteria
- [ ] Graph shortcuts are suppressed during text/widget edit modes where required
- [ ] Allowed shortcuts still dispatch through Core command API
- [ ] Behavior matches frozen focus contract

## Suggested Validation
- manual interaction checks
- compile checks
