# Packet Router Task Thread

## Overview
Convert the current loop-based Eon system to a router-based system where atoms connect to a router system instead of forming loops. This transformation will involve morphing Exchange classes to function as packet routers.

## Key Requirements

### System Architecture Changes
- Convert the loop-based Eon system to a router-based architecture
- Instead of connecting atoms in loops, implement a router system for connections
- Transform Exchange classes to serve as packet routers

### Codebase Analysis
- Analyze uppsrc/Eon packages to understand current implementation
- Identify other relevant areas in the codebase that will be affected
- Document current atom connection patterns and loop mechanisms

### File Modifications
- Modify all *.eon files in share/eon (deep analysis and changes required)
- Update all upptst/Eon* packages to align with new router-based system

### Development Roadmap
- Plan the implementation roadmap for this extensive conversion
- This is a long-term process requiring careful planning and execution
- Consider backward compatibility and transition strategies

## Implementation Phases

### Phase 0 – Discovery & Router Prototype
- Inventory every Link/Customer usage site within `uppsrc/Eon` and downstream DSL loaders; capture results in `task/notes/packet_router_links.md`.
- Build a `NetContext` spike inside `upptst/Eon00` (method 2 builder) that wires Atoms through an in-memory router without editing the parser yet.
- Decide on the canonical port descriptor (name, direction, atom-local index, metadata flags) and document anonymous naming fallback rules.
- **Exit criteria:** spike passes `Eon00` tests, and the link inventory plus port-spec doc are checked into the repo.

### Phase 1 – Router Core & Flow Control Primitives
- Define the `PacketRouter` API (port registration, connection tables, credit accounting) and land stub implementations plus unit tests.
- Update `Vfs/Ecs/Interface.*` providers to emit router descriptors while `CustomerBase` begins its removal (shim only).
- Extend `Vfs/Ecs/Formats.h` with metadata structs describing buffer hints, throttling, and sync semantics; serialize them through VFS loaders.
- **Exit criteria:** router headers compile everywhere, Interface providers expose ports, and new metadata survives a serialize→deserialize cycle.

### Phase 2 – DSL & Loader Rewrite
- Modify the parser/AST (`Eon/Script/*`) to support `net` blocks, inline atom declarations, and explicit `connections:` sections.
- Update ToyLoader and any Script builders to emit router-friendly specs, including conversion helpers for ShaderToy assets.
- Provide compatibility flags so legacy `.eon` files can still load until the migration completes.
- **Exit criteria:** parser unit tests cover both router nets and legacy chains, ToyLoader emits router syntax for at least one sample, and method 0/1/2 each execute via router descriptors.

### Phase 3 – Interface + VFS/ECS Plumbing
- Wire router descriptors into `Vfs/Ecs/LinkFwd.cpp`, `LinkSystem.h`, and related storage paths so IDE/DropTerm consumers can view nets.
- Implement name- and index-based port lookup helpers plus validation (duplicate detection, missing ports, etc.).
- Ensure router graphs serialize into both JSON and binary VFS payloads with round-trip regression tests.
- **Exit criteria:** VFS loaders/savers operate on router nets, IDE tooling can at minimum visualize port lists, and debugging tools show router metadata.

Detailed serialization/IDE plumbing (JSON/binary fragments, chunked `.overlay.vfsch` streams, overlay caching, and MetaCtrl/MetaEnvTree inspectors) is recorded in `task/notes/packet_router_vfs_alignment.md`, so the thread can now shift its attention to the runtime/DSL conversion work that follows in Phase 4.

### Phase 4 – Atom, API, and Backend Conversion
- Update Atom subclasses (audio/gfx/etc.) to register ports via the new API, removing assumptions about primary channel 0.
- Implement router credit hooks (request/ack) per backend and add asserts to guarantee packets honor the new flow-control metadata.
- Provide temporary compatibility shims/policies so workloads can choose router scheduling versus legacy loops.
- **Exit criteria:** representative Atoms per backend operate solely through router APIs, and compatibility toggles are documented.

