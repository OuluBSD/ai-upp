# Packet Router – Link & Customer Inventory

This note captures where loop-era constructs (Customers, Link factories, and side-link plumbing) live today. It is the seed list for Phase 0 discovery so we know exactly what needs to be untangled before dropping routers in.

## CustomerBase Touch Points
- **Core implementation:** `uppsrc/Eon/Core/Base.h` and `Base.cpp` define `CustomerBase`, provide loop-queue helpers (`EnsureAudioDefaultQueue`, `Recv/Send`, `ForwardPacket`), and expose the lifecycle hooks that all customer atoms inherit.
- **ChainContext usage:** `uppsrc/Eon/Core/Context.cpp:414` dynamically casts atoms to `CustomerBase` during `ChainContext::PostInitializeAll()` to apply queue sizing rules per loop.
- **Script guidance:** `uppsrc/Eon/Core/AGENTS.md:157` documents loop priming behavior and the default queue size bump performed for audio customers.
- **Atom catalog:** `uppsrc/Eon/Lib/GeneratedMinimal.h/.cpp` plus `uppsrc/EonApiEditor/Headers.cpp` register every built-in customer (`CenterCustomer`, `DxCustomer`, `OglCustomer`, etc.) via `CustomerBase`.
- **Misc references:** `roadmap/PacketRouter.md` and `task/PacketRouter.md` already call out that `CustomerBase` needs to be removed/replaced; this doc provides the concrete source files.

## LinkFactory & Link Stack
- **Factory declaration:** `uppsrc/Eon/Core/LinkFactory.h/.cpp` (included by `Core.h` and listed in `Core.upp`) host the registration macros for every loop Link type.
- **AGENTS pointer:** `uppsrc/Eon/Core/AGENTS.md:17` names `LinkFactory` as one of the key headers that bind link utilities/base classes together.
- **Roadmap references:** `roadmap/PacketRouter.md` (Context + Impact sections) treats `LinkBase/LinkFactory` as the targets for router-aware replacements; this ensures the roadmap lines up with actual source files.

## Side-Link Plumbing
- **Script layer:** `uppsrc/Eon/Script/Loader.h` defines `ScriptLoopLoader::SideLink`/`AtomSideLinks`, and `LoopLoader.cpp` populates `src_side_conns`/`sink_side_conns` plus the helpers `SetSideSourceConnected` / `SetSideSinkConnected`. `ScriptLoader.cpp:542` validates side-links before finishing loop construction.
- **Runtime validation:** `uppsrc/Eon/Core/Context.cpp:232` (`LoopContext::ValidateSideLinks`) and `:442` (`ChainContext::ValidateSideLinks`) enforce the script-level expectations once Atoms/Links exist.
- **Exchange/Link glue:** `uppsrc/Vfs/Ecs/Exchange.{h,cpp}` defines `ExchangeSideSourceProvider` / `ExchangeSideSinkProvider` and the associated link helpers. `uppsrc/Vfs/Ecs/Link.{h,cpp}` exposes `LinkSideSink`, `IsAllSideSourcesFull`, etc., and `LinkFwd.cpp` contains helper logic for side-buffer fullness.
- **API usage:** Side-channel data is consumed across backends (e.g., `uppsrc/api/Graphics/FboBase.cpp`, `ImageBase.cpp`, `ProgBase.cpp`, `api/MidiFile/Midi.cpp`, `api/Effect/AudioCore.cpp`) by iterating `link->SideSinks()` / `SideSources()`.
- **Verifier:** `uppsrc/Core/EcsEngine/Verifier.cpp` inspects `ScriptLoopLoader::SideLink` data in ECS pipelines, so router replacements must keep this verifier happy.

## Next Steps
- Use this inventory to drive `rg`/`ide` deep dives when defining router port descriptors.
- Once `NetContext` spike exists, revisit these sections to decide which files need shims vs. full rewrites during Phase 1.
