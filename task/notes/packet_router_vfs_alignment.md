# Packet Router – VFS/ECS Serialization Alignment

Context Links
- `uppsrc/Vfs/Ecs/AGENTS.md`: runtime owns Atom/Link registration via `VfsValueExtFactory`; every new descriptor must enter through the Factory layer.
- `uppsrc/Vfs/Storage/AGENTS.md`: storage package documents on-disk formats and should stay header-only until new schema lands.

Alignment Notes
1. **Descriptor placement**
   - Router port/connection structs will live beside today’s `Interface` types inside `uppsrc/Vfs/Ecs`.
   - Each `InterfaceSink/Source` gains a port descriptor list (`name`, `direction`, `vd tuple`, metadata for credits/sync). These descriptors serialize via VFS values under the loop/net nodes.
2. **Storage schema**
   - `Vfs/Storage` exposes the JSON/binary schema. New keys:
     - `ports`: array of `{id, direction, vd, metadata}` entries.
     - `connections`: array of `{from_atom, from_port, to_atom, to_port}` for router nets.
     - `flow_control`: object describing credit pools, optional legacy loop emulation flags.
   - Schema doc lives next to implementation in `VfsStorage.h` once code lands.
   - JSON fragment helpers (`VfsSaveFragment`/`VfsLoadFragment`) now live in `Vfs/Storage/VfsStorage.{h,cpp}` so router metadata stamped onto loop nodes persists to disk immediately.
3. **Compatibility strategy**
   - Legacy loop payloads keep their existing layout. Loader emits synthetic router descriptors at load time so IDE tooling can visualize nets even before `.eon` migration finishes.
   - Storage writers output both representations while the migration is in flight (guarded by feature flag).
4. **Next actions**
   - Define concrete structs in `uppsrc/Vfs/Ecs/Interface.h` (Phase 1 deliverable).
   - Draft schema snippet in `task/PacketRouter.md` once prototype data is available.
   - Coordinate with IDE team to ensure new keys are reflected in inspectors before enabling router serialization by default.
5. **Round-trip helpers + tests**
   - `StoreValDevTuple/StoreRouterPortDesc/StoreRouterConnectionDesc` live in `uppsrc/Vfs/Ecs/Formats.{h,cpp}` with matching loaders so JSON writers and binary packers share a single conversion path.
   - `upptst/Router` exercises the helpers plus the RouterNetContext metadata wiring to make sure vd + credit flags remain stable.

## Schema Outline (Draft)

### JSON view (human-facing)
```json
{
  "router": {
    "ports": [
      {
        "atom": "generator0",
        "id": 0,
        "name": "audio.out",
        "dir": "src",
        "vd": "ValCls::AUDIO",
        "meta": { "credits": 1, "sync": "none" }
      }
    ],
    "connections": [
      { "from": ["generator0", 0], "to": ["sink0", 0], "policy": "default" }
    ],
    "bridges": [
      { "from": ["output.side_src0", 0], "to": ["input.side_sink0", 0] }
    ],
    "flow_control": {
      "policy": "legacy-loop",
      "credits_per_port": 1,
      "burst_limit": 4
    }
  }
}
```
- `ports` flatten across sink/source roles; `dir` distinguishes them, `vd` mirrors `ValDevTuple`.
- `connections` capture intra-net edges, `bridges` capture cross-net edges (both collapse into the same router adjacency list once the runtime ignores loop shells).
- `flow_control` advertises policy knobs so routers can emulate the constant-packet loops while tests migrate.

### Binary view (Vfs/Storage payload)
- Introduce a `RouterPorts` chunk under each loop VFS node. Layout: `uint16 count`, followed by `count` entries of `{uint16 atom_idx, uint16 port_idx, uint8 dir, uint8 vd_class, uint8 vd_device, VarBytes name, VarBytes meta}`.
- `RouterConnections` chunk mirrors this with `{uint16 from_atom, uint16 from_port, uint16 to_atom, uint16 to_port, uint8 flags}`.
- `RouterFlowControl` chunk encodes policy enums and integers (credits, burst limit, optional timer ticks). Defaults match current loop behavior so omission means “legacy”.
- JSON exporters simply dump the same structures with friendly keys; binary readers/writers live in `uppsrc/Vfs/Storage/Serializer.{h,cpp}` next to today’s Link serialization.
