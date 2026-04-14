# Node Plan Audit Report

## 1. Executive Verdict
The planning tree is **conditionally ready** for implementation. The overall track/phase skeleton is coherent, package ownership is mostly disciplined, and the generated tasks preserve the Core/Ctrl separation intent in most files.

The strongest parts are the explicit track decomposition, broad coverage of Core-first responsibilities, and repeated boundary reminders inside Node/Ctrl tasks. The plan is also materially better after this audit because several dependency chains were corrected to match `PLAN-SKELETON.md` phase intent.

The highest-risk weaknesses are task quality and dependency precision at the task level. A subset of tasks are still too broad for one focused implementation item, and many Ctrl-heavy tasks rely mostly manual validation without contract-level automated checks. There is also residual ambiguity around bridge-phase ownership (Core scene track containing Node/Ctrl fixture work), which can cause execution-order confusion.

The plan can proceed, but it should be treated as **execution-ready with guardrails**, not fully hardened. Teams should prioritize the corrections listed in Sections 6–8 before parallel implementation ramps up.

## 2. Audit Scope
Reviewed artifacts:
- `./uppsrc/Node/plan/README.md`
- `./uppsrc/Node/plan/TRACK-INDEX.md`
- `./uppsrc/Node/plan/PLAN-SKELETON.md`
- All generated task files under `./uppsrc/Node/plan/<track>/<phase>/<task>.md` (66 files)

Supporting architectural evidence reviewed:
- `./uppsrc/GraphLib/GraphLib.h`
- `./uppsrc/GraphLib/Graph.h`
- `./uppsrc/GraphLib/Renderer.{h,cpp}`
- `./uppsrc/GraphLib/GraphNodeCtrl.{h,cpp}`
- `./tutorial/GraphLib1/main.cpp`
- `./tutorial/GraphLib2/main.cpp`
- `./tutorial/GraphLib3/main.cpp`
- `./tutorial/GraphLib4/main.cpp`

Architectural authority used:
1. `PLAN-SKELETON.md` (primary planning authority)
2. Core/Ctrl constraints from the task prompt
3. GraphLib/tutorial code as migration reality checks

## 3. Summary Findings
| Area | Verdict | Notes |
|---|---|---|
| Track structure | Good | 8-track structure matches skeleton intent and migration workflow. |
| Phase structure | Good | Phase order is consistent with critical-path framing. |
| Task granularity | Needs Work | Several tasks are still too broad (algorithm bundles, full tutorial migration, full scene builder baselines). |
| Package ownership | Good | Ownership labels are mostly correct; `Boundary` is used intentionally for contract freezes. |
| Core/Ctrl boundary discipline | Needs Work | No hard violations found, but some tasks are boundary-leak prone if implemented literally. |
| Rendering ownership | Good | Rendering/path/style ownership is consistently stated as Core-first. |
| Dependency graph | Needs Work | Multiple mismatches existed; major ones were corrected in this audit, but a few sequencing ambiguities remain. |
| Acceptance criteria | Needs Work | Many criteria are generic and not quantitatively verifiable. |
| Validation strategy | High Risk | Ctrl and integration tasks often rely on manual checks with limited automated fixtures. |
| Migration coverage | Needs Work | Coverage exists, but parity for nuanced legacy behavior (e.g., tutorial-specific interactions and payload semantics) is still under-specified. |

## 4. Package Boundary Violations
No **confirmed hard** Core/Ctrl ownership violations were found in current task metadata.

Potential boundary-risk items that require stricter execution discipline:

1. `./uppsrc/Node/plan/core-scene-render/05-draw-bridge-contract/02-prepare-ctrl-bridge-check-fixtures.md`
- Current problem: Node/Ctrl-owned fixture task is located under a Core-scene track/phase.
- Why this can violate architecture: teams may treat it as Core render implementation work and accidentally push draw-command semantics into Ctrl.
- Recommended correction: keep task but explicitly gate it as bridge-playback fixture only; keep scene/path/style semantics frozen in Core contract task.

