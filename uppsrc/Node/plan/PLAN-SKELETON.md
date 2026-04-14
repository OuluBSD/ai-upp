# Node Package Plan Skeleton

## 1. Executive Intent
The target architecture is a two-package system where `Node/Core` is a reusable, headless node/graph engine and `Node/Ctrl` is the U++ interactive shell. The main objective is to preserve useful `GraphLib` capabilities (graph model, layouts, edge drawing concepts, basic editor interactions) while restructuring responsibilities so the next implementation phases can scale without repeating current coupling problems.

`Node/Core` and `Node/Ctrl` must be separated because current `GraphLib` mixes model, draw geometry, layout state, and UI control behavior. This makes serialization, undo/redo, testing, and non-GUI reuse difficult. A strict split allows deterministic core tests, easier migration of algorithms, and compatibility with future non-`Ctrl` runtimes.

Draw/rendering responsibilities should be predominantly in `Node/Core` because scene construction, edge path generation, style resolution, hit geometry, and coordinate transforms are domain logic, not `Ctrl` plumbing. `Node/Ctrl` should only bridge those core outputs into `Ctrl::Paint`, route input events into core editor commands, and host U++ UI affordances (menus, focus, clipboard, scrolling widgets).

The architecture should be planned around three separations that are currently blurred in `GraphLib`: (1) document/model state, (2) view scene + rendering data, and (3) editor interaction state/commands. This plan skeleton defines tracks and phases so later detailed task files can be generated without rethinking package boundaries.

## 2. Proposed Package Responsibilities
### `./uppsrc/Node/Core/Core.upp`
What belongs here:
- Graph document model: graph/node/pin/edge/group entities, IDs, metadata.
- Document I/O: serialization/deserialization, schema versioning, migration helpers.
- Scene/render description: node visual descriptors, edge path generation, style resolution, paint command lists / draw batches.
- Geometry + transforms: coordinate spaces, viewport transforms, world/screen conversion helpers.
- Hit testing kernels and selection query helpers (headless algorithms).
- Layout and routing engines: spring/tree/layered, bezier/orthogonal routing.
- Editor domain state (headless): selection model, editor modes, command payloads.
- Command stack + undo/redo and command execution policy.
- Compatibility adapters from legacy `GraphLib` document structures where needed.

What must **not** belong here:
- `Ctrl` subclasses.
- Menus, keybinding dispatch tied to `Ctrl` events.
- OS clipboard access APIs.
- Direct `TopWindow`/`Ctrl` focus policy and widget parenting.

Rationale:
- Core must be reusable, testable, and deterministic without GUI lifecycle coupling.
- Draw/render logic in Core enforces the architectural rule that only final paint bridging is GUI-specific.

### `./uppsrc/Node/Ctrl/Ctrl.upp`
What belongs here:
- `Ctrl` integration layer(s): editor controls, viewport control, event adapters.
- Conversion from core scene/render output to `Draw` calls in `Ctrl::Paint`.
- Input dispatch (mouse/keyboard/wheel) to core editor actions.
- Context menus, command binding tables, focus navigation hooks.
- Clipboard integration (OS clipboard), drag/drop integration, UI-level affordances.
- Host/container layer for embedded child `Ctrl` widgets attached to core node slots.

What must **not** belong here:
- Core graph document classes and serialization format logic.
- Edge routing/path generation algorithms.
- Layout algorithms and general hit-test geometry math.
- Render-scene generation rules and style resolution policy.

Rationale:
- Ctrl package should be thin and replaceable, mostly adapting events and painting.
- Avoid repeating `GraphNodeCtrl.cpp` monolith where interaction rules and model mutation are hardwired in GUI code.