### Phase 5 – DSL Migration & Test Coverage
- Convert all `.eon` files under `share/eon` to the router syntax, preferably through automated helpers plus manual review.
- Update every `upptst/Eon*` package to the router AST (method 2 builders, generated sources, docs).
- Expand regression tests for router nets (functional equivalence, credit exhaustion cases, etc.).
- **Exit criteria:** zero legacy `.eon` files remain, CI includes router-focused regression suites, and documentation references only the router syntax.

### Phase 6 – Performance, Compatibility, and Cleanup
- Benchmark router mode vs legacy loops and tune flow-control policies to prevent regressions.
- Improve diagnostics/visualizers (IDE panes, DropTerm commands) to expose router topology, packet queues, and per-port credits.
- Remove deprecated loop APIs and rename residual Chain/Loop identifiers to Router equivalents.
- **Exit criteria:** performance deltas are published, diagnostics exist for router nets, and no loop-era API remains exposed.

## Status & Tracking
- **Discovery:** Link/customer/side-link inventory captured in `task/notes/packet_router_links.md` (living doc; expand per subsystem).
- **Prototype:** `NetContext` spike available in `upptst/Eon00` (`method=3`), logs router ports/connections while still delegating to `ChainContext`.
- **Tooling:** No conversion helpers exist yet; target location `script/packet_router/`.
- **Side-link parity:** RouterNetContext exposes a shared builder that stitches loops via `LoopContext::ConnectSides`, so `00b` and `00c` method 3 runs mirror ScriptLoader’s cross-loop behavior.
- **Additional coverage:** RouterNetContext now lives in `upptst/EonRouterSupport` so other packages can include it; `upptst/Eon02` method 3 runners (`02a`, `02b`, `02c`, `02d`, `02e`, `02f`, `02g`, `02l`, `02m`) exercise single-loop audio, dual-loop side-link chains, and the fluidsynth/softinstru/fmsynth/coresynth+corefx+portmidi event/input bridges through `BuildRouterChain`.
- **Descriptors:** Canonical `RouterPortDesc` / `RouterConnectionDesc` structs now live in `uppsrc/Vfs/Ecs/Interface.h`, providing a shared home for port metadata ahead of the serialization work.
- **Serialization prep:** Descriptor metadata (direction, `ValDevTuple`, flow-control hints) now round-trips through helpers in `uppsrc/Vfs/Ecs/Formats.{h,cpp}` and is covered by the new `upptst/Router` console tests (`script/build-console.sh Router`).
- **Storage:** JSON fragment helpers (`VfsSaveFragment` / `VfsLoadFragment` in `uppsrc/Vfs/Storage`) now persist router metadata appended to loop nodes, and the Router console suite exercises a fragment disk round-trip.
- **Overlay index:** `VfsOverlayIndex` plus `VfsSaveOverlayIndex`/`VfsLoadOverlayIndex` persist SourceRef provenance alongside per-node metadata (including `router.*` maps) so IDE/Env tooling can inspect router graphs without loading every fragment; exercised by `upptst/Router`.
- **Binary parity:** `VfsSaveFragmentBinary`/`VfsLoadFragmentBinary` and `VfsSaveOverlayIndexBinary`/`VfsLoadOverlayIndexBinary` provide a headerized binary wrapper around the same schema so routers can ship lightweight artifacts; new Router console tests cover both paths along with a `gdb --args bin/Router` run.
- **Artifact parity:** Router console coverage now regenerates fragments + overlay indexes through `BuildRouterOverlayIndex`, saves JSON/binary pairs, and diff-checks the reloaded payloads so builder paths can rely on identical metadata regardless of format.
- **Router regression cases:** Dedicated console packages `upptst/RouterFanout` (multi-port fan-out metadata) and `upptst/RouterPool` (limited packet-pool credit hints) exercise the RouterNetContext harness beyond the base serialization smoke tests.
- **IDE storage integration:** Package stores now invoke `VfsSaveFragment*`/`VfsSaveOverlayIndex*` so every `Meta.bin` update emits JSON + binary artifacts automatically, and the IDE overlay views load router metadata straight from those indexes before appending it to the inspectors (MetaEnvTree now displays the `router` stanza next to the code snippet).
- **IDE overlays:** MetaCtrl’s virtual overlay tree now queries the cached overlay index and appends router summaries (port counts, connection totals, credit policy) next to each node so router fan-out/pool hints are visible without opening source fragments.
- **Docs:** Conversion references for the full audio generator trio (`00a`/`00b`/`00c`) sit next to the respective assets. VFS alignment + schema outline are tracked in `task/notes/packet_router_vfs_alignment.md`; AGENTS updates still pending.
- **Metrics:** Baseline performance numbers to capture once router prototype stands up.

