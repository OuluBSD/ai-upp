# PacketRouter Phase 4 – Runtime Atom Conversions

## Goal
Gradually rewire backend atoms so they emit packets through the router instead of the legacy Link/Exchange paths. This builds on the DSL/net work from earlier phases and ensures `NetContext::ProcessFrame()` can drive actual workloads without falling back to old loops.

## Progress
- `CustomerBase::Send()` now populates its packet via `ForwardPacket()` and calls `AtomBase::EmitViaRouter()` when a `PacketRouter` is registered. Legacy loops remain unaffected because the router path is guarded by `packet_router` being present.
- The new code keeps the `PacketValue` content intact for downstream consumers by moving it into a temporary `Packet`, routing it, and then restoring it so the existing LinkSystem logic still observes the same `out` data.
- Camera, media-file, image, and SynSoft audio generators now register ports via the default `AtomBase::RegisterPorts`, request credits before emitting, and call `EmitViaRouter()`/`AckCredits()` so router-based nets drive the actual workloads alongside the legacy Link path.

## Next
- Continue migrating remaining backend sources (SDL/video/audio bridges, synth drivers, etc.) so every emission honors router credits, and keep an eye on diagnostics to make sure VFS metadata still reflects the live topology.
