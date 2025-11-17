# Packet Router Migration Roadmap
**Goal:** Replace the loop-centric Eon dataflow runtime with a router-based architecture where Atoms expose explicit ports and all packet movement is mediated by a PacketRouter rather than Link chains.

---

## Context & Targets
- **Task thread:** `task/PacketRouter.md` — convert loop-based Eon graphs into router-managed networks.
- **Primary motivator:** Side links, customers, and loop balancing no longer match how we want packet flow to work. We need uniform, nameable ports, router-managed flow control, and more flexible network topology.
- **Scope reminder:** Touches most of `uppsrc/Eon`, the `.eon` DSL (loaders + test data), router-facing VFS/ECS plumbing, and downstream `api/*` implementations that currently depend on loop semantics.

---

## Current Loop-Based Snapshot
- **Link stack (`Eon/Core/LinkBase.h`, `LinkFactory.h`):** Hardcodes customer/pipe/poller/etc. types that physically sit between Atom instances in a loop. They own packet queues, negotiation, and time slicing.
- **CustomerBase (`Eon/Core/Base.h`):** Injects demand-driven packet generation. Customer atoms pin the loop queue depth (constant packets per loop) and enforce “primary” ports.
- **Exchange providers (`Vfs/Ecs/Interface.*`):** Expose sink/source containers that the loop builder resolves into hardwired channel indexes (0 = primary). Side links are special cases.
- **Script DSL:** `.eon` uses `chain` + `loop` blocks. Loops imply circular topologies, rely on automatic Link selection, and require `.out/.in` qualifiers for sides.
- **Test harness (`upptst/Eon*/`, esp. Eon00 `00a/00b/00c`):** Provides method 0 (parse `.eon`), method 1 (AST), method 2 (direct VFS builder) forms of the same loop.

This arrangement keeps packet counts predictable but makes it impossible to create arbitrary graphs, multi-source routers, or on-demand packet creation outside Customer atoms.

---

## Router-First Vision
- **PacketRouter core:** Exchange classes morph into router providers (`RouterSourceProvider`, `RouterSinkProvider`, `RouterConnection`, etc.) that map Atom ports to router entries instead of enumerating side/pipe links.
- **Ports:** All Atom channels become peer ports with optional names. Primary/secondary distinction disappears; the router tracks direction (src/sink) and metadata instead.
- **Flow control:** Replace per-loop packet pools with router-governed credit/buffer management plus optional metadata (`MetaDataFormat` addition under `Vfs/Ecs/Formats.h`). CustomerBase goes away; Atoms expose hooks to request/ack credits on their ports.
- **Script DSL:** `chain` → `net`, loops vanish. New syntax `atom_id: action` + `connections: atom:port -> atom:port`. `.out/.in` qualifiers drop; colon notation + optional port names unify everything.
- **Tooling rename:** `ChainContext`, `Chain->Net`, ToyLoader rewriting, etc.
- **Compatibility:** Ability to emulate legacy loop scheduling (optional router policy) so we can keep older workloads working while migrating.

---

## Reference Pilot – `upptst/Eon00`
- **Reasoning:** `Eon00` already demonstrates three load paths, including `method 2` that writes loops directly into the VFS without script parsing. Converting this package first gives us:
  - A minimal `.eon` (`share/eon/tests/00a_audio_gen.eon`) that we can rewrite with the new `net` syntax.
  - Manual builders (`ChainContext`, `LoopContext`) we can recast into prototype router builders before touching the parser.
  - A fast test harness (small audio pipeline) to prove router semantics without touching SDL/Gfx stacks.
