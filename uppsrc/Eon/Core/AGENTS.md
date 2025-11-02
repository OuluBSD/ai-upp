Eon/Core — AGENTS Guide

Scope
- Applies to the entire `uppsrc/Eon/Core` package tree.
- Audience: contributors implementing core atom/link infrastructure, ECS helpers, and AST‑free VFS builders.

Purpose
- Provide the foundational building blocks used by Eon: atoms, links, exchange points, and related helpers.
- Expose AST‑free, VFS‑first context APIs to build ECS objects (pools/entities/components) and loop/chain graphs directly.
- Never depend on Script/Ast types (keep Script confined to `Eon/Script`).

Package Map (Files + Roles)
- `Core.h` umbrella header. All `.cpp/.icpp` in this package must include this first (BLITZ policy).
- `Types.h` core type aliases and small utilities.
- `Container.{h,cpp}` misc helpers used by atoms/links.
- `Atom.h` atom base add‑ons for Eon.
- `LinkUtil.{h,cpp}`, `LinkBase.{h,cpp}`, `LinkFactory.{h,cpp}` link utilities/base and registration.
- `Base.{h,cpp}`, `BaseAudio.{h,cpp}`, `BaseVideo.{h,cpp}`, `Network.{h,cpp}` core base implementations and categories.
- `GenAtom.inl` generator helpers for atoms.
- `Context.{h,cpp}` AST‑free VFS builder API: `PoolContext`, `EntityContext`, `ComponentContext`, `LoopContext`, `ChainContext`, and `ResolveLoopPath`.

Header Include Policy (U++ BLITZ)
- Every implementation file in this package must start with `#include "Core.h"`.
- Do not include third‑party/system headers in package headers other than the main header aggregation. Keep rare includes local to the `.cpp`.
- Keep this package independent of `Eon/Script` (no `AstNode` etc.). Other packages may include `Core.h`.

Subpackage Independence
- `Eon/Core` is a separate package. Don’t include internals from sibling/parent Eon packages.
- Register types and expose minimal, stable APIs consumed by other packages (Script, Ecs, etc.).

Atoms, Links, Interfaces (Core2)
- Register atoms via `VfsValueExtFactory::RegisterAtom<T>(name)`; implement:
  - `static AtomTypeCls T::GetAtomType()`
  - `static LinkTypeCls T::GetLinkType()` and `static LinkTypeCls T::GetLinkTypeStatic()` as required
  - `static String T::GetAction()` to bind human‑readable action ids
- Register links via `VfsValueExtFactory::RegisterLink<T>(name)`.
- Interface metadata:
  - `IfaceConnTuple` describes sink/source channels and side‑linking; call `iface.Realize(atom_type)` and then populate `iface.src[i]`/`iface.sink[i]` for side connections.

Context API (AST‑free VFS Builders)
- `PoolContext`/`EntityContext`/`ComponentContext`
  - Build ECS trees directly under `Engine::GetRootPool()`.
  - Example:
    - `PoolContext world(eng.GetRootPool());`
    - `auto ppl = world.AddPool("world");`
    - `auto player = ppl.AddEntity("player");`
    - `player.AddComponent("Transform", &args);`
- `LoopContext`
  - Build a loop’s atoms/links under a loop/space node (`Engine::GetRootLoop()` parallel to `GetRootSpace()`).
  - `AddAtom(atom_type, link_type, iface, &args, idx)` creates the atom/link, runs `Initialize*`, and records it in the loop. It does not run `PostInitialize`.
  - `MakePrimaryLinks()` links atoms sequentially via exchange points (optional; used for simple chains).
  - `PostInitializeAll()/StartAll()/UndoAll()` manage lifecycle for created atoms/links. You must call these after all atoms are added and links/sides are connected.
- `ChainContext`
  - Build multiple loops and connect side‑links across them using existing `IfaceConnTuple.conn` ids.
  - `ResolveAction(action, out_atom, out_link)` maps action strings to registered atom/link classes.
  - Aggregate lifecycle: `PostInitializeAll()/StartAll()/UndoAll()`.
- `ResolveLoopPath(Engine&, parts|dotted)`
  - Resolve a loop VFS node without Script/Ast/Id types.

Design Notes
- Contexts never use `AstNode` or Script code paths. They mutate VFS directly.
- Side‑linking must follow `IfaceConnTuple` channel semantics: optional channels may remain unconnected; required channels must be satisfied.
- Primary link creation uses the exchange point registry (`IfaceLinkDataMap`) to instantiate the correct exchange component.