Explicit migration intent from current `GraphLib`:
- Keep and refactor model + algorithm value from `Graph.*`, `Layout.*`, `Spring.*`, `TournamentTree.*`, `OrderedTree.*`, `TopologicalSort.*`, `Dijkstra.*`, `BinaryHeapMap.*` into Core subsystems.
- Refactor `Renderer.*` into Core scene/render and path-generation services.
- Replace `GraphLayout<T>` and `GraphNodeCtrl` with Ctrl wrappers over Core APIs.
- Drop stale artifacts (`Raphael.h`, `Snap.h`) after migration equivalents are settled.

## 3. Responsibility Mapping from Current GraphLib
| Current file / concept | Current responsibility | Proposed new package | Proposed new subsystem | Keep / Refactor / Replace / Drop | Notes |
|---|---|---|---|---|---|
| `Graph` (`Graph.h/.cpp`) | Primary graph container + CRUD + some defaults | `Node/Core` | `DocumentModel` | Refactor | Split pure document from editor/view/transient fields. |
| `Node` | Entity + visual + layout + selection + animation mixed | `Node/Core` | `DocumentModel` + `SceneState` + `EditorState` | Refactor | Separate persistent doc fields from derived scene/editor runtime fields. |
| `Edge` | Connection + style + animation flags | `Node/Core` | `DocumentModel` + `SceneStyle` | Refactor | Move flow animation to optional runtime layer if retained. |
| `Pin` | Port model + local geometry + type metadata | `Node/Core` | `DocumentModel` | Refactor | Keep kind/type metadata; define stable port anchoring semantics. |
| `GroupNode` | Group container + visual style + collapse/selection flags | `Node/Core` | `DocumentModel` + `SceneStyle` + `EditorState` | Refactor | Collapse should become real behavior in scene/editor services. |
| `Renderer` (`Renderer.h/.cpp`) | Draw + path generation + hit tests + coordinate mapping | `Node/Core` | `SceneRenderer` + `Pathing` + `HitTesting` + `Transforms` | Refactor | Keep geometry/path logic in Core; Ctrl only executes draw commands. |
| `GraphLayout<T>` (`GraphLib.h`) | Combines model, layout strategy, rendering buffer, scroll/zoom input | Split (`Node/Core` + `Node/Ctrl`) | Core: `ScenePipeline`; Ctrl: `GraphViewportCtrl` | Replace | Template inheritance pattern should be replaced by composition over Core services. |
| `GraphNodeCtrl` | Monolithic GUI editor behavior + direct model mutation + partial persistence | Split (`Node/Core` + `Node/Ctrl`) | Core: `EditorState`/`CommandStack`; Ctrl: `EditorCtrl` | Replace | Keep feature intent, but remove business logic from Ctrl layer. |
| `Layout` base | Layout abstraction | `Node/Core` | `LayoutEngine` | Refactor | Keep strategy concept; formalize plugin interface and deterministic options. |
| `Spring` | Force-directed layout | `Node/Core` | `LayoutEngine/Spring` | Keep (refactor) | Preserve algorithm, normalize input/output contracts. |
| `OrderedTree` | Topo-sort based tree-ish layout | `Node/Core` | `LayoutEngine/TreeLayered` | Refactor | Fix assumptions and naming clarity. |
| `TournamentTree` | Tournament tree layout | `Node/Core` | `LayoutEngine/TreeTournament` | Keep (refactor) | Preserve optional layout mode. |
| `TopologicalSort` | DAG ordering helper | `Node/Core` | `Algorithms/GraphOrder` | Keep (refactor) | Rework correctness and API clarity. |
| `Dijkstra` | Shortest path helper | `Node/Core` | `Algorithms/ShortestPath` | Keep (refactor) | Useful for route highlighting and analysis overlays. |
| `BinaryHeapMap` | Heap for Dijkstra | `Node/Core` | `Algorithms/Containers` | Keep (refactor) | Could be generalized or replaced by standard utility if available. |
| `GraphLib1..4` tutorials | Viewer/layout examples for current API | Mixed | `CompatibilitySamples` | Refactor | Re-target to new `Node/Ctrl` facade and keep as regression fixtures. |
| `SaveGraph` / `LoadGraph` in `GraphNodeCtrl` | Ad-hoc persistence, load missing | Core-first with Ctrl wrappers | `DocumentIO` | Replace | Define schema + round-trip tests in Core. |
| Edge hover/selection fragments in `GraphNodeCtrl` | Partial interaction logic | Split | Core: `HitTesting`; Ctrl: `InputAdapter` | Refactor | Formalize edge picking semantics and commands. |
| `Raphael.h`, `Snap.h` | Stale pseudo-code/non-compiling artifacts | none | none | Drop | Keep in history only; do not port. |