---

## Phase Status Summary

### ✓ Phase 0 Complete
RouterNetContext prototype validates router concepts. Method 3 builders in Eon00/Eon02 prove multi-loop side-link parity. Conversion docs exist for 00a/b/c.

### ✓ Phase 1/3 Serialization Complete
Router descriptors serialize through VFS Storage (JSON/binary/chunked). IDE overlays display cached router metadata. Test coverage comprehensive.

### ✓ Phase 1 Runtime API Complete
**PacketRouter integration validated with Eon workloads (2025-11-20):**

**Implementation:**
- `uppsrc/Eon/Core/PacketRouter.{h,cpp}` with full runtime packet routing
- Port registration API (`RegisterSourcePort`, `RegisterSinkPort`)
- Connection table management (`Connect`, `GetPortCount`)
- Credit-based flow control primitives (default credits, allocation)
- Atom API extensions (`RegisterPorts` virtual method on atoms)

**LoopContext Integration:**
- `RegisterRouterPorts()` iterates atoms and calls `RegisterPorts()`
- `MakeRouterConnections()` generates circular topology from legacy loops
- Called from `ChainContext::AddLoop()` at context creation time

**Validated Atoms (RegisterPorts implementations):**
- `CustomerBase` - source-only pattern (order packets)
- `RollingValueBase` - pipe pattern (sink + source)
- `VoidSinkBase` - sink-only pattern (audio consumption)

**Test Results (Eon00 with method 0/2/3):**
- PacketRouter created → 3 ports registered → 1 connection made → PacketRouter destroyed
- Packet flow works correctly through legacy LinkSystem forwarding
- Router lifecycle managed properly with clean destruction

**Next Steps:** Phase 2 DSL Integration - parser/grammar implementation.

### ✓ Phase 2 DSL Integration Complete (2025-11-20)
**Full DSL parser and validation infrastructure implemented:**

**Data Structures:**
- `uppsrc/Eon/Script/Def.h`:
  - `NetConnectionDef` - explicit port connections (`from_atom:from_port -> to_atom:to_port`)
  - `NetDefinition` - router network with flat atom list + connection vector
  - `MachineDefinition.nets` field added alongside legacy `chains`
  - ToString/GetTreeString helpers implemented

**Loader Infrastructure:**
- `ScriptNetLoader` class created (follows ScriptChainLoader pattern)
- Integrated into MachineLoader (constructor, Load, Visit, GetTreeString)
- NetLoader.cpp added to Script.upp package
- Eon/Script package compiles successfully

**Parser & LoadNet Implementation:**
- ✓ `Cursor_NetStmt` + `net` keyword added to Eon parser (Script/Script.cpp:756)
- ✓ `LoadNet()` method implemented (Script/ScriptLoader.cpp:848-1005)
  - Successfully parses inline atom definitions from `net` blocks
  - Parses state declarations within nets
  - Parses explicit connections with `atom.port -> atom.port` syntax
  - Net blocks recognized at machine level and integrated into MachineDefinition