2. `./uppsrc/Node/plan/ctrl-integration/02-input-mapping-layer/01-map-pointer-events-to-core-commands.md`
- Current problem: scope includes rich gestures (drag, marquee, link creation).
- Why this can violate architecture: gesture semantics can drift into Ctrl if selection/connect policies are reimplemented there.
- Recommended correction: add explicit sub-checklist requiring all selection/connect decisions to call Core command/pick contracts.

3. `./uppsrc/Node/plan/widgets-inside-nodes/02-ctrl-host-container/02-map-slot-geometry-to-child-ctrl-layout.md`
- Current problem: “map slot geometry” can be interpreted as recomputing geometry in Ctrl.
- Why this can violate architecture: slot geometry ownership is Core-side; Ctrl should consume resolved geometry only.
- Recommended correction: require Core-provided slot rectangles/transforms as input; forbid Ctrl-side geometric derivation.

4. `./uppsrc/Node/plan/ctrl-integration/04-clipboard-dragdrop-bridge/01-implement-os-clipboard-adapter.md`
- Current problem: clipboard bridge can tempt Ctrl-side payload parsing.
- Why this can violate architecture: document semantics and parsing must remain Core-owned.
- Recommended correction: require payload decode/validation entrypoints in Core (Ctrl transports bytes only).

## 5. Weak or Oversized Tasks
1. `./uppsrc/Node/plan/core-model-document/03-core-model-implementation/01-implement-core-document-entities.md`
- Issue type: too large
- Problem: combines full entity set + containers + mutation APIs + invariants in one item.
- Recommended split: (a) entity type definitions, (b) container/index APIs, (c) mutation integrity checks.

2. `./uppsrc/Node/plan/core-scene-render/02-scene-descriptor-model/02-implement-scene-builder-baseline.md`
- Issue type: too coupled
- Problem: likely mixes descriptor assembly, style resolution stubs, and scene completeness logic.
- Recommended split: (a) node/group descriptor builder, (b) edge/label descriptor builder, (c) full-scene assembly integration.

3. `./uppsrc/Node/plan/core-layout-routing/01-legacy-algorithm-audit-port/02-port-tree-and-shortestpath-algorithms.md`
- Issue type: too large
- Problem: tree layout + topological ordering + shortest path in one item.
- Recommended split: separate algorithm ports with distinct parity fixtures.

4. `./uppsrc/Node/plan/ctrl-integration/02-input-mapping-layer/01-map-pointer-events-to-core-commands.md`
- Issue type: too large
- Problem: click/drag/marquee/link creation is a large interaction surface.
- Recommended split: pointer select/move first, then link creation, then marquee behavior.

5. `./uppsrc/Node/plan/ctrl-integration/03-menu-focus-integration/01-implement-context-menu-command-bindings.md`
- Issue type: weak acceptance criteria
- Problem: success criteria do not require per-entity menu command coverage matrix.
- Recommended split/rewrite: add acceptance checklist by entity type (node/pin/edge/group) and command route verification.

6. `./uppsrc/Node/plan/ctrl-integration/04-clipboard-dragdrop-bridge/01-implement-os-clipboard-adapter.md`
- Issue type: weak validation
- Problem: mostly manual checks; no explicit fixture-based payload validation.
- Recommended split/rewrite: add contract/golden payload tests shared with Core DocumentIO schema.

7. `./uppsrc/Node/plan/widgets-inside-nodes/02-ctrl-host-container/01-implement-node-widget-host-container.md`
- Issue type: too coupled
- Problem: lifecycle + slot binding + viewport sync in one step.
- Recommended split: host lifecycle shell first, then geometry sync, then runtime binding hooks.

