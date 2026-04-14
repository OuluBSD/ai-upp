# Implement Drag/Drop Document Bridge

## Purpose
Implement Ctrl drag/drop entry points that import/export graph payloads through Core document APIs.

This task is a Ctrl integration task that consumes a Core contract and must not move domain logic into Ctrl.

## Scope
- Handle drop payload ingestion and dispatch into Core import commands
- Support drag export of selected graph fragments via Core serialization
- Define drag/drop capability flags and error feedback behavior

## Out of Scope
- Designing core migration adapters
- Implementing routing algorithms
- Widget host focus arbitration

## Package Ownership
- `Node/Ctrl`

## Depends On
- `./uppsrc/Node/plan/ctrl-integration/04-clipboard-dragdrop-bridge/01-implement-os-clipboard-adapter.md`

## Touches
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Keep drag/drop transport in Ctrl and payload interpretation in Core
- Avoid adding format-specific domain parsing in Ctrl
- Ensure undoable import commands remain Core-owned
- Consume Core contract: `Core DocumentIO API and Core Command Contract v1`.
- Do not place graph model, persistence, routing, hit testing, or render-path generation in Ctrl; keep those in Node/Core.

## Acceptance Criteria
- [ ] Drag/drop operations execute through Core contracts only
- [ ] Import/export errors are reported without model corruption
- [ ] Ctrl does not own document semantics

## Suggested Validation
- manual interaction checks
- compile checks
