# Implement Node Widget Host Container

## Purpose
Implement the Ctrl-side host container that creates and parents child controls according to Core slot descriptors.

This task is a Ctrl integration task that consumes a Core contract and must not move domain logic into Ctrl.

## Scope
- Create host container lifecycle and child Ctrl registry
- Map slot descriptors to concrete child Ctrl instances via factory hooks
- Integrate host updates with viewport repaint/layout cycles

## Out of Scope
- Defining slot descriptors in Core
- Core hit testing algorithms
- OS clipboard behavior

## Package Ownership
- `Node/Ctrl`

## Depends On
- `./uppsrc/Node/plan/widgets-inside-nodes/01-widget-slot-model/01-define-core-widget-slot-descriptors.md`
- `./uppsrc/Node/plan/ctrl-integration/01-viewport-ctrl-skeleton/01-create-viewport-ctrl-shell.md`

## Touches
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Consume slot geometry and identity from Core descriptors only
- Do not move slot model logic or persistence into Ctrl
- Keep child-Ctrl management encapsulated in host container
- Consume Core contract: `Core widget slot descriptor API and Core viewport transform API`.
- Do not place graph model, persistence, routing, hit testing, or render-path generation in Ctrl; keep those in Node/Core.
- Separate responsibilities explicitly: Core owns slot/descriptor model; Ctrl owns concrete child-Ctrl hosting and parenting.

## Acceptance Criteria
- [ ] Host container instantiates and reuses child Ctrls based on Core slots
- [ ] No document/domain logic is duplicated in Ctrl host
- [ ] Parenting lifecycle is stable under viewport updates

## Suggested Validation
- manual interaction checks
- compile checks