8. `./uppsrc/Node/plan/migration-compat/03-facade-shim-layer/01-implement-transitional-core-facade.md`
- Issue type: too vague
- Problem: “prioritized compatibility operations” is not scoped to explicit API subset.
- Recommended split/rewrite: define exact GraphLib symbols first, then implement per-symbol shim group.

9. `./uppsrc/Node/plan/migration-compat/04-tutorial-sample-migration/01-migrate-graphlib-tutorials-to-node.md`
- Issue type: too large
- Problem: all tutorials in one task; integration/debug scope is too broad.
- Recommended split: one task per tutorial package with parity checklist.

10. `./uppsrc/Node/plan/performance-scaling/02-scene-cache-dirty-regions/02-implement-core-scene-cache-policy.md`
- Issue type: weak acceptance criteria
- Problem: “measurable improvement” not bound to named budgets or scenarios.
- Recommended split/rewrite: require explicit budget IDs from performance contract and threshold pass/fail values.

## 6. Dependency Problems
Problems found and disposition:

1. `./uppsrc/Node/plan/core-scene-render/01-coordinate-system-spec/01-freeze-coordinate-space-contract.md`
- Current dependency state (before audit): depended on Core model implementation (too late).
- Recommended correction: depend on early domain-boundary freeze.
- Status: **Fixed in audit**.

2. `./uppsrc/Node/plan/core-editor-commands/01-editor-state-model/01-define-editor-runtime-state.md`
- Current dependency state (before audit): depended on late hit-testing API; blocked T4-P1 unnecessarily.
- Recommended correction: depend on Core model entities + coordinate-space contract.
- Status: **Fixed in audit**.

3. `./uppsrc/Node/plan/core-layout-routing/02-routing-policy-layer/01-define-routing-policy-contract.md`
- Current dependency state (before audit): depended on already-implemented edge path generator, reversing intended flow.
- Recommended correction: depend on scene descriptor model + legacy algorithm port phase.
- Status: **Fixed in audit**.

4. `./uppsrc/Node/plan/migration-compat/01-compatibility-inventory-freeze/01-freeze-graphlib-compatibility-matrix.md`
- Current dependency state (before audit): depended on schema-v1 task, delaying compatibility inventory.
- Recommended correction: depend on early domain boundary classification.
- Status: **Fixed in audit**.

5. `./uppsrc/Node/plan/widgets-inside-nodes/04-widget-persistence-hooks/01-define-widget-value-binding-contract.md`
- Current dependency state (before audit): lacked explicit persistence dependency.
- Recommended correction: depend on Core document reader/writer task.
- Status: **Fixed in audit**.

6. `./uppsrc/Node/plan/core-scene-render/02-scene-descriptor-model/01-design-scene-descriptor-types.md`
- Current dependency state (before audit): missing explicit dependency on Core document entities.
- Recommended correction: add dependency on Core model implementation task.
- Status: **Fixed in audit**.

7. `./uppsrc/Node/plan/core-model-document/03-core-model-implementation/01-implement-core-document-entities.md`
- Current dependency state (before audit): over-constrained on schema versioning.
- Recommended correction: align with skeleton by depending on boundary split (allow parallel schema evolution).
- Status: **Fixed in audit**.

8. `./uppsrc/Node/plan/core-model-document/04-persistence-implementation/01-implement-document-reader-writer.md`
- Current dependency state (before audit): did not explicitly depend on schema/version rules.
- Recommended correction: add versioning/schema dependency.
- Status: **Fixed in audit**.

9. `./uppsrc/Node/plan/migration-compat/04-tutorial-sample-migration/01-migrate-graphlib-tutorials-to-node.md`
- Current dependency state (before audit): lacked explicit dependency on compatibility fixtures.
- Recommended correction: add fixture dependency and explicit GraphLib1..4 parity criterion.
- Status: **Fixed in audit**.

