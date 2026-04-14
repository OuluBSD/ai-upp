# Bridge Core Scene to Ctrl Paint

## Purpose
Implement the Ctrl::Paint bridge that replays Core paint commands without adding geometry or style logic in Ctrl.

This task is a Ctrl integration task that consumes a Core contract and must not move domain logic into Ctrl.

## Scope
- Implement command playback into U++ Draw operations
- Handle clipping and ordering according to contract invariants
- Add diagnostics for unsupported command opcodes

## Out of Scope
- Generating edge paths in Ctrl
- Style fallback decisions in Ctrl
- Input command routing

## Package Ownership
- `Node/Ctrl`

## Depends On
- `./uppsrc/Node/plan/ctrl-integration/01-viewport-ctrl-skeleton/01-create-viewport-ctrl-shell.md`
- `./uppsrc/Node/plan/core-scene-render/05-draw-bridge-contract/01-freeze-core-paint-command-contract.md`

## Touches
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphLib.h`

## Implementation Notes
- Replay only contract-provided geometry and styles from Core
- Keep bridge deterministic and side-effect free beyond paint output
- Do not compute routes anchors or hit geometry in Ctrl
- Consume Core contract: `Core Paint Command Contract v1`.
- Do not place graph model, persistence, routing, hit testing, or render-path generation in Ctrl; keep those in Node/Core.
- Keep rendering logic (scene assembly, style resolution, path generation) in Node/Core; Ctrl may only perform final `Ctrl::Paint` bridging.

## Acceptance Criteria
- [ ] Ctrl paint output reflects Core command stream order and payloads
- [ ] Unsupported command handling is explicit and testable
- [ ] No domain logic is duplicated in Ctrl

## Suggested Validation
- compile checks
- manual interaction checks
- paint playback fixture checks