Debugging
- Each context class implements `GetTreeString(int indent)` for quick dumps of what was built.
- Use standard logging macros to trace created atoms/links and loop paths (see `Context.cpp`).
- Driver-backed loops: ScriptLoader now remaps non-driver loop ids under the closest driver prefix (e.g. `x11.app.context.program.video`). If a downstream atom cannot find its driver context, run `bin/Eon03 0 <method>` and look for `remapped loop path under driver` in the logs to confirm the path adjustment happened; missing logs mean the loop id needs manual fixup or a new remapping rule.

Extending Core
- New atom/link: implement and register via `VfsValueExtFactory`; add `GetAction()` to appear in action resolution.
- New context helpers: keep them AST‑free and avoid adding cross‑package dependencies.
- If you add headers, aggregate only in `Core.h` and keep other headers minimal.

Current Tasks (living list)
- Complete migration of Script loop/chain construction to use `ChainContext`/`LoopContext` (adapters are in place).
- Add small utilities for VFS path resolution (entity/pool) mirroring `ResolveLoopPath` if needed.
- Evaluate rollback improvements in contexts (fine‑grained undo for partial failures).

Lifecycle Order (important)
- Build all atoms: `LoopContext::AddAtom(...)` for each atom in the loop.
- Create primary links if desired: `LoopContext::MakePrimaryLinks()`.
- For side links across loops, use `LoopContext::ConnectSides(loopA, loopB)` after both loops are built.
- Finalize: `ChainContext::PostInitializeAll()` or `LoopContext::PostInitializeAll()`.
- Start: `ChainContext::StartAll()` or `LoopContext::StartAll()`.
- On failure during post‑init or start, call `UndoAll()` on the affected `ChainContext`/`LoopContext` to stop and uninitialize in reverse order.

Manual Usage Example
- Resolve target loop path: `VfsValue* l = ResolveLoopPath(eng, "tester.generator");`
- Prepare `ChainContext::AtomSpec` list with realized interfaces and optional args.
- Add loop: `LoopContext& loop = chain.AddLoop(*l, specs, make_primary_links=true);`
- If you have several loops, connect sides via `ConnectSides` for each pair needing side‑links.
- Call `chain.PostInitializeAll();` then `chain.StartAll();`.

Script parity
- The Script path already performs: build → connect sides → post‑initialize → start. When building via Core contexts directly, replicate this sequencing explicitly as above.

## Debugging Machine/Driver/Chain Structures

### Overview
Machine/driver/chain structures (introduced in Eon03) define hierarchical loop systems:
```
machine x11.app:
    driver context:      # Initialization/context atoms
        x11.context
    chain program:       # Processing chains
        loop video:      # Actual processing loop
            center.customer
            center.video.src.dbg_generator
            x11.video.pipe
```

### Key Architectural Points

**VFS Tree Hierarchy:**
- Driver loops create atoms in VFS tree at: `loop.{machine}.{driver}`
- Chain loops create atoms in VFS tree at: `loop.{machine}.{driver}.{chain}.{loop}`
- Example: `loop.x11.app.context` (driver) and `loop.x11.app.context.program.video` (chain loop)
- **Critical:** Driver atoms must be in VFS tree for `FindOwnerWithCast<T>()` to work
  - Video atoms traverse up VFS tree to find context atoms (e.g., X11Context)
  - `a.val.FindOwnerWithCast<X11Context>()` checks siblings at each owner level

**Forwarding Loops vs VFS Tree:**
- LinkSystem forwarding operates on **chain loops only** (the actual processing loops)
- Driver loops are **not** in the forwarding chain - they're initialization-only
- Example from working Eon03:
  - ChainContext has 2 LoopContexts: driver (1 atom) + video loop (3 atoms)
  - LinkSystem shows only "loop id=0" with video loop atoms
  - Driver atom sits in VFS tree where video atoms can find it

**is_driver Flag:**
- ScriptLoader marks driver loops with `is_driver = true` and `has_link = false`
- Driver loops should create atoms but not participate in packet forwarding
- See `ScriptLoader::LoadChain()` line 647: `bool is_driver = loop->src == Cursor_DriverStmt;`

### Common Issues & Debugging

**Issue 1: "error: could not find X11 context"**
- **Symptom:** Video pipeline fails to initialize, packets stuck
- **Root cause:** Driver atom (e.g., x11.context) not in VFS tree
- **Check:** Look for ChainContext output showing driver LoopContext
- **Debug cmd:** `bin/Eon03 0 2 | grep "LoopContext space:"`
  - Should show BOTH `loop.{machine}.{driver}` AND `loop.{machine}.{driver}.{chain}.{loop}`