10. `./uppsrc/Node/plan/core-scene-render/05-draw-bridge-contract/02-prepare-ctrl-bridge-check-fixtures.md`
- Current dependency state: depends only on paint contract freeze.
- Recommended correction: optional explicit dependency on viewport shell task if fixture harness needs concrete Ctrl wrapper scaffolding.
- Status: **Open (non-blocking)**.

## 7. Coverage Gaps
1. Clipboard payload schema contract is under-specified.
- Current state: Ctrl clipboard adapter references Core DocumentIO generally.
- Gap: no explicit task freezing a dedicated clipboard/subgraph payload contract (full-doc vs subgraph fragment semantics, metadata, version tag).

2. Scene-vs-document-vs-editor split is present but not fully traceable.
- Current state: tasks exist for split, scene descriptors, and editor state.
- Gap: no single traceability artifact tying each migrated GraphLib field to exactly one state layer with owner and persistence policy.

3. Invalid data handling is only partially concrete.
- Current state: serialization tasks mention structured errors.
- Gap: no dedicated malformed-input matrix (unknown entity IDs, dangling pin refs, duplicate IDs, version skew policy) as acceptance fixture inventory.

4. Legacy tutorial parity depth is still thin.
- Current state: tutorial migration task now explicitly references GraphLib1..4 parity.
- Gap: no per-tutorial behavior checklist task (e.g., GraphLib4 route-weight click workflow, GraphLib3 shape/label rendering semantics).

5. Hit-testing ownership is correct but performance-linked semantics are under-defined.
- Current state: Core hit-query API and spatial indexing tasks exist.
- Gap: no explicit policy for tie-breaking/priority (pin vs edge vs node vs group) under overlap conditions.

6. Widget split is mostly correct but value-flow lifecycle remains coarse.
- Current state: Core slot model and Ctrl host split are explicit.
- Gap: no separate task for delayed validation/commit behavior (on-change vs on-commit) tied to Core command batching.

## 8. Recommended Corrections
### Critical
1. Add/strengthen automated contract validation for Ctrl bridge tasks.
- Targets: `ctrl-integration/*`, `core-scene-render/05-draw-bridge-contract/02-*`.

2. Split oversized migration and interaction tasks into narrower executable items.
- Targets: `migration-compat/04-tutorial-sample-migration/01-*`, `ctrl-integration/02-input-mapping-layer/01-*`, `core-layout-routing/01-legacy-algorithm-audit-port/02-*`.

3. Define explicit clipboard payload contract as Core-owned boundary artifact.
- Targets: `ctrl-integration/04-clipboard-dragdrop-bridge/01-*` plus a new Boundary/Core contract task.

### Important
1. Add a state-layer traceability table task (Document vs Scene vs Editor runtime).
- Targets: `core-model-document/01-*`, `core-scene-render/02-*`, `core-editor-commands/01-*`.

2. Add explicit hit-priority/tie-break contract task for overlap cases.
- Targets: `core-scene-render/04-hit-geometry-core/01-*`.

3. Require budget-ID linkage in performance acceptance criteria.
- Targets: `performance-scaling/02-*`, `performance-scaling/03-*`, `performance-scaling/04-*`.

### Nice to Have
1. Add per-tutorial parity checklist docs to reduce regression ambiguity.
- Targets: `migration-compat/04-*`.

2. Add explicit “non-goals” matrix for legacy features that remain intentionally dropped.
- Targets: `migration-compat/01-*`, `migration-compat/02-*`.

3. Normalize validation wording so every task declares at least one automated validation path when feasible.
- Targets: all tasks that currently rely mainly on manual checks.

## 9. Edit Log
Edits were made to planning files during this audit.

1. `./uppsrc/Node/plan/core-scene-render/01-coordinate-system-spec/01-freeze-coordinate-space-contract.md`
- Changed dependency from late Core model implementation to early ID-boundary task.
- Why: align with skeleton prerequisite timing and avoid unnecessary critical-path delay.

