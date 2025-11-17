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
- **Docs:** Conversion references for the full audio generator trio (`00a`/`00b`/`00c`) sit next to the respective assets. VFS alignment + schema outline are tracked in `task/notes/packet_router_vfs_alignment.md`; AGENTS updates still pending.
- **Metrics:** Baseline performance numbers to capture once router prototype stands up.

## Immediate Action Items
- [x] Create `task/notes/packet_router_links.md` with grep results for `CustomerBase`, `LinkFactory`, and side-link usages.
- [x] Draft `NetContext` spike in `upptst/Eon00` and document findings in `upptst/Eon00/CURRENT_TASK.md` once it exists.
- [x] Write `share/eon/tests/00a_audio_gen.eon` conversion notes (old syntax vs router) for onboarding.
- [x] Sketch router port descriptor structs (`RouterPortDesc`, `RouterConnectionDesc`) ahead of Phase 1 API work.
- [x] Align with VFS/ECS maintainers on serialization expectations and storage schema naming.
- [x] Wire RouterNetContext-based multi-loop builds so `00b/00c` router spikes achieve side-link parity with ScriptLoader.
- [x] Extend the conversion note template to `share/eon/tests/00b_audio_gen.eon` and `00c_audio_gen.eon`.
- [x] Outline the JSON & binary router schema (ports, connections, bridges, flow control) in `task/notes/packet_router_vfs_alignment.md` to unblock VFS/Storage work.

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
