# PacketRouter Phase 2 – DSL Integration

## Goal
Migrate the parser/AST from loop-based `chain`/`loop` blocks to explicit `net` definitions so every atom declares ports by name, connections are enumerated with `atom:port -> atom:port`, and loader code can instantiate router graphs directly.

## Current State
- `NetDefinition`, `NetConnectionDef`, and `MachineDefinition::nets` live in `uppsrc/Eon/Script/Def.h`, and the AST now produces `Cursor_NetStmt` nodes via `uppsrc/Vfs/AstBuilder.cpp`/`uppsrc/Vfs/Ast.cpp` so `net` blocks enter the same pipeline as chains.
- `ScriptLoader::LoadMachine` (see `uppsrc/Eon/Script/ScriptLoader.cpp`) recognizes `Cursor_NetStmt`, resolves the dotted net id, and calls `LoadNet`, which parses inline atoms, optional states, and textual `atom.port -> atom.port` (or `atom:port`) expressions to populate `Eon::NetDefinition`.
- `ScriptLoader::BuildNet` creates a `NetContext` (`uppsrc/Eon/Core/Context.{h,cpp}`), instantiates atoms via `VfsValueExtFactory`, registers ports, wires `NetConnectionDef` records through `PacketRouter::Connect`, and pushes the resulting `NetContext` into `Loader::built_nets` so lifecycle steps (post-initialize/start) run inside `ScriptLoader::ImplementScript` just like chains.
- `ScriptNetLoader` (`uppsrc/Eon/Script/NetLoader.cpp`) is the new loader subclass that delegates to `ScriptLoader::BuildNet`; it currently skips state-targeted loaders (todo) but already logs atoms/connections and reports success/failure at load time.
- The new `.eon` assets under `share/eon/tests` illustrate router nets: `00d_audio_gen_net.eon`, `00e_fork_net.eon`, `00f_diamond_net.eon`, `00g_branch_net.eon`, `00h_router_flow.eon`, and the DSL conversion notes next to `00a/00b/00c` show old vs. new syntax.
- Test drivers under `upptst/Eon00` now include `00d_audio_gen_net.cpp` through `00i_router_perf.cpp`, each calling into the router stacks to validate the same pipeline topologies expressed in the `.eon` assets.

## Tests & Coverage
- `share/eon/tests/{00d,00e,00f,00g,00h}_*.eon` exercise linear, fork, diamond, branched, and runtime-inspection nets using the new syntax with explicit `connections` tables.
- `upptst/Eon00/00d_audio_gen_net.cpp`–`00h_router_flow.cpp` build the same nets via the loader and confirm `NetContext` registers ports, wires the router, and emits packets (00h also exercises `PacketRouter::RoutePacket`).
- `upptst/Eon00/00i_router_perf.cpp` collects performance counters by calling `NetContext::ProcessFrame`, ensuring the DSL-built nets can run repeated packet frames.
- `share/eon/tests/00a_audio_gen_CONVERSION.md` plus the sibling `CONVERSION.md` files for 00b/00c document how to rewrite legacy `.eon` files to routers, so future assets can follow the same pattern.

## Status
- Parser, AST, loader, and runtime wiring for `net` blocks are complete: the loader stores built routers (`built_nets`) and handles post-initialize/start lifecycles, `NetContext` knows how to connect atoms via port indices, and the script machine now exposes router stats (via `ScriptLoader::GetNetCount`/`GetNetRouter`).
- Connection parsing still relies on textual extraction from `Cursor_ExprStmt` because the AST currently emits raw strings for `atom:port -> atom:port`; this parsable rule should stay stable while the grammar continues to evolve.

## Next
- Phase 3 (VFS/ECS plumbing) can iterate on the metadata now that nets are fully parsed; see `task/notes/packet_router_vfs_alignment.md` for serialization/IDE integration requirements.
