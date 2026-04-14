# Create Viewport Ctrl Shell

## Purpose
Create the initial Node/Ctrl viewport control skeleton that hosts the editor surface and delegates domain behavior to Core.

This task is a Ctrl integration task that consumes a Core contract and must not move domain logic into Ctrl.

## Scope
- Define Ctrl class skeleton and lifecycle hooks
- Wire dependency injection for Core document scene and command services
- Add minimal paint and input stubs without domain logic

## Out of Scope
- Implementing full paint bridge command playback
- Complex interaction handlers
- Widget hosting

## Package Ownership
- `Node/Ctrl`

## Depends On
- `./uppsrc/Node/plan/core-scene-render/05-draw-bridge-contract/01-freeze-core-paint-command-contract.md`
- `./uppsrc/Node/plan/core-editor-commands/02-command-protocol/01-freeze-command-interface-contract.md`

## Touches
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.h`

## Implementation Notes
- Keep Ctrl class thin and orchestration-focused
- Do not mutate document state directly from Ctrl methods
- Use contracts from Core as the only mutation/render interfaces
- Consume Core contract: `Core Paint Command Contract v1 and Core Command Contract v1`.
- Do not place graph model, persistence, routing, hit testing, or render-path generation in Ctrl; keep those in Node/Core.

## Acceptance Criteria
- [ ] Viewport Ctrl compiles and instantiates with Core service contracts
- [ ] No graph algorithm logic is added to Ctrl
- [ ] Skeleton supports later incremental feature wiring

## Suggested Validation
- compile checks for Node/Ctrl
- manual interaction checks for basic control lifecycle
