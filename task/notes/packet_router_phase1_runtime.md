# PacketRouter Phase 1 – Runtime API Implementation

## Goal
Replace the loop-based Exchange/Link runtime with a router that owns port registration, connections, and credit-aware packet delivery so atoms can emit via an explicit router API and downstream packages stop depending on `CustomerBase` queues.

## Current State
- `uppsrc/Eon/Core/PacketRouter.{h,cpp}` now drives the runtime: ports are tracked via `RouterPortDesc`, connections live in a dedicated `connection_table`, credits are granted/requested, `DisconnectAtom` can tear down endpoints, and diagnostics include topology, port status, and connection tables.
- `uppsrc/Eon/Core/Atom.h` stores per-atom `router_sink_ports`/`router_source_ports`, exposes `RegisterSinkPort`, `RegisterSourcePort`, and lets atoms call `EmitViaRouter` once a router registers their ports; `PacketRouter::PortHandle` caching keeps the router pointer on the atom for the duration of the net.
- `uppsrc/Eon/Core/Context.{h,cpp}` introduces `NetContext` (with `AddAtom`, `RegisterPorts`, `MakeConnections`, `PostInitializeAll`, `StartAll`, `ProcessFrame`) plus router-aware `LoopContext` helpers so both new nets and legacy loops can register ports and wire connections through `PacketRouter` before emitting packets.
- Runtime metadata is surfaced by `upptst/EonRouterSupport/EonRouterSupport.h` (`RouterNetContext`) and the `Vfs/Ecs/Formats.{h,cpp}` helpers, so storage/IDE tooling already records router ports/connections while editorial code (e.g., `Vfs/Storage/VfsStorage.{h,cpp}`) persists fragments and overlay indexes that include the router stanza.

## Tests & Coverage
- `upptst/RouterCore` targets the `PacketRouter` API with a minimal stub, exercising port registration, connection creation, request/ack credit bookkeeping, and topology dumps without pulling in the full Eon stack.
- `upptst/Router/RouterTest.cpp` drives `RouterNetContext` metadata end-to-end: it adds atoms/ports/connections, round-trips `RouterPortDesc`/`RouterConnectionDesc`, reloads fragments and overlays via `VfsSaveFragment`, `VfsLoadFragment`, and the binary helpers, and asserts the cached `router` stanza matches the generated schema.
- `upptst/RouterFanout` and `upptst/RouterPool` stress multi-port fan-out metadata and packet-pool hints, verifying the schema exposes burst groups, pool shares, and legacy-loop credit policies through `RouterSchema`.
- `upptst/Eon00/00h_router_flow.cpp` executes a real net built by the script loader so routers actually route packets through `PacketRouter::RoutePacket`, increment statistics, and play nice with the remaining loop-centric code.

## Status
- All Phase 1 exit criteria are satisfied: the router runtime exists, atoms register ports, connections are wired, code logs diagnostics, and several console packages validate the new topology and serialization helpers.
- With PacketRouter in place we can now focus on DSL-driven net instantiation and IDE/storage plumbing.

## Next
- See `task/notes/packet_router_phase2_dsl.md` for how the parser/AST now emit router nets, how `NetContext` is wired to `ScriptLoader::BuildNet`, and which `.eon` assets/tests exercise the new syntax.