## 4. Architectural Tracks
### Track T1: Core Model & Document Contract
- Goal: define stable headless document model and persistence contract.
- Package ownership: `Node/Core`.
- Why: everything else (commands, rendering, migration) depends on a stable model and schema.

### Track T2: Core Scene/Render Kernel
- Goal: produce draw-ready scene representation and edge/node render primitives in Core.
- Package ownership: `Node/Core` (with tiny paint bridge in Ctrl later).
- Why: enforces the requirement that rendering logic belongs in Core, not Ctrl.

### Track T3: Core Layout & Routing Engine
- Goal: migrate and normalize automatic layouts and edge routing policies.
- Package ownership: `Node/Core`.
- Why: layout/routing are domain algorithms and should be reusable headlessly.

### Track T4: Core Editor State + Command Stack
- Goal: implement selection/edit state machine and undo/redo command model in Core.
- Package ownership: `Node/Core`.
- Why: removes direct mutable GUI logic and enables deterministic behavior/testing.

### Track T5: Ctrl Integration & Interaction Shell
- Goal: build `Ctrl` adapters for painting, event dispatch, menus, focus, viewport wiring.
- Package ownership: `Node/Ctrl` (consumes Core interfaces).
- Why: isolate U++ input/event mechanics from core graph logic.

### Track T6: Widgets-Inside-Nodes Host Layer
- Goal: support embedded child widgets in nodes without violating Core/Ctrl boundaries.
- Package ownership: mixed (Core slot descriptors + Ctrl host implementation).
- Why: major feature area requiring explicit ownership and focus/input contracts.

### Track T7: Performance & Scaling
- Goal: ensure large-graph responsiveness via caching, indexing, culling, incremental updates.
- Package ownership: mixed (Core algorithms + Ctrl repaint policy).
- Why: current architecture redraws too much and uses linear/O(N^2) operations frequently.

### Track T8: Migration & Compatibility Surface
- Goal: provide migration path from `GraphLib` APIs/tutorials to `Node` packages.
- Package ownership: mixed (Core data adapters + Ctrl facade shims + sample updates).
- Why: reduce breakage risk and keep tutorial/value continuity while refactoring.

## 5. Phase Skeleton per Track
### T1: Core Model & Document Contract
| Phase ID | Title | Objective | Scope | Prerequisites | Deliverables | Exit Criteria |
|---|---|---|---|---|---|---|
| T1-P1 | Domain Boundary Freeze | Freeze entity taxonomy and ownership/lifetime model | IDs, ownership graph, persistent vs transient field split | none | boundary spec doc + model draft headers | team can classify any field as doc/scene/editor without ambiguity |
| T1-P2 | Document Schema V1 | Define serialization schema and version markers | graph/node/pin/edge/group + style references + metadata | T1-P1 | schema draft + parser/writer interfaces | round-trip tests drafted with golden fixtures |
| T1-P3 | Core Model Implementation | Implement normalized model classes in Core | CRUD, validation, references, ID maps | T1-P1 | `Node/Core` model module | model tests pass for creation, deletion, reference integrity |
| T1-P4 | Persistence Implementation | Implement reader/writer + migration hooks | schema encode/decode + invalid data handling | T1-P2, T1-P3 | DocumentIO module + tests | save/load round-trip stable across test graphs |