2. `./uppsrc/Node/plan/core-editor-commands/01-editor-state-model/01-define-editor-runtime-state.md`
- Replaced dependencies to match T4-P1 intent (`T1-P3` + `T2-P1` equivalents).
- Why: remove incorrect dependence on late hit-query API.

3. `./uppsrc/Node/plan/core-layout-routing/02-routing-policy-layer/01-define-routing-policy-contract.md`
- Replaced dependency on edge path generator with dependency on scene descriptor design.
- Why: restore intended order (routing contract should precede path-generation implementation coupling).

4. `./uppsrc/Node/plan/migration-compat/01-compatibility-inventory-freeze/01-freeze-graphlib-compatibility-matrix.md`
- Changed dependency from schema-v1 task to early domain-boundary split task.
- Why: allow compatibility inventory to start early as skeleton intended.

5. `./uppsrc/Node/plan/widgets-inside-nodes/04-widget-persistence-hooks/01-define-widget-value-binding-contract.md`
- Added dependency on Core document reader/writer.
- Why: binding contract requires concrete persistence semantics.

6. `./uppsrc/Node/plan/core-scene-render/02-scene-descriptor-model/01-design-scene-descriptor-types.md`
- Added dependency on Core document entities task.
- Why: scene descriptors depend on stabilized model entities.

7. `./uppsrc/Node/plan/core-model-document/03-core-model-implementation/01-implement-core-document-entities.md`
- Changed dependency from schema-versioning to persistent/runtime split task.
- Why: align with skeleton phase dependency and permit schema/model parallelization.

8. `./uppsrc/Node/plan/core-model-document/04-persistence-implementation/01-implement-document-reader-writer.md`
- Added explicit dependency on schema/versioning task.
- Why: persistence cannot be finalized without versioning rules.

9. `./uppsrc/Node/plan/migration-compat/04-tutorial-sample-migration/01-migrate-graphlib-tutorials-to-node.md`
- Added dependency on compatibility regression fixtures.
- Added acceptance criterion explicitly covering `GraphLib1`..`GraphLib4` parity.
- Why: make migration execution/testability concrete.

10. `./uppsrc/Node/plan/TRACK-INDEX.md`
- Updated track ownership wording for `core-scene-render` to acknowledge one Node/Ctrl fixture task.
- Why: remove ownership-summary inconsistency.

## 10. Evidence Appendix
Key files reviewed and why they mattered:

- `./uppsrc/Node/plan/PLAN-SKELETON.md`
  - Baseline architecture, track/phase prerequisites, and Core/Ctrl ownership authority.

- `./uppsrc/Node/plan/README.md`
  - Execution-order and package-boundary rules used to assess implementation readiness.

- `./uppsrc/Node/plan/TRACK-INDEX.md`
  - Cross-track dependency and ownership summary; checked for consistency with task files.

- All task files under `./uppsrc/Node/plan/*/*/*.md`
  - Audited for ownership, dependencies, acceptance criteria quality, and validation realism.

- `./uppsrc/GraphLib/GraphLib.h`
  - Evidence of mixed model/layout/render/Ctrl coupling in `GraphLayout<T>`.

- `./uppsrc/GraphLib/GraphNodeCtrl.h` and `./uppsrc/GraphLib/GraphNodeCtrl.cpp`
  - Evidence of monolithic UI + domain mutation, ad-hoc persistence, and interaction coupling.

- `./uppsrc/GraphLib/Renderer.h` and `./uppsrc/GraphLib/Renderer.cpp`
  - Evidence that path generation/hit/painter logic currently lives in one mixed module and should migrate to Core rendering subsystems.

- `./uppsrc/GraphLib/Graph.h`
  - Evidence for mixed persistent/transient/runtime fields motivating model-layer separation tasks.

- `./tutorial/GraphLib1/main.cpp` to `./tutorial/GraphLib4/main.cpp`
  - Evidence for migration parity surface (layout styles, directed edges, labels, Dijkstra/route-weights interaction).
