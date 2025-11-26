# Packet Router Phase 4 Runtime Diary

This document tracks runtime conversions and implementation details for Phase 4 of the Packet Router project.

## Phase 4 Overview
Phase 4 focused on extending router infrastructure and diagnostics, implementing runtime packet flow, and connecting various components to the new router system.

## Conversions Completed

### Atom Runtime Integration
- **Camera, media-file, image, and audio generators** (SynSoft plus SynFluidsynth, SynFmSynth, SynCoreSynth, SynCoreDrummer)
  - Now register ports via the default `AtomBase::RegisterPorts()`
  - Request credits and call `EmitViaRouter()`/`AckCredits()`
  - Router-based nets run the workloads while preserving legacy Link data for compatibility

- **SynLV2 and SDL event bridge** 
  - Now route packets through the PacketRouter

- **AudioGenBase and VideoGenBase** (built-in Eon noise/sine generators)
  - Request credits, emit through `EmitViaRouter()`, acknowledge grants
  - Router-driven nets can use debug sources without bypassing legacy diagnostic path

- **V4L2/OpenCV camera**
  - Returns `false` when credits are denied
  - Router diagnostics and flow-control metadata stay accurate

- **MidiFileReaderAtom**
  - Obeys router credits, emits MIDI batches via `EmitViaRouter()`
  - Restores `PacketValue` afterwards
  - MIDI playback participates in metadata-driven flow control

- **FxAudioCore::Effect_Send**
  - Requests router credits, routes audio buffers via `EmitViaRouter()`
  - Acknowledges grant before legacy Link logic observes packet
  - Custom `Audio::Effect` atoms participate in router diagnostics

- **Screen event senders**
  - Use templated `EventsBase_Send` implementation in `Screen/Impl.inl`
  - Request router credits, emit `GeomEventCollection` via `EmitViaRouter()`
  - ACKs before clearing future sendable flag
  - OS input streams report router credit diagnostics alongside SDL bridge

- **SdlOglAudioSource**
  - Invokes `AudioBaseT::Send` credit handling
  - Pumps FBO-backed audio packets through `EmitViaRouter()`
  - Shows up in PacketRouter diagnostics ahead of legacy Link delivery

## Key Changes to Runtime Flow

### Before (Loop-Based):
- Packet flow managed by LinkSystem in circular loops
- Fixed packet quotas per loop
- Customer atoms controlled packet generation

### After (Router-Based):
- Packet flow managed through PacketRouter
- Credit-based flow control
- Atoms can emit packets via router with `EmitViaRouter()` method
- Per-connection flow control and diagnostics

## Testing Results

### Router Runtime Flow Counters
- `upptst/Router/RouterTest.cpp::TestRouterRuntimeFlowCounters` validates:
  - Connection counts match expected values
  - Per-connection packet counters
  - Aggregate sums match router totals
  - Debug audio/video/SDL/PortMidi sources routing packets through credited ports

### Test Coverage
- `upptst/RouterFanout` - Multi-port fan-out metadata
- `upptst/RouterPool` - Limited packet-pool credit hints
- `upptst/Eon00/00h_router_flow` - Runtime packet flow verification

## Statistics API
- `GetTotalPacketsRouted()` - Query total packets routed across all connections
- `GetPacketsRouted(int conn_idx)` - Query packets routed on specific connection
- `GetTotalDeliveryFailures()` - Track delivery failures
- `GetDeliveryFailures(int conn_idx)` - Per-connection failure count

## ScriptLoader Integration
- `GetNetCount()` - Query number of built nets
- `GetNetRouter(int net_idx)` - Access router by net index
- `GetTotalPacketsRouted()` - Aggregate packets across all nets

## Error Handling
- `DisconnectAtom(AtomBase*)` - Safely disconnect atom mid-run
- Enhanced delivery failure tracking with per-connection counters
- Improved logging with failure counts

## Behavioral Contract
- Debug sources request credits
- Emit through `EmitViaRouter()` 
- Restore `PacketValue` before legacy Link path observes packets
- Maintain compatibility while using new router infrastructure

## Outstanding Items
- Complete migration of remaining hardware-specific sources
- Performance validation for new router pathway
- Integration with legacy LinkSystem for unconverted atoms