### T2: Core Scene/Render Kernel
| Phase ID | Title | Objective | Scope | Prerequisites | Deliverables | Exit Criteria |
|---|---|---|---|---|---|---|
| T2-P1 | Coordinate System Spec | Fix world/view/screen coordinate spaces and transform ownership | transform math, DPI/scale assumptions | T1-P1 | transform spec + utility interfaces | all tracks reference one coordinate model |
| T2-P2 | Scene Descriptor Model | Define paintable scene graph/command structures in Core | node glyphs, edge paths, labels, group visuals | T2-P1, T1-P3 | scene descriptor module | sample graphs can generate complete scene descriptors |
| T2-P3 | Path & Style Resolver | Implement edge path generation and style resolution core-side | bezier + future routing hooks, label anchors | T2-P2, T3-P2 optional | render prep pipeline | deterministic scene output from same document/state |
| T2-P4 | Hit Geometry Core | Provide geometry-based pick queries from scene descriptors | node/pin/edge/group hit queries | T2-P2 | hit-test kernel APIs + tests | hit-test behavior reproducible without Ctrl |
| T2-P5 | Draw Bridge Contract | Define Core-to-Ctrl paint bridge API | command-to-`Draw` translation boundary | T2-P2 | adapter contract doc + test harness | Ctrl can paint core scene without duplicating path logic |

### T3: Core Layout & Routing Engine
| Phase ID | Title | Objective | Scope | Prerequisites | Deliverables | Exit Criteria |
|---|---|---|---|---|---|---|
| T3-P1 | Legacy Algorithm Audit Port | Port current layouts/algorithms behind new interfaces | spring/tree/topological/Dijkstra baseline | T1-P3 | adapter implementations + parity tests | behavior parity with existing tutorials where applicable |
| T3-P2 | Routing Policy Layer | Introduce routing abstraction | bezier default + extension points for orthogonal | T2-P1, T2-P2 | routing policy interfaces | edge routing strategy swappable without Ctrl changes |
| T3-P3 | Layered/DAG Generalization | Upgrade ordered layout beyond strict assumptions | generalized DAG layering heuristics | T3-P1 | new layered layout module | works on non-perfect-tree DAG fixtures |
| T3-P4 | Snap/Guide Geometry Core | Add grid and alignment computation kernels | snapping targets, guide extraction | T2-P1 | geometry helpers | Ctrl can consume snap candidates without custom math |

### T4: Core Editor State + Command Stack
| Phase ID | Title | Objective | Scope | Prerequisites | Deliverables | Exit Criteria |
|---|---|---|---|---|---|---|
| T4-P1 | Editor State Model | Define headless interaction state | selection sets, hover, tool mode, viewport state | T1-P3, T2-P1 | editor-state structs + invariants | no persistent-doc pollution from transient state |
| T4-P2 | Command Protocol | Define command objects + execution contracts | add/remove/move/connect/group/style operations | T4-P1, T1-P3 | command API spec | every mutating action representable as command |
| T4-P3 | Undo/Redo Engine | Implement history stack and reversible commands | transaction grouping, merge/coalesce rules | T4-P2 | command stack core | undo/redo test suite passes on composite edits |
| T4-P4 | Selection & Picking Integration | Connect hit-test outputs to selection commands | box, click, modifier policies | T2-P4, T4-P2 | selection command implementations | consistent selection behavior across all entity types |