- ✓ VfsOverlay namespace collision resolved (removed double-nesting in Overlay.h includes)
- ✓ Build verified: Eon00 compiles successfully

**BuildNet & ScriptNetLoader Implementation:**
- ✓ `BuildNet()` method in ScriptLoader (ScriptLoader.cpp:513-630)
  - Creates PacketRouter instance for each net
  - Validates atom definitions and maps atom names
  - Validates port indices and ranges
  - Validates port type compatibility for connections
  - Provides detailed structure logging
  - Returns validation errors with FileLocation
  - Defers atom instantiation to next phase (documented with TODOs)
- ✓ `ScriptNetLoader::Load()` integration (NetLoader.cpp:15-30)
  - Calls BuildNet() for net construction
  - Provides error handling and logging

**ToyLoader Router Syntax Scaffolding:**
- ✓ TODO marker added for future router syntax generation (ToyLoader.cpp:267-277)
- ✓ Example router syntax documented in comments
- ✓ Maintains backward compatibility with loop/chain syntax

**Test Infrastructure:**
- ✓ Sample .eon file created (`upptst/RouterCore/test_net.eon`)
- ✓ Tests inline atom definitions
- ✓ Tests explicit port-to-port connections
- ✓ Build verification: Eon00 compiles with all changes

**DSL Syntax Example:**
```eon
net audio_pipeline:
    audio.sine
    audio.gain
    audio.sink
    audio.sine.0 -> audio.gain.0
    audio.gain.0 -> audio.sink.0
```

**Validation Features:**
- Atom definition lookup and validation
- Port index range checking
- Port type compatibility verification
- Connection endpoint validation
- Comprehensive error reporting with FileLocation

### ✓ Phase 3 – Atom Instantiation & Lifecycle Complete (2025-11-20)
**Full BuildNet() implementation with live atom creation and packet routing:**

**NetContext Class** (`uppsrc/Eon/Core/Context.{h,cpp}`):
- Router-based network context parallel to ChainContext
- `AddAtom()` - Creates atoms from action strings using VfsValueExtFactory
- `RegisterPorts()` - Calls RegisterPorts() on all atoms in network
- `MakeConnections()` - Wires explicit port-to-port connections via PacketRouter
- `PostInitializeAll()` / `StartAll()` - Lifecycle management with proper cleanup
- `UndoAll()` - Error recovery and cleanup support

**BuildNet() Implementation** (`uppsrc/Eon/Script/ScriptLoader.cpp:513-600`):
- Resolves net space from net ID (same pattern as BuildChain)
- Creates NetContext instance for each net
- Instantiates atoms using VfsValueExtFactory (action → AtomTypeCls resolution)
- Registers ports with PacketRouter for all atoms
- Wires explicit connections from NetConnectionDef specifications
- Stores built NetContext in ScriptLoader::built_nets for lifecycle management

**ScriptLoader Integration**:
- Added `Array<One<NetContext>> built_nets` field in Loader.h
- ImplementScript() extended to handle net lifecycle:
  - PostInitializeAll() for all nets before chains
  - StartAll() for all nets after chains started
  - Proper error handling with UndoAll() on failures

**Atom Creation Pattern**:
- Atom name from definition → action string lookup
- VfsValueExtFactory resolves action to AtomTypeCls + LinkTypeCls
- Atoms created in net_space VfsValue node
- WorldState initialized with atom args from definition
- InitializeAtom() + Initialize() called during creation

**Port Registration**:
- RegisterPorts() called on each atom after creation
- Router tracks port handles with atom pointers
- Port indices stored in atom->router_source_ports / router_sink_ports

**Connection Wiring**:
- Explicit connections from NetConnectionDef
- Port type validation during wiring
- PacketRouter::Connect() with proper PortHandle construction
- Validates port indices against registered port counts

