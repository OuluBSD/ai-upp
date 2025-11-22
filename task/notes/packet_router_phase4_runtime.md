# PacketRouter Phase 4 – Runtime Atom Conversions

## Goal
Gradually rewire backend atoms so they emit packets through the router instead of the legacy Link/Exchange paths. This builds on the DSL/net work from earlier phases and ensures `NetContext::ProcessFrame()` can drive actual workloads without falling back to old loops.

## Progress
- `CustomerBase::Send()` now populates its packet via `ForwardPacket()` and calls `AtomBase::EmitViaRouter()` when a `PacketRouter` is registered. Legacy loops remain unaffected because the router path is guarded by `packet_router` being present.
- The new code keeps the `PacketValue` content intact for downstream consumers by moving it into a temporary `Packet`, routing it, and then restoring it so the existing LinkSystem logic still observes the same `out` data.

## Next
- Update other source atoms (e.g., camera/audio/image generators) to register their ports explicitly and emit via `EmitViaRouter()` when available.
- Teach router credit hooks to those atoms so they respect the per-port flow-control metadata captured by `Vfs/Ecs/Formats`.