### T5: Ctrl Integration & Interaction Shell
| Phase ID | Title | Objective | Scope | Prerequisites | Deliverables | Exit Criteria |
|---|---|---|---|---|---|---|
| T5-P1 | Viewport Ctrl Skeleton | Create minimal `Ctrl` consuming core scene and commands | paint bridge, basic event dispatch | T2-P5, T4-P2 | initial `Node/Ctrl` control | can render static graph and perform click selection |
| T5-P2 | Input Mapping Layer | Implement mouse/keyboard mappings to core commands | drag move, drag connect, box select, shortcuts | T5-P1, T4-P4 | input adapter module | no direct model mutation in Ctrl code |
| T5-P3 | Menu/Focus Integration | Add context menus and focus behavior on top of command APIs | node/pin/edge/group menus and key focus | T5-P2 | menu/focus wiring | UI actions route through command stack consistently |
| T5-P4 | Clipboard/DragDrop Bridge | Add OS clipboard and optional drag/drop bridge | payload encode/decode via Core schema | T1-P4, T5-P2 | clipboard integration module | copy/paste round-trip works across sessions/documents |

### T6: Widgets-Inside-Nodes Host Layer
| Phase ID | Title | Objective | Scope | Prerequisites | Deliverables | Exit Criteria |
|---|---|---|---|---|---|---|
| T6-P1 | Widget Slot Model | Define core-side node widget slots/anchors and metadata | persistent slot descriptors and constraints | T1-P3, T2-P2 | slot descriptor model in Core | node documents can describe widget slots without Ctrl types |
| T6-P2 | Ctrl Host Container | Implement Ctrl-side child-widget host bound to slots | lifecycle, mapping, geometry syncing | T6-P1, T5-P1 | host container control | child `Ctrl`s position and update with viewport changes |
| T6-P3 | Focus/Input Arbitration | Resolve text editing and widget interaction conflicts | focus handoff rules, gesture ownership | T6-P2, T5-P3 | focus policy layer | typing in widgets does not trigger graph-edit shortcuts |
| T6-P4 | Widget Persistence Hooks | Persist widget-bound values via Core document channels | value binding contracts | T1-P4, T6-P1 | binding API + sample widgets | widget values survive save/load |

### T7: Performance & Scaling
| Phase ID | Title | Objective | Scope | Prerequisites | Deliverables | Exit Criteria |
|---|---|---|---|---|---|---|
| T7-P1 | Baseline Metrics & Budgets | Define performance targets and profiling fixtures | graph sizes, frame/update budgets | T5-P1 | benchmark fixtures + thresholds | measurable baseline established |
| T7-P2 | Scene Cache & Dirty Regions | Introduce cache invalidation strategy in Core/bridge | scene diff, region invalidation | T2-P2, T5-P1 | cache subsystem | repaint cost reduced on localized edits |
| T7-P3 | Spatial Indexing for Hit Tests | Replace linear scans with indexes | node/pin/edge pick acceleration | T2-P4 | spatial index module | pick cost scales sublinearly on large graphs |
| T7-P4 | Incremental Layout/Path Update | Recompute only affected subgraphs where possible | edge/path/layout invalidation graph | T3-P2, T7-P2 | incremental update engine | large edit operations avoid full-scene recompute |

### T8: Migration & Compatibility Surface
| Phase ID | Title | Objective | Scope | Prerequisites | Deliverables | Exit Criteria |
|---|---|---|---|---|---|---|
| T8-P1 | Compatibility Inventory Freeze | Lock list of legacy APIs/tutorial behaviors to support | GraphLib1..4 and selected APIs | T1-P1 | compatibility matrix | migration scope agreed and testable |
| T8-P2 | Data Migration Adapters | Create import/export adapters from legacy GraphLib data | legacy mapping + warnings | T1-P4 | adapter layer | existing graphs/tutorial data can be loaded |
| T8-P3 | Facade/Shim Layer | Add transitional API wrappers in Ctrl/Core | minimal compatibility wrappers | T5-P2, T8-P2 | shim interfaces | old examples compile with constrained changes |
| T8-P4 | Tutorial & Sample Migration | Rehome and modernize GraphLib tutorials to Node packages | sample refresh and docs | T8-P3 | updated tutorials + migration notes | tutorials validate new architecture end-to-end |