- **Plan:** Fork `upptst/Eon00` in the new branch, add router prototypes, and keep method 0/1/2 compiling until we complete the DSL migration.
- **Status update:** Method 3 now shares RouterNetContext instances through a helper that connects loops via `LoopContext::ConnectSides`, so `00b/00c` mimic ScriptLoader’s side-link parity. Conversion guides for `00a/00b/00c` live next to the `.eon` assets for future rewrites. The helper moved into `upptst/EonRouterSupport` and `upptst/Eon02` method 3 runners (`02a`, `02b`, `02c`, `02d`, `02e`, `02f`, `02g`, `02l`, `02m`) now exercise the same router nets to cover single-loop audio, chain-linked side loops, and the fluidsynth/softinstru/fmsynth/coresynth+corefx/portmidi event/input bridges. `RouterPortDesc`/`RouterConnectionDesc` have been defined inside `uppsrc/Vfs/Ecs/Interface.h` so runtime-facing packages share a single descriptor format before serialization lands, complete with metadata + serialization helpers covered by the `upptst/Router` test harness, and `Vfs/Storage` now ships both JSON and binary fragment/overlay writers (`VfsSaveFragmentBinary`, `VfsSaveOverlayIndexBinary`, and matching loaders) verified via the Router console build + gdb run. Scenario-focused suites `upptst/RouterFanout` and `upptst/RouterPool` extend that coverage with multi-port fan-out + limited packet-pool metadata checks, and the base `upptst/Router` suite now exercises the builder path end-to-end by emitting fragments + overlay indexes via `BuildRouterOverlayIndex` and asserting JSON/binary equality. IDE package saves now call those helpers automatically whenever `Meta.bin` updates, emitting `.fragment.json` + `.fragment.vfsbin` and overlay counterparts while `MetaEnvTree` loads the cached overlay indexes to surface router metadata alongside each inspected node, and MetaCtrl’s overlay tree now inlines router summaries (flow-control policy + credit hints) beside each logical node. Overlay builders now stream router metadata into `.overlay.vfsch` chunk files through the same traversal so JSON/binary artifacts stay in sync without rebuilding fragments, and MetaCtrl’s inspectors render per-connection details (burst, pool hints, flow-control JSON) to make those stored hints actionable.

---

## Impact Overview (files called out in task)
- **`uppsrc/Eon/Core/LinkBase.h` / `LinkFactory.h`:** Replace with router-aware providers. Side-link, pipe-link, customer semantics go away; router handles adjacency and scheduling.
- **`uppsrc/Eon/Core/Base.h`:** `CustomerBase` must be removed. All Atoms gain port-aware virtuals (e.g., `OnPortReady`, `EmitPacket(port_id, Packet&)`, `RequestCredits`). `Atom.h` mostly survives but gains port registration helpers.
- **`uppsrc/Eon/Script/*`:** DSL grammar, AST, builders, and loaders change entirely. Rename `Chain` → `Net`, produce explicit connection tables, and emit router descriptors instead of loops.
- **`uppsrc/Eon/Script/ToyLoader.cpp`:** Rebuild to emit routers rather than loops (stage I/O → named ports, connections block). Drives shader toy tests and must author the new syntax.
- **`uppsrc/Eon/Script/EcsLoader.{h,cpp}`:** Probably unchanged, but needs verification once the surrounding AST objects rename their nodes (`chain` vs `net` referencing).
- **`uppsrc/Vfs/Ecs/Interface.{h,cpp}`:** Update interfaces to provide named ports, optional lookup by name at connect time, and maintain `Visit(Vis&)`. `InterfaceSink/Source` now wrap Router providers, not Exchange types. Make port naming optional but supported so routers can match `customer0:foo`.
- **`uppsrc/Vfs/Ecs/Link.h`:** Likely reusable (Exchange struct still valid) but expect adjustments for router metadata and new flow-control fields.
- **`uppsrc/Vfs/Ecs/LinkFwd.cpp`:** Re-implement forward/decode logic for router packets. Function names stay but bodies change to build router tables.
- **`uppsrc/Vfs/Ecs/LinkSystem.h`:** Probably survives if we treat systems as router clients; confirm after router prototype.
- **`uppsrc/Vfs/Ecs/Formats.h`:** Extend with `MetaDataFormat` (packet flow metadata) and any router descriptors we need to persist.
- **`share/eon/**/*.eon`:** Rewrite DSL to the new syntax (rename `chain` → `net`, list atoms + `connections:`). This is a sweeping edit.
- **`upptst/Eon*` packages:** Regenerate expanded `.eon` tests. `Eon00` becomes the pilot; other tests follow after DSL conversion.
- **`api/**/*`:** Port IO handling changes—no more implicit loop packet quotas. Each backend (audio/video/gfx) must respect router-managed buffers and the new port APIs.

---

## Phased Roadmap

### Phase 0 – Discovery & Prototype Router Builder
1. **Document loop usage:** Inventory existing Link types, side-link cases, and customer behaviors (derive from `ChainContext`/`LoopContext` data). Output: reference doc.
2. **Eon00 builder spike:** Clone `ChainContext` into an experimental `NetContext` that constructs routers in-memory without script parsing (driven by `Run00aAudioGen` method 2).
3. **Decide port naming scheme:** Define how optional names map to indexes and how to resolve duplicates, plus how to refer to anonymous ports (`customer0:0` fallback).

