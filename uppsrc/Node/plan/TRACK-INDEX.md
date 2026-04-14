# Node Plan Track Index

## Track Summary
- `core-model-document` (`Node/Core`): document model, schema, persistence foundations.
- `core-scene-render` (mostly `Node/Core` + Boundary; one `Node/Ctrl` fixture task): scene/render pipeline and paint bridge contracts.
- `core-layout-routing` (`Node/Core` + Boundary): layout and routing algorithms.
- `core-editor-commands` (`Node/Core` + Boundary): editor state, commands, undo/redo.
- `ctrl-integration` (`Node/Ctrl`): thin Ctrl shell consuming Core contracts.
- `widgets-inside-nodes` (Mixed): Core slot descriptors + Ctrl child-hosting.
- `performance-scaling` (Mixed): optimization and scaling over stable contracts.
- `migration-compat` (Mixed): compatibility matrix, adapters, facades, tutorial migration.

## Critical Path
Critical path markers: **[CP]**

1. **[CP]** `core-model-document`
   - `01-domain-boundary-freeze`: freeze IDs and persistent/runtime split.
   - `02-document-schema-v1`: define schema and versioning rules.
   - `03-core-model-implementation`: implement normalized entities and validation.
   - `04-persistence-implementation`: implement reader/writer and roundtrip fixtures.
2. **[CP]** `core-scene-render`
   - `01-coordinate-system-spec`: freeze world/view/screen transforms.
   - `02-scene-descriptor-model`: define and build scene descriptors.
   - `03-path-style-resolver`: style resolution and Core edge paths.
   - `04-hit-geometry-core`: headless hit query contract + fixtures.
   - `05-draw-bridge-contract`: freeze paint command contract.
3. **[CP]** `core-editor-commands`
   - `01-editor-state-model`: runtime editor state model.
   - `02-command-protocol`: freeze command contract + dispatch kernel.
   - `03-undo-redo-engine`: history stack and transaction/merge rules.
   - `04-selection-picking-integration`: selection command set wired to picks.
4. **[CP]** `ctrl-integration`
   - `01-viewport-ctrl-skeleton`: base Ctrl shell + paint bridge entry.
   - `02-input-mapping-layer`: pointer and keyboard mapping to Core commands.
   - `03-menu-focus-integration`: context menu and focus routing.
   - `04-clipboard-dragdrop-bridge`: OS clipboard and drag/drop adapters.

## Parallelizable Phases (after prerequisites)
- `core-layout-routing/01-legacy-algorithm-audit-port` can start after core model interfaces settle.
- `core-layout-routing/02-routing-policy-layer` can proceed in parallel with later scene/editor wiring.
- `widgets-inside-nodes/01-widget-slot-model` can start after model and scene descriptor contracts are stable.
- `performance-scaling/01-baseline-metrics-budgets` can begin once minimal Ctrl bridge exists.
- `migration-compat/01-compatibility-inventory-freeze` can run early as planning/test inventory.

## Package Ownership Summary Per Track
- `core-model-document`: Core only.
- `core-scene-render`: mostly Core; one Boundary contract task and one Node/Ctrl fixture task in draw-bridge phase; Ctrl consumes contract only.
- `core-layout-routing`: Core with Boundary contract for routing policy.
- `core-editor-commands`: Core with Boundary command contract tasks.
- `ctrl-integration`: Ctrl only, strictly consuming frozen Core contracts.
- `widgets-inside-nodes`: Core for slot descriptors/persistence, Ctrl for host container/focus plumbing, Boundary for arbitration contracts.
- `performance-scaling`: Core-heavy optimization plus Boundary budget contract and Ctrl bridge validation.
- `migration-compat`: Boundary/Core inventory and adapters, Ctrl facades and sample migration on top.
