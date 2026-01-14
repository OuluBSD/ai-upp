# Black Screen Investigation (Eon03/Eon07, net router)

## Context
- Tests: `bin/Eon03 4 0` (soft renderer), `bin/Eon07 1 0` (similar path), running on the converted net-based eon.
- Eon script converted to router net: `share/eon/tests/03e_x11_video_sw3d.eon`.
- Build via `script/build.py -j14 --conf-debug -v Eon03` (and similar for Eon07).

## Key Code Changes During Session
1) **Net + State handling**
   - Added state pre-creation when building nets (same pattern as chains) so `state event.register` exists in VFS before atom init.
   - Added eager-build support for nets: when `SetEagerChainBuild(true)` is used (as in Run03e), `LoadNet` calls `BuildNet` immediately and clears stored defs to avoid double instantiation.
   - Files: `uppsrc/Eon/Script/ScriptLoader.cpp` (BuildNet now pre-creates EnvState; LoadNet now eager-builds nets when eager mode is on).

2) **NetContext link creation + LinkSystem cfg**
   - NetContext now creates the link object too (matching the atom’s link type), initializes it, stores it, and provides a fallback `RealtimeSourceConfig` so LinkSystem doesn’t hit null `last_cfg`.
   - Link lifecycle mirrors atoms: PostInitialize/Start/Undo now process links as well.
   - Added `link` and optional `cfg` to `NetContext::AtomInstance` and set `link->last_cfg = &cfg`.
   - Files: `uppsrc/Eon/Core/Context.{h,cpp}`, `uppsrc/Vfs/Ecs/Link.h`.

3) **Avg-color instrumentation via WS args**
   - X11Sw sink now reads `.avg_color_log` (bool) and `.avg_color_interval` (int) from WorldState to control logging.
   - Wiring added to `03e_x11_video_sw3d.eon` (`avg_color_log = true`, `avg_color_interval = 16`).
   - Files: `uppsrc/api/Screen/X11Sw.cpp`, `share/eon/tests/03e_x11_video_sw3d.eon`.

## Current Behavior
- Build succeeds after above changes (Eon03). Eon07 not re-run post-patch.
- Running `bin/Eon03 4 0` now eagerly builds the router net. PacketRouter registers ports and routes packets. Massive packet spam from FboProg → video.pipe but still black screen; no avg-color logs observed because `GFXLOG` is compiled out unless `flagDEBUG_GFX` is set.
- Run ends with heap leak panic (U++ debug heap) after PacketRouter destruction; likely due to router atoms/links not being fully cleaned or new fallback cfg allocation. No hard crash in LinkSystem anymore.

## Key Logs / Clues
- Previously crashed in LinkSystem::ForwardLinks (GetConfig null); fixed via fallback cfg.
- PacketRouter logs now show connections (7 ports, 3 connections) and large routed packet counts.
- No `GFXLOG` output for avg color; likely compiled out (flagDEBUG_GFX not defined). Also suggests X11Sw::Finalize may not be reached or sink buffer remains empty.
- Heap leak dump at shutdown (multiple `FreeFreeFree` blocks) triggers Panic.

## Open Issues
1) **Black screen persists**
   - Need to verify X11Sw sink Finalize gets called and receives non-empty accel_buf. Consider forcing a test pattern.
   - Avg-color logging is not emitted because GFXLOG is stripped; either enable flagDEBUG_GFX or temporarily switch logging to `LOG`.

2) **Heap leak panic on exit**
   - Router net teardown likely leaves allocations flagged by U++ heapdbg. NetContext::UndoAll currently stops/uninitializes atoms and links; need to audit PacketRouter and link/atom ownership to suppress leak or call MemoryIgnoreLeaks around router teardown.

3) **FboProg rendering pipeline**
   - We previously added initialization to FboProg (bf.Initialize / ImageInitialize). Still may not render or Finalize outputs.
   - Packet spam indicates processing loop doesn’t exit; might be stuck in customer/fbo cycle without producing frames.

4) **Segfault in XFlush on exit (seen earlier in X11Sw::SinkDevice_Uninitialize)**
   - Not rechecked after current changes. Ensure ctx.win/display validity before XFlush/XCloseDisplay; already had some guards.

## Suggested Next Steps
- Enable debug gfx logging: compile with `flagDEBUG_GFX` or temporarily change `GFXLOG` in X11Sw.cpp to `LOG` to see avg colors and whether Finalize runs.
- Add a one-shot test pattern in X11Sw::SinkDevice_Finalize when no packets received, to confirm window path works.
- Investigate FboProg → video.pipe link: ensure video.pipe link/atom implements ProcessPackets and sets last_cfg; check that center.customer actually feeds data.
- Track leak: wrap NetContext teardown or router destruction with `MemoryIgnoreLeaksBegin/End` temporarily, or audit NetContext::UndoAll/PacketRouter ownership to free properly.
- Rerun `bin/Eon07 1 0` after fixes to see if the same pipeline works.

## Reference Commands
- Build: `script/build.py -j14 --conf-debug -v Eon03`
- Run (soft): `bin/Eon03 4 0`
- Run (Eon07 soft): `bin/Eon07 1 0`

## Files Touched
- `uppsrc/Eon/Script/ScriptLoader.cpp`
- `uppsrc/Eon/Core/Context.h`
- `uppsrc/Eon/Core/Context.cpp`
- `uppsrc/Vfs/Ecs/Link.h`
- `uppsrc/api/Screen/X11Sw.cpp`
- `share/eon/tests/03e_x11_video_sw3d.eon`