## 6. Cross-Track Dependency Graph
Blocking relationships:
- **T1 blocks almost everything**: model and schema decisions are foundational.
- **T2 depends on T1**: scene descriptors require stable model fields and IDs.
- **T4 depends on T1 and T2**: command payloads need stable model, selection/picking needs hit geometry.
- **T5 depends on T2 and T4**: Ctrl cannot be clean until core render/command contracts exist.
- **T6 depends on T5 and T1**: widget hosting requires both core slot descriptors and Ctrl host infrastructure.
- **T7 depends on T2/T5/T3 maturity**: optimization should follow baseline architecture, not precede it.
- **T8 runs partially in parallel** (inventory early), but adapters/facades need T1/T5 contracts.

Critical path:
1. T1 (model + document)
2. T2 (scene/render kernel)
3. T4 (command stack/editor state)
4. T5 (Ctrl integration)
5. T8 (compatibility rollout)

Parallelizable work:
- T3 can proceed after T1 with partial overlap with T2.
- T8-P1 can begin early (inventory/testing).
- T7-P1 (metrics harness) can begin once first Ctrl skeleton exists.

Why persistence/document design should be early:
- It defines the long-lived contract for IDs, ownership, and schema migration.
- Delaying it risks rework in commands, scene generation, clipboard payloads, and compatibility adapters.

Why draw/render architecture should come before deep Ctrl integration:
- Without core-side render contracts, Ctrl code tends to absorb rendering/path logic again.
- Early render-kernel definition enforces the boundary rule and prevents monolithic `GraphNodeCtrl` patterns.

When embedded node widgets should begin:
- After command/input and paint bridges are stable (post T5-P2/P3).
- Starting too early creates focus/ownership churn and boundary leakage into Core.

## 7. Design Decisions That Must Be Fixed Early
| Decision | Why it matters | Owning package |
|---|---|---|
| Global ID model (stable IDs for nodes/pins/edges/groups) | Needed for serialization, commands, references, migration adapters | `Node/Core` |
| Ownership/lifetime model (document entities vs runtime state) | Prevents accidental state mixing and persistence bugs | `Node/Core` |
| Model vs scene vs editor-state split | Core architectural seam for scalability and testability | `Node/Core` |
| Coordinate spaces (world/view/screen) and transform directionality | Required for rendering, hit testing, and input mapping consistency | `Node/Core` |
| Serialization format and versioning policy | Locks migration strategy and compatibility guarantees | `Node/Core` |
| Command architecture (granularity, transactions, merge rules) | Determines undo/redo behavior and API stability | `Node/Core` |
| Render API boundary (scene command contract to Ctrl) | Enforces rendering-in-Core rule and keeps Ctrl thin | `Node/Core` (contract), `Node/Ctrl` (adapter impl) |
| Hit testing ownership (fully core-side geometry vs shared) | Avoids duplicated picking logic and inconsistent behavior | `Node/Core` |
| Selection semantics (single/multi/range/box precedence) | Impacts interaction correctness and user expectations | `Node/Core` policy, `Node/Ctrl` input mapping |
| Widget slot contract for embedded controls | Prevents Core from depending on `Ctrl` types | Core descriptors in `Node/Core`, host logic in `Node/Ctrl` |

## 8. Recommended Generation Order for Future Task Files
Recommended batching strategy for `./uppsrc/Node/plan/<track>/<phase>/<task>.md` generation:
1. Batch A (foundation):
   - Generate all tasks for T1-P1..P4 first.
   - Include validation tasks and schema fixture tasks in same batch.
2. Batch B (core rendering backbone):
   - Generate T2-P1..P5 tasks next.
   - End with explicit Core-to-Ctrl paint bridge contract tasks.
3. Batch C (core editing behavior):
   - Generate T4-P1..P4 tasks.
   - Include undo/redo conformance tests before Ctrl integration tasks.
