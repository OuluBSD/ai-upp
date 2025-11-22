# Packet Router – VFS/ECS Serialization Alignment

Context Links
- `uppsrc/Vfs/Ecs/AGENTS.md`: runtime owns Atom/Link registration via `VfsValueExtFactory`; every new descriptor must enter through the Factory layer.
- `uppsrc/Vfs/Storage/AGENTS.md`: storage package documents on-disk formats and should stay header-only until new schema lands.

## Phase 3 Alignment Status

### Descriptor placement
- Router port/connection structs now sit beside the Interface types in `uppsrc/Vfs/Ecs/Interface.h`. `AtomBase::SetInterface` uses `BuildRouterPortList` to produce `RouterPortDesc` entries (direction, index, optional flag, VD tuple, metadata) and `InterfaceBase::GetRouterPorts` exposes those descriptors to IDE/runtime consumers. `RouterNetContext::StoreRouterMetadata` (see `upptst/EonRouterSupport/EonRouterSupport.h`) serializes `StoreRouterSchema(BuildRouterSchema())` into the `router` ValueMap attached to every loop/net `VfsValue`, so each fragment carries the same port/connection metadata as the runtime observes.

### Storage schema
- `uppsrc/Vfs/Storage/VfsStorage.{h,cpp}` now implement `VfsSaveFragment`, `VfsSaveFragmentBinary`, `VfsSaveOverlayIndex`, `VfsSaveOverlayIndexBinary`, and `VfsSaveOverlayIndexChunked`. `EmitPackageStorageArtifacts` (called from `uppsrc/ide/Vfs/Vfs.cpp::VfsSrcPkg::Store`) serializes `.fragment.json`, `.fragment.vfsbin`, `.overlay.json`, `.overlay.vfsbin`, and the streaming `.overlay.vfsch` by running `BuildRouterOverlayIndex` with an `OverlayIndexCollectorSink` and, when available, an `OverlayIndexChunkWriter`. The chunked writer writes each `OverlayNodeRecord`'s path, source refs, and `metadata["router"]`, so a single traversal covers the JSON/binary representations and the incremental chunk without touching the fragment tree twice. Overlay readers prefer the chunked index (`VfsLoadOverlayIndexChunked`) and transparently fall back to binary or JSON when the chunked file is missing or invalid, allowing inspectors to stay responsive even while legacy payloads still exist.

### Compatibility strategy
- Legacy fragments retain their original layout because router metadata is added as a `router` record next to the existing node data instead of replacing it. `IdeMetaEnvironment::LoadOverlayIndexForPkg` caches the generated overlay index per package and resolves `router` metadata by first checking the chunked cache, then the binary envelope, and finally the JSON dump. `MetaEnvTree::DataFocusSelection` and `MetaCtrl` consume that cached metadata (`IdeMetaEnv().GetRouterMetadataForNode` → `RouterSummaryForVirtualNode`/`RouterDetailsForVirtualNode`) so the IDE can display router summaries and per-connection hints without reloading fragments, enabling slow legacy workloads to continue while the router migration progresses.

### Round-trip helpers + tests
- `StoreRouterPortDesc`, `LoadRouterPortDesc`, `StoreRouterConnectionDesc`, `LoadRouterConnectionDesc`, and `StoreRouterSchema`/`LoadRouterSchema` in `uppsrc/Vfs/Ecs/Formats.{h,cpp}` keep the JSON ↔ binary conversions coherent. `upptst/Router/RouterTest.cpp` exercises those helpers while emitting fragments, overlay indexes, and chunked `.overlay.vfsch` files, asserting that every payload holds the same `router` metadata, and `upptst/RouterFanout` / `upptst/RouterPool` extend the cases to multi-port fan‑out and packet-pool hints. `BuildRouterOverlayIndex` (combined with `OverlayIndexChunkWriter`) collects metadata for every node so `MetaCtrl::RouterSummaryForVirtualNode` and `RouterDetailsForVirtualNode` can render port counts, flow-control policies, and per-connection JSON without reopening fragments.

## Schema Outline (Live)
```json
{
  "router": {
    "ports": [
      {
        "atom": "generator0",
        "name": "audio.out",
        "id": 0,
        "dir": "src",
        "vd": "ValCls::AUDIO",
        "meta": { "credits": 1, "sync": "none" }
      }
    ],
    "connections": [
      {
        "from": ["generator0", 0],
        "to": ["sink0", 0],
        "policy": "default"
      }
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
- `ports` flatten sink/source entries, `dir` mirrors `RouterPortDesc::Direction`, and `vd` mirrors a serialized `ValDevTuple`. `connections` captures intra-net edges while `bridges` collect cross-net hops; both collapse into the router adjacency table once the runtime ignores loop shells. `flow_control` advertises the credit policy that allows routers to emulate the constant-packet loops until every consumer migrates.

### Binary view (Vfs/Storage payload)
- `VfsSaveFragmentBinary`/`VfsLoadFragmentBinary` wrap the JSON fragment in a headerized envelope, and `VfsSaveOverlayIndexBinary`/`VfsLoadOverlayIndexBinary` do the same for the overlay schema so CLI tools can ship compact `.vfsbin` artifacts. `OverlayIndexChunkWriter` writes `.overlay.vfsch` records as `<path, sources, metadata>` tuples; the metadata map contains the same `router` ValueMap so chunk loaders can rebuild `VfsOverlayIndex` instances without parsing JSON.

## Next steps
- Router serialization/IDE plumbing is complete and summarized here; the remaining work shifts to Phase 4 (runtime/Atom conversion) and Phase 5 (DSL migration + test coverage). Use `task/PacketRouter.md` to track the upcoming runtime API updates, port registration polish, and backend/DSL migrations that still need completion.