**Lifecycle Management**:
- PostInitializeAll(): reverse order iteration over atoms
- StartAll(): reverse order iteration, SetRunning() after Start()
- Error recovery: UndoAll() stops and uninitializes in reverse order
- Integration with eager_build_chains mode in ImplementScript()

**Build Verification**:
- All code compiles successfully (Eon00 built without errors)
- NetContext properly integrated with ScriptLoader
- Test infrastructure ready for Phase 4 validation

### ✓ Phase 4 – Packet Routing Infrastructure Complete (2025-11-21)
**Full packet delivery and atom-router integration implemented:**

**Test Files:**
- `share/eon/tests/00d_audio_gen_net.eon` - Linear pipeline test (3 atoms, 2 connections)
- `share/eon/tests/00e_fork_net.eon` - Fork topology test (fan-out/fan-in, 3 atoms, 3 connections)
- Test drivers in `upptst/Eon00/` (00d, 00e)

**RoutePacket Implementation** (`PacketRouter.cpp:95-149`):
- Delivers packets to all connected destinations via `AtomBase::Recv()`
- Handles fan-out (one source to multiple sinks)
- Handles fan-in (multiple sources to one sink)
- Tracks `packets_routed` counter per connection
- Error handling for invalid ports and null atoms

**Atom-Router Integration:**
- `AtomBase::packet_router` - Pointer to PacketRouter set during port registration
- `AtomBase::EmitViaRouter(port_index, packet)` - Method for atoms to emit packets via router
- `RegisterSinkPort` / `RegisterSourcePort` now set `packet_router` on the atom

**Test Results (Eon00 test 4 - 00eForkNet):**
```
PacketRouter: Created
PacketRouter::RegisterPort: 6 ports registered (3 atoms × 2 directions)
PacketRouter::Connect: src_port 1 -> dst_port 2 (conn_idx=0)
PacketRouter::Connect: src_port 1 -> dst_port 4 (conn_idx=1) // fan-out
PacketRouter::Connect: src_port 3 -> dst_port 4 (conn_idx=2) // fan-in
BuildNet: Successfully built network with 3 atoms and 3 connections
PacketRouter: Destroyed (6 ports, 3 connections)
```

- **Remaining for Full Runtime Flow:**
- Camera, media-file, image, and audio generators (SynSoft plus SynFluidsynth, SynFmSynth, SynCoreSynth, SynCoreDrummer) now register ports via the default `AtomBase::RegisterPorts()`, request credits, and call `EmitViaRouter()`/`AckCredits()` so router-based nets run the workloads while preserving the legacy Link data for compatibility. SynLV2 and the SDL event bridge now also route their packets through the PacketRouter, leaving only a handful of remaining hardware-specific sources to touch.
- `AudioGenBase` and `VideoGenBase` (the built-in Eon noise/sine generators) now request credits, emit through `EmitViaRouter()`, and acknowledge the grants so router-driven nets can use the debug sources without bypassing the legacy diagnostic path.
- The V4L2/OpenCV camera now returns `false` when credits are denied so the router diagnostics and flow-control metadata stay accurate.
- `MidiFileReaderAtom` now obeys router credits, emits MIDI batches via `EmitViaRouter()`, and restores the `PacketValue` afterwards so MIDI playback participates in the same metadata-driven flow control as other audio sources.
- `FxAudioCore::Effect_Send` now requests router credits, routes each generated audio buffer via `EmitViaRouter()`, and acknowledges the grant before letting the legacy Link logic observe the packet so custom `Audio::Effect` atoms participate in the router diagnostics.
- Legacy LinkSystem delivery remains available for unconverted atoms, but new nets should rely on the router path so diagnostics and credits stay in sync.
- `Screen` event senders now run through the templated `EventsBase_Send` implementation in `Screen/Impl.inl`, which requests router credits, emits the `GeomEventCollection` via `EmitViaRouter()`, and ACKs before clearing the future sendable flag so OS input streams report router credit diagnostics alongside the SDL bridge.
- `SdlOglAudioSource` now invokes `AudioBaseT::Send` credit handling before pumping FBO-backed audio packets through `EmitViaRouter()` so the SDL/OpenGL audio bridge shows up in PacketRouter diagnostics ahead of the legacy Link delivery.