4. Batch D (algorithm expansion in parallel):
   - Generate T3 tasks and allow T3-P1/P2 parallel with late T2/T4 stabilization.
5. Batch E (Ctrl shell):
   - Generate T5 tasks after B+C are mostly complete.
6. Batch F (widgets + performance + migration):
   - Generate T6, T7, and T8 tasks, sequencing T8-P1 early and T8-P4 late.

Ordering rule:
- Do not generate Ctrl-heavy task files before Core contracts are frozen.
- Prefer one track-phase wave at a time, with explicit cross-phase handoff checks.

## 9. Risks and Migration Notes
Technical debt risks:
- Current `GraphLib` combines data/visual/editor states in shared structs; naive porting can copy this debt into `Node/Core`.
- Monolithic interaction code in `GraphNodeCtrl.cpp` can tempt boundary violations if directly transplanted.

Partial-feature traps:
- `LoadGraph` is missing while save exists; migration may appear complete but fail at round-trip.
- Group collapse currently toggles a flag only; users may assume behavior that is not implemented.
- Edge selection is partially present but interaction paths are inconsistent.

Package-boundary violation risks:
- Reintroducing path generation and hit testing into Ctrl for convenience.
- Putting menu/shortcut semantics in Core instead of keeping Core command-oriented and UI-agnostic.

Likely refactoring hotspots:
- `GraphLib.h` (`GraphLayout<T>` inheritance stack and buffer rendering).
- `GraphNodeCtrl.cpp` (input handling, direct state mutation, persistence stubs).
- `Renderer.cpp` (should become Core scene/painter pipeline, not Ctrl utility).
- `Graph.h` entity field decomposition into persistent vs transient domains.

Migration notes:
- Keep old tutorials as compatibility tests during transition.
- Introduce adapters instead of large flag-day rewrites.
- Freeze schema and command contracts before broad UI migration.

## 10. Evidence Appendix
Key files reviewed and why they matter:
- `tutorial/GraphLib1/main.cpp`: confirms legacy integer-edge API and tree-layout usage expectations.
- `tutorial/GraphLib2/main.cpp`: confirms ordered/DAG-like usage patterns and implicit node creation.
- `tutorial/GraphLib3/main.cpp`: confirms shape/style and directed edge usage.
- `tutorial/GraphLib4/main.cpp`: confirms weighted pathfinding highlighting behavior.
- `tutorial/GraphLib{1..4}/*.upp`: confirms package-level tutorial intent and dependency on `GraphLib`.
- `uppsrc/GraphLib/GraphLib.upp`: authoritative list of compiled files; indicates stale headers not part of build.
- `uppsrc/GraphLib/GraphLib.h`: `GraphLayout<T>` coupling of model/layout/render/ctrl behavior and cached painting.
- `uppsrc/GraphLib/Graph.h` + `Graph.cpp`: current model and CRUD behavior, including mixed persistent/transient fields.
- `uppsrc/GraphLib/Renderer.h/.cpp`: current render, edge path, transform, and hit helper logic.
- `uppsrc/GraphLib/GraphNodeCtrl.h/.cpp`: current editor interactions, shortcuts, context menus, copy/paste, persistence gaps.
- `uppsrc/GraphLib/Layout.h/.cpp`: base layout abstraction and current incompleteness (`LayoutPrepare` throw).
- `uppsrc/GraphLib/Spring.*`, `OrderedTree.*`, `TournamentTree.*`: existing layout assets to preserve/refactor.
- `uppsrc/GraphLib/TopologicalSort.*`, `Dijkstra.*`, `BinaryHeapMap.*`: algorithm assets for Core algorithm layer.
- `uppsrc/GraphLib/GroupNode.h`: current grouping model and collapse/select fields.
- `uppsrc/GraphLib/Raphael.h`, `Snap.h`: stale legacy artifacts, should be dropped rather than migrated.