### Phase 1 – Router Core & Flow Control Primitives
1. **Define `PacketRouter` core API:** Manage port registration, connection tables, packet buffers, and flow-control metadata. Provide `RouterSourceProvider/SinkProvider/Connection` replacements for current Exchange classes.
2. **Refactor `InterfaceBase` derivatives:** Attach port descriptors (direction, format, optional name) and router hooks. Ensure `Visit(Vis&)` still reports the correct type chain.
3. **Remove `CustomerBase`:** Introduce new Atom virtuals for credit management and per-port readiness. Provide helper mixins or macros for existing Atoms to opt in.
4. **Flow-control metadata:** Extend `Vfs/Ecs/Formats.h` to describe buffer hints, throttling, and sync attributes previously handled by loops.

### Phase 2 – DSL & Loader Rewrite
1. **Grammar:** Update parser (`Eon/Script/*`) to support `net` blocks, inline atom definitions (`atom_id: action`), and explicit `connections:` tables.
2. **AST/Builder rename:** `Chain` → `Net`, `Loop` → `RouterNetwork`. Ensure `Builder` and `ScriptLoader` emit router specs instead of loops.
3. **ToyLoader rewrite:** Map ShaderToy stage inputs/outputs directly to named ports and generate router-based `.eon` snippets. Provide conversion utilities for existing data.
4. **ECS loader audit:** Confirm `EcsLoader` references to chain IDs or loop contexts are updated (even if behavior is unchanged).

### Phase 3 – Interface + VFS/ECS Plumbing
1. **Router-aware VFS nodes:** Update `Vfs/Ecs/Interface.*`, `LinkFwd.cpp`, and `LinkSystem.h` to construct routers when scripts are loaded. Keep existing visitor interfaces stable for IDE tooling.
2. **Metadata propagation:** Ensure router descriptors serialize/deserialise via VFS and that IDE/DropTerm inspectors can visualize nets.
3. **Port lookup helpers:** Implement name-based port discovery for initial connections (per the task request). Provide API for atoms to query by name/index.

### Phase 4 – Atom, API, and Backend Conversion
1. **Atom ports:** Update Atom subclasses (especially in `api/*`) to declare ports via the new API (direction, value/device type, optional names). Remove assumptions about channel 0 primaries.
2. **Flow-control integration:** Implement router credit handling inside Atoms that previously relied on `CustomerBase` or loop-limited packet pools.
3. **Backend router adoption:** Update graphics/audio/etc. packages so they call router APIs to push/pull packets. Replace loop-limited packet throttling with router metadata and local buffering.

### Phase 5 – DSL Migration & Test Coverage
1. **Rewrite `.eon` assets:** Convert all `share/eon/**/*.eon` files to the router syntax. Provide conversion scripts (loop detection → connection list) to reduce manual work.
2. **Update `upptst/Eon*`:** Regenerate expanded source equivalents (method 2 builders) to exercise the new router AST. Start with `Eon00`, then roll through other tests.
3. **Introduce regression tests:** Expand `upptst` coverage to ensure router nets can emulate legacy loops (for compatibility) and to validate flow-control metadata.

### Phase 6 – Performance, Compatibility, and Cleanup
1. **Flow-control policy tuning:** Reintroduce “legacy loop” behavior as an optional router policy for workloads that depend on constant packet pools.
2. **Diagnostics:** Update tooling/logging to display router topology, port status, and buffer metrics. This replaces the current loop-centric diagnostics.
3. **Deprecation cleanup:** Remove dead loop APIs, rename residual Chain/Loop types, and ensure docs/AGENTS mention routers everywhere.

---

## Phase Exit Criteria & Deliverables
- **Phase 0 – Discovery & Prototype Router Builder**
  - Artifact: Doc describing every Link/Customer usage site (`Eon/Core`, DSL builders, ECS glue).
  - Code spike: `NetContext` sample in `upptst/Eon00` proving packets can move without loops.
  - Decision record for port naming + anonymous fallback semantics.
- **Phase 1 – Router Core & Flow Control Primitives**
  - `PacketRouter` headers with credit/connection APIs merged (even if backed by stub implementations).
  - All Interface providers compiling with router descriptors; `CustomerBase` deleted or shimmed.
  - Flow-control metadata structs in `Vfs/Ecs/Formats.h` with serialization tests.
- **Phase 2 – DSL & Loader Rewrite**
  - Parser unit tests covering new `net` syntax plus backward-compat toggles.
  - Builder emits router tables consumable by the `NetContext` spike.
  - ToyLoader generates router `.eon` output for at least one ShaderToy example.