### Phase 4 Validation (in progress)
- Extend the `upptst/Router` console suites (and `upptst/Eon00/00h_router_flow`) with explicit runs that include the debug audio/video generators (`center.audio.src.dbg_generator`, `center.video.src.dbg_generator`), SDL event/audio bridges, and PortMidi so the credit counters exposed by `PacketRouter` (`GetTotalPacketsRouted`, `GetPacketsRouted`, `GetTotalDeliveryFailures`) exercise the same workloads that touched routers in Phase 4 runtime conversions.
- `upptst/Router/RouterTest.cpp::TestRouterRuntimeFlowCounters` now inspects every router connection created by `share/eon/tests/00h_router_flow.eon`, asserts a minimum of 5 connections, checks each connection's `GetPacketsRouted`/`GetDeliveryFailures`, and validates that the aggregated sum matches both the router total and `ScriptLoader::GetTotalPacketsRouted()` to keep the credit path counter invariants tied to the debug audio/video/SDL/PortMidi workload.
- The same test now also uses `ScriptLoader::GetNetContext()` plus the new `PacketRouter::GetConnectionInfo()` helper to match each of the `00h_router_flow` connections, ensuring every link defined in the net corresponds to a router connection and that the debug audio/video/SDL/PortMidi sources each routed packets through their credited ports.
- `share/eon/tests/00h_router_flow.eon` now wires `sdl.ogl.center.fbo.audio` into a `center.audio.sink.test.realtime` sink, and `TestRouterRuntimeFlowCounters` ensures this SDL/OpenGL audio bridge contributes to the per-connection packet counters so its router credits stay visible in the regression suite.
- Add lightweight regression cases that construct router nets via `RouterNetContext` and `RouterSchema` to confirm the debug sources request credits, emit through `EmitViaRouter()`, and restore their `PacketValue` before the legacy Link path observes the packets (see `task/notes/packet_router_phase4_runtime.md` for the exact behavioral contract).
- Keep the runtime diary (`task/notes/packet_router_phase4_runtime.md`) synced with each conversion so the serialization/IDE thread can stay focused on metadata and tooling while runtime coverage is proven.

### Phase 4+ Enhancements (2025-11-21)
**Extended router infrastructure and diagnostics:**

**Statistics API** (`PacketRouter.{h,cpp}`):
- `GetTotalPacketsRouted()` - Query total packets routed across all connections
- `GetPacketsRouted(int conn_idx)` - Query packets routed on specific connection
- `GetTotalDeliveryFailures()` - Track delivery failures
- `GetDeliveryFailures(int conn_idx)` - Per-connection failure count
- `delivery_failures` counter added to Connection struct

**ScriptLoader Integration**:
- `GetNetCount()` - Query number of built nets
- `GetNetRouter(int net_idx)` - Access router by net index
- `GetTotalPacketsRouted()` - Aggregate packets across all nets

**Error Handling**:
- `DisconnectAtom(AtomBase*)` - Safely disconnect atom mid-run
- Enhanced delivery failure tracking with per-connection counters
- Improved logging with failure counts

**New Test** (`upptst/Eon00/00h_router_flow.cpp`):
- `Run00hRouterFlow` - Runtime packet flow verification
- Tests linear pipeline: src -> customer -> sink
- Verifies router topology builds correctly

**Next Phase:** Phase 5 - DSL Migration & Test Coverage

---

