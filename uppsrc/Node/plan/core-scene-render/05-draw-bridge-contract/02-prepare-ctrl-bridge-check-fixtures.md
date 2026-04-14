# Prepare Ctrl Bridge Check Fixtures

## Purpose
Prepare lightweight fixtures and harness expectations used by later Ctrl paint bridge implementation tasks.

This task is a Ctrl integration task that consumes a Core contract and must not move domain logic into Ctrl.

## Scope
- Define fixture inputs as frozen Core paint command streams
- Define expected Draw call sequences and sanity assertions
- Define failure diagnostics for missing unsupported command handling

## Out of Scope
- Implementing full viewport Ctrl
- Input-event dispatch
- Model mutation logic

## Package Ownership
- `Node/Ctrl`

## Depends On
- `./uppsrc/Node/plan/core-scene-render/05-draw-bridge-contract/01-freeze-core-paint-command-contract.md`

## Touches
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphLib.h`

## Implementation Notes
- This task only consumes paint command streams produced by Core
- Do not add route geometry or style resolution in Ctrl fixture code
- Keep harness thin and focused on bridge correctness
- Consume Core contract: `Core Paint Command Contract v1 (scene draw command schema consumed by Ctrl::Paint bridge)`.
- Do not place graph model, persistence, routing, hit testing, or render-path generation in Ctrl; keep those in Node/Core.
- Keep rendering logic (scene assembly, style resolution, path generation) in Node/Core; Ctrl may only perform final `Ctrl::Paint` bridging.

## Acceptance Criteria
- [ ] Fixture harness can validate command stream playback in Ctrl tests
- [ ] Unsupported command behavior is explicitly defined
- [ ] No Core domain logic is duplicated in Ctrl

## Suggested Validation
- compile checks for Node/Ctrl
- manual interaction checks limited to paint playback harness