- **Phase 3 – Interface + VFS/ECS Plumbing**
  - Router descriptors stored/retrieved via VFS JSON/binary formats.
  - IDE tooling still renders existing graphs (even if still loop-shaped) while router nets are visible in inspectors.
  - Port lookup helpers exposed to script builders and runtime code.
- **Phase 4 – Atom, API, and Backend Conversion**
  - Representative Atoms per backend (audio, gfx, SDL I/O) declare ports via new API.
  - Router-managed credits exercised in at least one backend test (e.g., audio generator).
  - Legacy builds continue running via compatibility policy flag.
- **Phase 5 – DSL Migration & Test Coverage**
  - All `share/eon/**/*.eon` rewritten; automated converter + manual checklist recorded.
  - `upptst/Eon*` packages reflect router nets with method 0/1/2 parity.
  - CI suite includes router + legacy compatibility coverage.
- **Phase 6 – Performance, Compatibility, and Cleanup**
  - Benchmarks comparing loop vs router mode published; any regressions triaged.
  - Diagnostics page/screens for router nets implemented in IDE or DropTerm.
  - Deprecated loop APIs removed from headers and docs.

---

## Collaboration & Tooling Expectations
- **Eon Core + Script team:** Owns router runtime, DSL rewrite, Atom API updates, and the Eon00 pilot.
- **VFS/ECS team:** Defines router descriptors, storage schema, and IDE integration; partners on serialization + inspector updates.
- **Backend owners (`api/audio`, `api/gfx`, etc.):** Provide per-backend port definitions, credit-handling hooks, and regression benchmarks.
- **Tooling support:** Build conversion scripts (likely under `script/` or `task/PacketRouter`) to translate `.eon` files and method 2 builders; share progress logs via `CURRENT_TASK.md`.
- **Documentation:** Update AGENTS, ROADMAP, and TASK files whenever a phase crosses its exit criteria; cross-link to decision logs stored under `pseudocode_analysis/` or `task/notes`.

---

## Metrics & Validation Strategy
- **Functional:** Router nets must recreate packet order/latency guarantees previously enforced by loops (verified via deterministic test scenes and audio samples).
- **Flow control:** Credit accounting traced via debug counters and assertions; target zero packet leaks or starvation over extended runs.
- **Performance:** Track CPU cost per packet in `upptst/Eon00` and a representative SDL/Gfx workload; maintain parity with legacy loops within ±5% before Phase 6 optimizations.
- **Adoption:** Number of `.eon` assets migrated vs total; maintain a running tally in `task/PacketRouter.md`.
- **Compatibility:** Continuous test ensuring loop-compatible policy remains available until all downstream packages opt in.

---

## Supporting Workstreams
- **Conversion tooling:** Scripts to translate legacy `.eon` loops into router nets (including port mapping heuristics).
- **Documentation:** Update `AGENTS.md`, `CURRENT_TASK.md` entries, and developer guides after each major milestone.
- **IDE/Visualizer impact:** Once routers exist, IDE integrations must know how to display them. Track this as a follow-up thread.
- **Serialization prep:** JSON/binary schema outline for router ports, bridges, and flow-control metadata now lives in `task/notes/packet_router_vfs_alignment.md` so VFS/Storage work can start as soon as descriptors stabilize.

---

## Risks & Open Questions
- **Flow control semantics:** Exact replacement for loop packet quotas is undecided. Do we maintain a global packet pool or per-connection credit windows?
- **Cross-net connections:** The new syntax hints at potential `net1.atom:port -> net2.atom:port` links; need concrete rules for scoping and lifecycle.
- **Port naming collisions:** Need deterministic behavior when multiple ports share the same optional name (e.g., arrays). Maybe append indexes automatically.
- **Backwards compatibility:** Some projects may rely on implicit `.out/.in` semantics. Decide whether to support legacy parser mode temporarily.
- **API package breadth:** `api/**/*` touches many backends. We may need staged toggles (router mode vs legacy) while migrating.

---

## Immediate Next Steps
1. **Stand up a `PacketRouter` prototype** beside `ChainContext` inside `upptst/Eon00`, wiring ports manually to confirm data flow.
2. **Draft port descriptor structs** (name, direction, vd tuple, metadata) and sketch how `DefaultInterfaceSink/Source` expose them.
3. **Write conversion notes for `share/eon/tests/00a_audio_gen.eon`** demonstrating the old vs new syntax; this becomes the template for the rest of the assets.
4. **Update `CURRENT_TASK.md` (Eon scope)** once the branch exists so other developers know this sweeping change is happening elsewhere.

Natural follow-up once these land: branch cut, implement Phase 0 prototype, and iterate on the router core before touching the DSL across the repo.