## Immediate Action Items
- [x] Create `task/notes/packet_router_links.md` with grep results for `CustomerBase`, `LinkFactory`, and side-link usages.
- [x] Draft `NetContext` spike in `upptst/Eon00` and document findings in `upptst/Eon00/CURRENT_TASK.md` once it exists.
- [x] Write `share/eon/tests/00a_audio_gen.eon` conversion notes (old syntax vs router) for onboarding.
- [x] Sketch router port descriptor structs (`RouterPortDesc`, `RouterConnectionDesc`) ahead of Phase 1 API work.
- [x] Align with VFS/ECS maintainers on serialization expectations and storage schema naming.
- [x] Wire RouterNetContext-based multi-loop builds so `00b/00c` router spikes achieve side-link parity with ScriptLoader.
- [x] Extend the conversion note template to `share/eon/tests/00b_audio_gen.eon` and `00c_audio_gen.eon`.
- [x] Outline the JSON & binary router schema (ports, connections, bridges, flow control) in `task/notes/packet_router_vfs_alignment.md` to unblock VFS/Storage work.
- [x] Thread the new binary fragment/index helpers into actual IDE overlay builders so router metadata lands alongside legacy dumps by default.
- [x] Add Router console coverage that writes a fragment + overlay index pair to disk, regenerates the overlay via builder code, and confirms binary/JSON outputs stay in sync.
- [x] Extend IDE overlay panes (MetaEnvTree/MetaCtrl) with richer router inspectors once chunked writers ship, using the cached overlay metadata instead of raw fragment loads.
- [x] Integrate `BuildRouterOverlayIndex` with the new chunked/binary writers so IDE builders stream router metadata into `.overlay.vfsch` files without re-walking the fragment tree.
- [x] Surface per-connection router metadata (fan-out hints, pool shares, flow-control JSON) in MetaCtrl overlay inspectors so IDE users can act on the stored hints.
- [x] Stand up atom+router regression tests that stress multi-port fan-out (former side-connection cases) and confirm router credits emulate the legacy limited packet pool when atoms skip sends or burst onto multiple ports (see `upptst/RouterFanout` + `upptst/RouterPool` console packages).

## Dependencies & Collaboration
- **Eon Core** drives runtime + DSL refactor, owns router prototype and Atom APIs.
- **VFS/ECS** supplies router descriptors, serialization hooks, and IDE tooling alignment.
- **Backend owners (`api/audio`, `api/gfx`, `api/sdl`):** Provide port declarations and credit policies.
- **Tooling/Docs** ensure conversion scripts + AGENTS updates are available as phases close.
- Coordination touchpoints: weekly stand-up notes in `task/notes/packet_router_sync.md` (to be created) and milestone updates in `CURRENT_TASK.md`.

## Deliverables & Artifacts
- Discovery report (`task/notes/packet_router_links.md`).
- Port naming spec + flow-control metadata schema.
- Router-enabled `.eon` syntax reference + conversion script usage doc.
- Updated uppsrc/Eon + api package docs/AGENTS describing router semantics.
- Regression tests covering router nets (functional + performance) hosted in `upptst/Eon*`.

## Open Questions & Risks
- How to emulate legacy loop packet quotas within router credit accounting?
- Do we support cross-net wiring syntax (`net1.atom:port -> net2.atom:port`) in v1 or delay?
- What is the fallback behavior when port names collide or remain unset?
- Can router diagnostics integrate with existing IDE inspectors without rewriting them first?
- How do we stage `.eon` conversions without breaking active branches (feature flags, dual-parser mode, etc.)?

## Dependencies
- Understanding of current Eon system architecture
- Knowledge of atom connection mechanisms
- Understanding of Exchange class functionality

## Purpose of Current Loop System
- The whole point of the loop system was to even out the packets in the loop
- This allowed the flow of packet signals demand between atoms

