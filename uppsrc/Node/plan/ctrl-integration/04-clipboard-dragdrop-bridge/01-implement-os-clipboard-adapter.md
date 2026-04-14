# Implement OS Clipboard Adapter

## Purpose
Implement Ctrl-side OS clipboard integration by serializing/deserializing through Core document contracts.

This task is a Ctrl integration task that consumes a Core contract and must not move domain logic into Ctrl.

## Scope
- Encode selected subgraph payloads using Core document format
- Decode clipboard payloads via Core reader and dispatch paste commands
- Handle incompatible payload and version errors in Ctrl UX layer

## Out of Scope
- Defining document schema
- Core selection command semantics
- Performance tuning

## Package Ownership
- `Node/Ctrl`

## Depends On
- `./uppsrc/Node/plan/core-model-document/04-persistence-implementation/01-implement-document-reader-writer.md`
- `./uppsrc/Node/plan/ctrl-integration/02-input-mapping-layer/02-map-keyboard-shortcuts-to-command-dispatch.md`

## Touches
- `./uppsrc/Node/Ctrl/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`

## Implementation Notes
- Clipboard is a Ctrl concern but payload semantics must come from Core schema
- Do not parse or mutate graph entities directly in Ctrl
- Keep error handling UI-only while preserving Core diagnostics
- Consume Core contract: `Core DocumentIO API and Core Command Contract v1`.
- Do not place graph model, persistence, routing, hit testing, or render-path generation in Ctrl; keep those in Node/Core.

## Acceptance Criteria
- [ ] Copy/paste via OS clipboard uses Core serialization contracts
- [ ] Invalid payloads fail gracefully without corruption
- [ ] No domain parsing logic is duplicated in Ctrl

## Suggested Validation
- manual interaction checks
- compile checks
- clipboard roundtrip tests