- **Issue 2: Missing atoms in forwarding chain**
  - **Symptom:** `LinkSystem::ForwardLinks: loop id=0 packets: | 0__0 | 0__1 |` (only 2 atoms)
    - Expected: 3+ atoms for full video pipeline
  - **Root cause:** Atom initialization failed (usually can't find context)
  - **Check:** Look for "error:" messages before packet forwarding starts
  - **Compare:** Working method should show all atoms linked:
    ```
    Upp::CenterCustomer linked to Upp::VideoDbgSrc
    Upp::VideoDbgSrc linked to Upp::X11VideoAtomPipe
    Upp::X11VideoAtomPipe linked to Upp::CenterCustomer
    ```
- **Gotcha:** The forwarding trace prints `current_queue_size(min_queue_capacity)` for each sink/source. In steady state you may see `... | 0(10)__1(10) | ...` — that means “0 packets buffered now, min capacity is 10”. If the minimum is unexpectedly `1`, check which atom forced it during `LinkBase::LinkSideSink`.
- **Loop priming:** `center.customer` seeds the loop with `min_queue_size` packets during initialization. By default it keeps the queue at `1`. For audio loops set `.queue = 10` (or whatever depth you need) in the `.eon` script so buffers are prefilled before playback starts.

**Issue 3: ScriptLoader not creating driver loop**
- **Symptom:** AST shows DriverStmt but no driver LoopContext created
- **Root cause:** `ScriptLoader::BuildChain()` may not handle `is_driver` loops correctly
- **Check:** Verify `BuildChain()` processes BOTH driver and regular loops
  - See line 205: `bool has_link = !loop_def.is_driver;`
  - Driver loops should still call `cc->AddLoop()` but with `has_link=false`

### Debugging Commands

**Check packet flow:**
```bash
# Count packet queue lines (should be many if working)
timeout 1 bin/Eon03 0 2 | grep -c "| 0__1 | 0_"
```

**Check loop structure:**
```bash
# See all forwarding loops
timeout 1 bin/Eon03 0 2 | grep "LinkSystem::ForwardLinks: loop id=" | head -20
```

**Check VFS tree:**
```bash
# See what LoopContexts were created
timeout 1 bin/Eon03 0 2 | grep -A5 "LoopContext space:"
```

**Check atom linking:**
```bash
# Verify all atoms linked properly
timeout 1 bin/Eon03 0 2 | grep "linked to"
```

**Compare methods:**
```bash
# Method 2 (manual ChainContext - reference implementation)
timeout 1 bin/Eon03 0 2 > /tmp/m2.log 2>&1
# Method 1 (Builder API using ScriptLoader)
timeout 1 bin/Eon03 0 1 > /tmp/m1.log 2>&1
# Method 0 (direct .eon file using ScriptLoader)
timeout 1 bin/Eon03 0 0 > /tmp/m0.log 2>&1

# Compare outputs
diff /tmp/m2.log /tmp/m1.log | head -50
```

### Manual Implementation Pattern (Method 2 - Working Reference)

When manually implementing machine/driver/chain with ChainContext:

```cpp
// 1. Create driver loop path in VFS
VfsValue* driver_l = &loop_root;
VfsValue* driver_s = &space_root;
for (const String& part : Split("x11.app.context", ".")) {
    driver_l = &driver_l->GetAdd(part, 0);
    driver_s = &driver_s->GetAdd(part, 0);
}

// 2. Create driver atoms
Vector<ChainContext::AtomSpec> driver_atoms;
ChainContext::AtomSpec& driver_atom = driver_atoms.Add();
driver_atom.action = "x11.context";
// ... realize interface ...

// 3. Add driver loop (separate from video loop!)
ChainContext cc;
LoopContext& driver_loop = cc.AddLoop(*driver_l, driver_atoms, true);

// 4. Create video loop path (under driver path)
VfsValue* video_l = &loop_root;
VfsValue* video_s = &space_root;
for (const String& part : Split("x11.app.context.program.video", ".")) {
    video_l = &video_l->GetAdd(part, 0);
    video_s = &video_s->GetAdd(part, 0);
}

// 5. Create video atoms
Vector<ChainContext::AtomSpec> video_atoms;
// ... add customer, generator, pipe atoms ...

// 6. Add video loop
LoopContext& video_loop = cc.AddLoop(*video_l, video_atoms, true);

// 7. Initialize and start
cc.PostInitializeAll();
cc.StartAll();
```

**Key Points:**
- Driver and video are **separate AddLoop() calls**
- Driver path: `x11.app.context`
- Video path: `x11.app.context.program.video` (nested under driver)
- This creates correct VFS hierarchy for FindOwnerWithCast to work