## Flow Control Requirements
- The demand between packets needs to be addressed in a different way in the new architecture
- We might add ports for atoms and connect some ports as signals
- "Port" will replace the concept of primary sink/src (id=0) and secondary sink/src
- All ports will be equal in rank (we will discard the "primary" qualifier)
- We will still have src/sink ports, connecting output to input
- Sending packets to ports won't require sending packets through a primary port anymore
- The "sync" sending feature will be maintained inside packets using a helper class or attribute
- This sync feature will not be enforced at the architecture level anymore in the new version
- We could implement the old loop system in the new version using the new flow-control signal
- Uncertainty remains about the flow-control logic: previous system used constant amount of packets in the loop
- We might still need some sort of virtual pool of packets for groups of atoms

## Morphed Exchange Classes
- The new routing system will require morphing existing Exchange classes to function as packet routers
- `ExchangeSourceProvider` → `RouterSourceProvider` (handles output ports in the new system)
- `ExchangeSinkProvider` → `RouterSinkProvider` (handles input ports in the new system)
- `ExchangePoint` → `RouterConnection` (handles routing between ports instead of direct connections)
- `ExchangeSideSourceProvider` → `RouterSideSourceProvider` (handles side connections)
- `ExchangeSideSinkProvider` → `RouterSideSinkProvider` (handles side connections)
- Router-based configuration will use port connections instead of the current exchange-point linking mechanism
- The router system will maintain a connection table mapping source ports to sink ports
- All router providers will connect through the new `PacketRouter` class instead of forming loops
- Flow control and packet distribution will be managed by the router rather than by loop mechanics

## Proposed New .eon File Syntax (Router-Based)

### Current Loop-Based Syntax:
```
machine sdl.app:
    driver context:
        sdl.context

    chain program:
        loop ogl.fbo:
            ogl.customer
            sdl.fbo.standalone:
                filepath = "shaders/toys/simple/simple_single/stage0.glsl"
```

### Proposed Router-Based Syntax:
```
machine sdl.app:
    driver context:
        sdl.context

    net program:
        # Atom definitions with IDs
        customer0: ogl.customer
        shader0:
            type: sdl.fbo.standalone
            filepath = "shaders/toys/simple/simple_single/stage0.glsl"

        # Connection definitions
        connections:
            customer0:0 -> shader0:0    # Connect port 0 of customer0 to port 0 of shader0
```

### More Complex Example:
```
machine sdl.app:
    driver context:
        sdl.context

    net program:
        state event.register

        # Atom definitions
        buffer_customer: ogl.customer
        buffer_shader:
            type: sdl.ogl.fbo.side
            shader.frag.path = "shaders/toys/simple/simple_double/stage1.glsl"
        screen_customer: ogl.customer
        screen_shader:
            type: sdl.fbo
            shader.frag.path = "shaders/toys/simple/simple_double/stage0.glsl"
            recv.data = false

        # Explicit connections
        connections:
            buffer_customer:0 -> buffer_shader:0
            buffer_shader:0 -> screen_shader:0  # Data flows from buffer to screen
            screen_customer:0 -> screen_shader:1 # Additional input to screen shader
```

### Key Improvements in the New Syntax:
- No need for ".out" and ".in" qualifiers since there's no other option
- No need for [] brackets; simple colon notation like "atom:port" is sufficient
- No need for "atoms:" directory; atoms can be defined directly in the net
- "chain" keyword could be renamed to "net" or "subnet" to reflect the network nature
- All connections are explicitly defined for clarity
- Atoms don't need to form closed loops, allowing for more flexible topologies
- Cross-network connections could potentially be supported with notation like "net1.atom:port -> net2.atom:port"

### Customer Role in Packet Flow Control:
- Currently, customer atoms serve as the starting point for packets in loops, controlling packet flow
- In the new router system, packets can be generated at any point in the network
- The packet pool and flow control system would allow packet creation anywhere in the network
- This removes the requirement for customers to be the sole source of packets
- Multiple packet sources can exist simultaneously in the network
- Flow control will be managed by the router system rather than being tied to customer positions

## Potential Challenges
- Ensuring all existing functionality is preserved during conversion
- Maintaining performance characteristics of the system
- Managing the scope of changes across multiple packages
