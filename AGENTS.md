# AGENTS - General AI Agent Guide

**For**: All AI agents working with this codebase
**See also**:
- **[CLAUDE.md](CLAUDE.md)** - Advanced guidance for Claude (Anthropic)
- **[QWEN.md](QWEN.md)** - Simple, repetitive guidance for Qwen AI

---

## AI Agent-Specific Guides

### Claude (Anthropic)
Claude excels at complex reasoning and architectural analysis. See **[CLAUDE.md](CLAUDE.md)** for:
- Advanced reasoning patterns
- Complex refactoring guidance
- Architectural decision-making tips
- Cross-reference to active threads

### Qwen AI
Qwen benefits from simple, repetitive instructions. See **[QWEN.md](QWEN.md)** for:
- Step-by-step procedures
- Repeated critical rules
- Simple task checklists
- Basic workflow guidance

---

## Important Flags Convention

When working with the Ultimate++ codebase, pay special attention to the `flagV1` preprocessor flag:

- **`flagV1` code represents the original Ultimate++ code only**
- Our custom additions and enhancements are implemented as non-V1 or V2+ features
- When merging from upstream Ultimate++, code within `#ifndef flagV1` blocks represents our custom additions
- Code within `#ifdef flagV1` or `#ifdef flagV1 ... #else ... #endif` blocks represents the original Ultimate++ code
- Preserve this distinction when resolving merge conflicts to maintain our custom functionality

## Current Implementation Status

- The stdsrc implementation for Core, CtrlCore, CtrlLib, and Draw packages is currently incomplete
- Package files (`.upp`) now exist for stdsrc/CtrlLib, stdsrc/CtrlCore, and stdsrc/Draw
- These components still need to be properly implemented to provide STL-backed equivalents of the U++ functionality

## Read These First
- **[CODESTYLE.md](CODESTYLE.md)**: coding conventions and design tenets.
- **[THREAD_DEPENDENCIES.md](THREAD_DEPENDENCIES.md)**: thread dependencies and priorities.
- **[task/](task/)**: active development threads (replaces old TASKS.md).
- **[HIERARCHY.md](HIERARCHY.md)**: overview of important folders.
- **`agents/`**: helper code and examples used by AGENTS guides.
- **`stdsrc/AGENTS.md`**: STL-backed Core (U++-compatible) for agents and tests.

Deep Dives
- TheIDE (IDE application and modules): see `uppsrc/ide/AGENTS.md`. Each subpackage under `uppsrc/ide/*` also has its own AGENTS with extension points and file maps.
- AI (Core + UI framework): see `uppsrc/AI/AGENTS.md` for the package map and how it integrates with TheIDE.
- Geometry + Vision + Sound: `uppsrc/Geometry/AGENTS.md`, `uppsrc/ComputerVision/AGENTS.md`, `uppsrc/Sound/AGENTS.md`, `uppsrc/SoundCtrl/AGENTS.md`.
- Forms: `uppsrc/Form/AGENTS.md`, `uppsrc/FormEditor/AGENTS.md`.
- Meta tools: `uppsrc/MetaCtrl/AGENTS.md`, `uppsrc/Vfs/AGENTS.md`.
- Developer console: `uppsrc/DropTerm/AGENTS.md`.
- Eon (ECS + Dataflow + DSL): see `uppsrc/Eon/AGENTS.md` for a deep dive into:
  - The script DSL (`machine`, `ecs`, `loop`, `driver`, `state`) and examples from `obsolete/share/eon/tests`.
  - Atoms/Links/Loops, side links, queue sizing, and how the loader materializes graphs.
  - Extending with new Systems, Components, Atoms, and Links (registration patterns included).

Conventions For Packages
- Place an `AGENTS.md` in every package directory; it applies to that directory tree. Nested AGENTS override parents.
- In `.upp` manifests, list `AGENTS.md` as the first entry in the `file` section for quick discoverability. All relevant `.upp` files in `uppsrc` have been updated accordingly.

Current Task Files (`CURRENT_TASK.md`)
- Any directory or package may contain a `CURRENT_TASK.md`. Treat it as the authoritative, living note for what is being worked on right now in that scope.
- Before starting changes in that scope, read `CURRENT_TASK.md` and align the plan; after completing changes, update it to reflect what was done and what’s next.
- If `CURRENT_TASK.md` resides in a package (a directory with a `.upp` manifest), add it to the package’s `file` list:
  - Preferably first; if `AGENTS.md` exists, list `CURRENT_TASK.md` immediately after it.
- Rationale: we keep tasks in the working tree so they’re visible in TheIDE and play nicely with AI/developer tools.

## Build & Sandbox Policy

- Build entrypoints live in `script/`. We do **not** maintain repo-level `Makefile` or `umkMakefile`; use our custom U++ make utility `uppsrc/umk` via the helper scripts (e.g., `script/build_ide_console.sh` for TheIDE console builds).
- Repository build scripts (e.g., those under `script/`) assume full filesystem access. Running them inside a sandboxed environment (read-only cache paths) causes permission failures in `~/.cache/upp.out`.
- AI agents must detect sandboxed execution before invoking `script/build_*.sh`. If sandboxing is active (no write access to `~/.cache`), halt and report instead of attempting the build.
- **Windows Environment**: In Windows environments, `busybox` might be available and should be preferred for shell-like operations where standard Windows commands (cmd/PowerShell) might behave unexpectedly or when Unix-like behavior is needed (e.g., `busybox sh`, `busybox base64`).

## UWP Development

TheIDE now fully supports building, deploying, and debugging Universal Windows Platform (UWP) applications.

- **Builder**: Use `UWP_INTERNAL` builder in `umk` or TheIDE.
- **Auto-Registration**: The debugger (`LaunchUwpApp`) automatically registers the package from the build output directory (`AppxLayout`) before launching, ensuring you debug the latest build.
- **Debugging**: The `Pdb` debugger supports attaching to UWP processes via `IApplicationActivationManager` (launch) and `IPackageDebugSettings` (suspend-on-launch).
- **Diagnostics**: If debugging fails, check the Exclamation popups for details (e.g., missing manifest, invalid PFN). Ensure developer mode is enabled in Windows Settings.

## Memory Leak Detection & Valgrind

When the debug build reports heap leaks (e.g., "PANIC: Heap leaks detected!"), you **must** validate with Valgrind to confirm real leaks vs false positives:

### Step-by-step procedure:

1. **Check available configurations**:
   ```bash
   script/build.py --list-conf upptst/Eon03
   ```

2. **Look for a "Valgrind" or "Release (Valgrind)" configuration**:
   - If it exists, use it: `script/build.py -mc <num> -j12 upptst/Eon03`
   - Then run under valgrind: `valgrind --leak-check=full bin/Eon03 <args>`

3. **If no Valgrind config exists, create one**:
   - Edit the package's `.upp` file (e.g., `upptst/Eon03/Eon03.upp`)
   - Add a new mainconfig entry with **USEMALLOC** flag:
     ```
     mainconfig
         "Release" = ".AI .SCREEN .AUDIO ...",
         "Debug" = ".AI .SCREEN .AUDIO ... .DEBUG_RT",
         "Release (Valgrind)" = "USEMALLOC .AI .SCREEN .AUDIO ...";
     ```
   - **USEMALLOC is critical** - it disables U++'s custom allocator and uses system malloc, which valgrind can track
   - Then build: `script/build.py -mc <valgrind-num> -j12 upptst/Eon03`

4. **Run under valgrind**:
   ```bash
   valgrind --leak-check=full --show-reachable=yes --show-leak-kinds=all \
            --track-origins=yes bin/Eon03 <test-args>
   ```

5. **Interpret results**:
   - "Definitely lost" = real leaks that must be fixed
   - "Possibly lost" = often false positives from C++ containers/strings
   - "Still reachable" = intentional static allocations (normal for globals)
   - Debug builds may show false positives; always use Valgrind for confirmation

**Example**: See `upptst/Eon03/Eon03.upp` line 26 for the Valgrind configuration pattern.

**Why this matters**: Debug heap checker shows internal "Free" markers as leaks, but these are allocator artifacts. Only Valgrind can distinguish real leaks from allocator bookkeeping.


Header Include Policy (U++ BLITZ)
- Source files (`.cpp`, `.icpp`, `.c`) in any package (with a `.upp` file) must include only the package's main header first, using a relative include: `#include "PackageName.h"`.
- Rationale: we use U++'s BLITZ (custom header precompilation). It is intentionally simple; including only the main header first keeps BLITZ effective and build times stable.
- After the main header, add other includes only if the implementation needs to forward-include something from a later package in the dependency queue. Avoid direct intra-package header includes from source files.
- If an implementation file requires a rare or file-specific include, keep it local to that implementation file; do not add it to `PackageName.h`. We want the global header include stack optimally short for later packages.
- Every package must provide a main header named `PackageName.h`. This header aggregates/includes all other headers in the package. Keep it minimal beyond aggregation for clarity (small packages may be exceptions).
- Prefer that headers other than the main header do not declare a namespace. Instead, the main header should wrap the aggregated headers in `namespace Upp` via the `NAMESPACE_UPP` macro. This reduces clutter and plays better with BLITZ.
- Treat this as a no-exceptions pattern unless explicitly justified. Many non-U++ C++ conventions differ; we require this approach here.
- Do not add `#include` statements to any package headers other than the main header. The only exception is truly inline header includes that belong to that header. Rationale: it is ugly for BLITZ and third-party/system headers might end up under `NAMESPACE_UPP` when pulled via the main header wrapper.
- `.icpp` files are not "companion includes" for headers. Treat `.icpp` as implementation files (like `.cpp`/`.c`): compile them normally, and have them start with `#include "PackageName.h"` and any rare, file-specific includes afterward as needed.

### CRITICAL: Avoiding Nested Namespace (`Upp::Upp::`) Errors

**Problem**: Including U++ headers inside `namespace Upp { }` causes nested namespace issues. The `UPP` macro expands to `Upp`, so `UPP::VppLog()` becomes `Upp::Upp::VppLog()` when used inside namespace Upp. This breaks macros like `LOG`, `INITBLOCK`, etc.

**Compile-time check**: Core/Defs.h contains `extern int Upp;` which triggers a "redefinition as different kind of symbol" error if you try to declare `namespace Upp` after including Core headers. This catches the error early.

**Rules to prevent nested namespace errors**:

1. **Main header structure**: Only the main package header (`PackageName.h`) should have `NAMESPACE_UPP`/`END_UPP_NAMESPACE`.

2. **Sub-headers**: Headers included FROM within the main header's namespace block must NOT have their own `NAMESPACE_UPP`. They are already inside the namespace.

3. **No includes inside namespace**: Sub-headers must NOT include other headers. Only the main header may include other headers (before entering the namespace).

4. **Implementation files**: `.cpp` files should include the main header (e.g., `#include "PackageName.h"`), not sub-headers directly.

**Correct pattern**:
```cpp
// PackageName.h (main header)
#ifndef _Package_PackageName_h_
#define _Package_PackageName_h_

#include <Core/Core.h>      // Includes go BEFORE namespace
#include <OtherDep/Dep.h>

NAMESPACE_UPP

#include "SubHeader1.h"     // These have NO namespace or includes
#include "SubHeader2.h"

END_UPP_NAMESPACE
#endif

// SubHeader1.h (sub-header)
#ifndef _Package_SubHeader1_h_
#define _Package_SubHeader1_h_
// NO includes here
// NO NAMESPACE_UPP here

class MyClass { ... };

#endif

// Implementation.cpp
#include "PackageName.h"    // Include main header, NOT sub-headers

NAMESPACE_UPP
// Implementation...
END_UPP_NAMESPACE
```

**Wrong patterns that cause `Upp::Upp::`**:
```cpp
// WRONG: Include inside namespace
NAMESPACE_UPP
#include "SubHeader.h"      // If SubHeader.h has NAMESPACE_UPP → nested!

// WRONG: Sub-header with own namespace when included from main
// SubHeader.h
#include <Core/Core.h>      // WRONG: sub-headers should not include
NAMESPACE_UPP               // WRONG: will nest if included from main header
class MyClass { ... };
END_UPP_NAMESPACE

// WRONG: .cpp including sub-header directly
#include "SubHeader.h"      // Should include "PackageName.h" instead
```

Subpackage Independence
- Subpackages like `AI`, `AI/Core`, `AI/Core/Core` are independent packages; do not gather headers in the parent package.
- A parent package may include only the subpackage's main header (e.g., `#include "Core.h"` from `AI`). Do not cross-include subpackage internals directly.
- Subpackages must chain dependencies correctly by including previous subpackages or other packages as needed, without creating circular dependencies.
- In general, a subpackage should not include its parent package. The reverse is acceptable when the subpackage genuinely extends the parent.


Book Chronicle
- Chronicle work in `Book/` by pairing first-person chapters (`Book/<index> - <Title>.md`) with compact summaries that reuse the chapter title (`Book/<Title>.md`).
- Keep both files current whenever work progresses; refer to the user in third person with he/him pronouns, selecting from {Spearhead, Captain, Curator, Director, Chief, Ringleader} to match context.
- Preserve prior chapters—append new material so the narrative reflects real-time progress.

Book Contribution Gate (Mandatory)
- Before editing anything under `Book/`, you must read `Book/AGENTS.md` and follow its style rules (headings, Date Span placement, list formats, and reference style).
- Pull requests changing `Book/*` must explicitly confirm compliance (e.g., checklist item: "Read and applied Book/AGENTS.md").
- Maintainers: reject or request changes if the above confirmation or formatting is missing.

## Graphics Pipeline and FBO Atom Data Flow

### FBO Atom Data Transmission
- `FboAtomT::Send` now transmits both graphics state ("gfxstate") and rendered framebuffer content ("gfxbuf")
- This allows downstream video sink atoms to display the rendered output from FBO programs
- Previously only scene data was transmitted, causing white screen issues

### GfxAccelAtom Reception
- `GfxAccelAtom::Recv` now treats "gfxbuf" packets the same as "gfxvector" packets
- Both are stored in `fb_packet` which is processed in the Render method as direct framebuffer data
- This ensures rendered framebuffer content is properly displayed instead of being treated as input scene data

### Architecture
- FBO programs render scene data to internal framebuffers
- The rendered content is transmitted through the packet system to video sink atoms
- Video sink atoms display the framebuffer content to the screen

## Skybox Cubemap Keys

### Skybox Bindings
- `skybox.display` controls the visible background cubemap (shader uniform `iCubeDisplay`, texture slot `TEXTYPE_CUBE_DISPLAY`)
- `skybox.specular` is the prefiltered specular IBL cubemap (falls back to `skybox.diffuse` for legacy content)
- `skybox.irradiance` is the diffuse IBL cubemap
- If `skybox.display` is not provided, the renderer uses the specular cubemap for display

## Known Graphics Rendering Issues and Fixes

### Stereo Rendering (BufferStageT::SetStereo)
**Location**: `uppsrc/api/Graphics/TBufferStage.cpp:10-21`

**Issue**: The TODO at line 12 caused crashes when stereo tests (03k, 03l) attempted to run. The code was always using `fb[0]` for both left and right eye framebuffers.

**Fix**: Changed to use `fb[stereo_id]` where stereo_id selects between separate framebuffers for left (0) and right (1) eyes:
```cpp
auto& fb = this->fb[stereo_id];  // Instead of this->fb[0]
```
Added bounds checking with ASSERT to prevent invalid stereo_id values.

**Remaining Issue**: While stereo tests now run without crashing, the left image doesn't update - it stays frozen while the right image animates correctly. This points to a framebuffer update issue specific to the left eye buffer.

### X11 OpenGL HMD Initialization Bug (ScrX11Ogl::SinkDevice_Initialize)
**Location**: `uppsrc/api/Screen/X11Ogl.cpp:125`

**Issue**: 
1. The initialization was skipping the call to `dev.accel.Initialize(a, ws)`, which meant graphics stages were never populated. This led to an assertion failure `stages.GetCount()` in `TBuffer.cpp`.
2. Calls to `system()` for `start_hmd_x.py` and `xrandr` were wrapped in `IGNORE_RESULT`, causing the application to proceed even if HMD setup failed.
3. Fallback to "HDMI-A-1" masked detection failures.

**Fix**: 
1. Added the missing `dev.accel.Initialize(a, ws)` call.
2. Removed `IGNORE_RESULT` and added exit code checks for all system calls; `SinkDevice_Initialize` now returns `false` on script failure.
3. Removed the hardcoded "HDMI-A-1" fallback to ensure failure when detection fails.
4. Added post-setup verification to ensure the HMD output is reported as "connected" by `xrandr`, using a 5-second polling loop to handle hardware startup delays.
5. Implemented forced window placement: The X-coordinate is now set to the detected width of the "connected primary" screen, and the resolution is fixed to 2880x1440, ensuring correct HMD placement even when Window Managers attempt to override positions.

### Cube Texture Index Bug (Model::AddCubeTexture)
**Location**: `uppsrc/Geometry/Model.cpp:271`

**Issue**: Wrong collection used for indexing - code was using `textures.GetCount()-1` as an index into `cube_textures` AMap, causing out-of-bounds access when loading PBR skybox textures (test 03m).

**Fix**: Changed to use the correct collection for indexing:
```cpp
int id = cube_textures.IsEmpty() ? 0 : cube_textures.GetKey(cube_textures.GetCount()-1) + 1;
// Instead of: cube_textures.GetKey(textures.GetCount()-1) + 1
```

**Remaining Issue**: Test 03m now runs without crashing but has texture corruption - skybox cubemap appears corrupted and the gun model is black with no textures. The bug is somewhere in the image-to-shader transfer pipeline after loading.

### Texture Hash-to-Filename Mapping
**Location**: `uppsrc/Core/EcsEngine/Util2.cpp:101-139` (`ToyShaderHashToName`)

**Purpose**: ShaderToy shader tests often reference textures by SHA256 hash (from ShaderToy's texture library). We maintain a hash-to-filename mapping to convert these to local texture filenames.

**How it works**:
- ShaderToy `.toy` files reference textures like `"488bd40303a2e2b9a71987e48c66ef41f5e937174bf316d3ed0e86410784b919.jpg"`
- The hash is looked up in `ToyShaderHashToName()` which maps it to a local filename (e.g., `"bg4"`)
- The system then loads from `share/imgs/bg4.jpg` (or `bg4_1.jpg`, etc. for cubemaps)

**Cubemap Loading**:
- **Location**: `uppsrc/api/Graphics/ImageBase.cpp:37-73` (loading), `ImageBase.cpp:138-183` (transmission)
- For cubemap textures (`cubemap = true`), the system loads 6 cube faces:
  - Face 0: `filename.jpg` (the exact filename from the hash mapping)
  - Faces 1-5: `filename_1.jpg`, `filename_2.jpg`, ..., `filename_5.jpg`
- All 6 files must exist in `share/imgs/` for cubemap tests to work
- Example: hash `488bd...` → `bg4` → loads `bg4.jpg`, `bg4_1.jpg`, ..., `bg4_5.jpg`

**Cubemap Data Transmission Fix** (Eon06 test 2):
- **Issue**: `ImageBaseAtomT::Send` was only sending the first image (`imgs[0]`) for cubemaps, causing black screen
- **Fix**: Modified `Send` function to concatenate all 6 cubemap faces into the packet data buffer
- **Location**: `uppsrc/api/Graphics/ImageBase.cpp:155-171`
- The receiving end (`BufferStageT::InitializeCubemap` and `ReadCubemap`) expects all 6 faces in sequence

**Adding new texture mappings**:
1. Add the hash-to-name mapping in `ToyShaderHashToName()` in `Util2.cpp`
2. Place the texture file(s) in `share/imgs/`
3. For cubemaps, ensure all 6 faces exist with the `_1`, `_2`, etc. suffix pattern

## DemoRoom Assets and DDS

### DemoRoom Assets
- DemoRoom environment maps + BRDF LUT live in `share/demoroom/Environment/` and `share/demoroom/PBR/`.
- `share/demoroom/Copying` is the MIT license from MixedRealityToolkit; keep it with any new DemoRoom assets.
- HLSL shader sources from MixedRealityToolkit are mirrored in `share/shaders/hlsl/`.
- DemoRoom glb models live in `share/models/ms/` (Baseball/Gun/PaintBrush). `KnownModelNames::GetPath` uses `.glb` paths.
- DemoRoom tool scale reference: Baseball `0.15`, Gun `0.35`, PaintBrush `1.0`.
- DemoRoom `.glb` hashes should match the originals in `MixedRealityToolkit/SpatialInput/Samples/DemoRoom/Media/Models`.

### DDS Loader Notes
- `Draw/Extensions/SimpleImage` provides `LoadDdsImage`/`LoadDdsImages` for uncompressed RGBA and float RGBA DDS (A32B32G32R32F, A16B16G16R16F), including cubemaps.
- `ImageBaseAtomT` and `Model::LoadCubemapFile` use these helpers for `.dds` input.

### OpenGL Program Binding (ECS Rendering)
- `BufferStageT::Process` uses `Gfx::UseProgram` per model; on OpenGL do not keep a program pipeline bound at the same time.
- If `glDrawElements` throws `GL_INVALID_OPERATION`, ensure OpenGL path unbinds the program pipeline before draw (`Gfx::UnbindProgramPipeline`).
- apitrace “shader compiler issue … Shader Stats” lines are driver info, not link failures; only treat it as an error if the log reports a failed link (e.g. “program is not linked successfully”).

### ECS Script Args (DemoRoom)
- `ModelComponent` accepts `scale_x/scale_y/scale_z` aliases (same as `cx/cy/cz`); use these in `.eon` files for clarity.
- `PhysicsBody::test.fn` only supports `fixed` and `do.circle` (no `dynamic`).
- If prefab models are loaded via `ModelCache`, `GfxModelState::LoadModel` must still be called to populate GL buffers (otherwise only the skybox renders).

### Model Asset Paths
- `KnownModelNames::GetPath` resolves to `share/models/...` (e.g., `share/models/ms/Baseball.glb`, `share/models/ms/Gun.glb`, `share/models/ms/PaintBrush.glb`).

### Graphics Base Class Templates (GFXTYPE Macro Pattern)
**Location**: `uppsrc/api/Graphics/Base.h:185-192`

**Pattern**: Template base classes are defined for multiple graphics backends using a macro-based type aliasing system:

```cpp
#define GFXTYPE(x) \
	using x##ShaderBase = ShaderBaseT<x##Gfx>; \
	using x##TextureBase = TextureBaseT<x##Gfx>; \
	using x##FboReaderBase = FboReaderBaseT<x##Gfx>; \
	using x##KeyboardBase = KeyboardBaseT<x##Gfx>; \
	using x##AudioBase = AudioBaseT<x##Gfx>;
GFXTYPE_LIST
#undef GFXTYPE
```

**How it works**:
- Base templates like `KeyboardBaseT<Gfx>` are defined generically (Base.h:141-161)
- The `GFXTYPE` macro creates type aliases for specific graphics backends
- For example, `SdlOglKeyboardBase = KeyboardBaseT<SdlOglGfx>`
- This pattern is applied to all base templates: ShaderBase, TextureBase, FboReaderBase, KeyboardBase, AudioBase

**Concrete atom classes**:
- **Location**: `uppsrc/Eon/Lib/GeneratedMinimal.h:1072-1084`
- Concrete atoms inherit from these type-aliased bases
- Example: `class SdlOglKeyboardSource : public SdlOglKeyboardBase`
- The class is then registered in the Eon atom system via `uppsrc/EonApiEditor/Headers.cpp:789-798`

**Understanding the chain**:
1. Template base class: `KeyboardBaseT<Gfx>` (Base.h:141)
2. Type alias via macro: `SdlOglKeyboardBase = KeyboardBaseT<SdlOglGfx>` (Base.h:189)
3. Concrete atom class: `SdlOglKeyboardSource : public SdlOglKeyboardBase` (GeneratedMinimal.h:1072)
4. Registered action: `"sdl.ogl.fbo.keyboard"` (Headers.cpp:792)

**Why this pattern**:
- Allows writing generic graphics code that works across multiple backends (SDL+OpenGL, X11+OpenGL, Windows+DirectX, etc.)
- The concrete atoms are auto-generated from the Headers.cpp definitions
- Makes it easy to track which atoms belong to which graphics backend via type aliases

## Packet Router Architecture

### Overview
The PacketRouter replaces the loop-centric Eon dataflow runtime with a router-based architecture where Atoms expose explicit ports and all packet movement is mediated by a PacketRouter rather than Link chains.

### Key Features
- **Net syntax**: DSL uses `net` blocks instead of `loop` for defining connections
- **Explicit connections**: Connections defined as `atom:port -> atom:port` instead of implicit loop wiring
- **Port-based flow**: All atom channels become peer ports with optional names
- **Credit management**: Router-governed credit/buffer management for flow control
- **Arbitrary topology**: Networks can have fan-out/fan-in and non-circular topologies

### DSL Syntax Example
```eon
net audio_pipeline:
    center.customer
    center.audio.src.test
    center.audio.sink.test.realtime:
        dbg_limit = 100
    center.customer.0 -> center.audio.src.test.0
    center.audio.src.test.0 -> center.audio.sink.test.realtime.0
```

### Migration Status
- **Phases 0-4**: Completed (Core API, DSL integration, runtime flow)
- **Phase 5**: In progress (DSL migration & test coverage)
- **Phase 6**: Planned (Performance, compatibility, cleanup)

### Key Files
- `uppsrc/Eon/Core/PacketRouter.{h,cpp}` - Core router implementation
- `uppsrc/Eon/Script/ScriptLoader.cpp` - BuildNet() and DSL integration
- `uppsrc/Eon/Core/Context.{h,cpp}` - NetContext for router networks
- `uppsrc/Eon/AGENTS.md` - Detailed migration guide for atoms

### Benefits
- Supports arbitrary network topologies (not just loops)
- Explicit port-to-point connections for clarity
- Credit-based flow control instead of fixed packet pools
- Better diagnostics and topology inspection

---


## ECS Initialization and Component Lifecycle

### Overview

The ECS (Entity-Component-System) architecture has a precise initialization sequence. Understanding this lifecycle is critical for implementing components that reference other entities or systems.

### Initialization Phases

**Phase 1: Arg() - Configuration Storage**
- **When**: Called during DSL parsing and entity creation
- **Purpose**: Store configuration arguments for later use
- **Limitation**: Other entities/components may not exist yet
- **Best Practice**: Store paths, IDs, and configuration values; **defer resolution**

**Phase 2: Initialize() - Cross-Reference Resolution**
- **When**: Called after all entities and components are created
- **Purpose**: Resolve cross-references, find other entities/components, register with systems
- **Pattern**: This is where you resolve entity paths, find components on other entities, register with engine systems
- **Return**: `bool` - return `false` on failure to block entity initialization

**Phase 3: PostInitialize() - Dependency Completion**
- **When**: Called after all Initialize() calls complete
- **Purpose**: Handle dependencies on other components' Initialize() side effects
- **Example**: If Component A needs data that Component B sets up in Initialize(), Component A should use PostInitialize()

### Common Patterns

#### Pattern 1: Deferred Entity Path Resolution

**Problem**: Component needs to reference another entity by path (e.g., camera targeting a model).

**Wrong Approach** (fails because target entity doesn't exist yet):
```cpp
bool ChaseCam::Arg(String key, Value value) {
    if (key == "target") {
        // WRONG: Trying to resolve path during Arg phase
        EntityPtr target_ent = FindEntityByPath(value.ToString());
        target = target_ent->Find<Transform>();
    }
}
```

**Correct Approach** (store path, resolve in Initialize):
```cpp
// In Camera.h
String target_path;  // Store path for deferred resolution

// In Camera.cpp
bool ChaseCam::Arg(String key, Value value) {
    if (key == "target") {
        // Store path for later resolution
        target_path = value.ToString();
        return true;
    }
}

bool ChaseCam::Initialize(const WorldState& ws) {
    // Now resolve the path when all entities exist
    if (!target_path.IsEmpty()) {
        Val* root = &val.FindOwner<Engine>()->GetRootPool();
        EntityPtr tgt_ent = ResolveEntityPath(root, target_path);
        if (!tgt_ent) {
            LOG("ChaseCam::Initialize: error: could not find target entity");
            return false;
        }
        target = tgt_ent->Find<Transform>();
    }
    return true;
}
```

#### Pattern 2: Manual VfsValue Path Traversal

**Problem**: Standard `FindPath<Entity>()` requires all intermediate VfsValue nodes to be typed. Paths like `/world/ball` may have untyped intermediate nodes (`world` with `type_hash=0`).

**Solution**: Manual path traversal that handles untyped nodes:
```cpp
// Parse path: "/world/ball" -> ["world", "ball"]
String manual_path = target_path;
if (manual_path.StartsWith("/"))
    manual_path = manual_path.Mid(1);
Vector<String> parts = Split(manual_path, '/');

// Traverse VfsValue tree manually
VfsValue* current = root;
for (const String& part : parts) {
    int idx = -1;
    for (int i = 0; i < current->sub.GetCount(); i++) {
        if (current->sub[i].id == part) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        LOG("Path traversal failed at: " << part);
        return false;
    }
    current = &current->sub[idx];
}

// Extract Entity from final VfsValue
EntityPtr tgt_ent = current->FindExt<Entity>();
```

#### Pattern 3: Component-to-System Access

**Problem**: Component needs to register with an engine-level system.

**Wrong Approach** (searches entity scope, not engine):
```cpp
bool ChaseCam::Initialize(const WorldState& ws) {
    Entity* e = val.owner->FindExt<Entity>();
    // WRONG: Searches in entity's VfsValue, not engine
    RenderingSystemPtr rend = e->val.Find<RenderingSystem>();
}
```

**Correct Approach** (use GetEngine() for system access):
```cpp
bool ChaseCam::Initialize(const WorldState& ws) {
    // Systems live at engine level, not entity level
    RenderingSystemPtr rend = GetEngine().TryGet<RenderingSystem>();
    if (rend) {
        rend->AddCamera(*this);
    }
    return true;
}

void ChaseCam::Uninitialize() {
    // Mirror the registration in Uninitialize
    RenderingSystemPtr rend = GetEngine().TryGet<RenderingSystem>();
    if (rend)
        rend->RemoveCamera(*this);
}
```

#### Pattern 4: Entity Initialization Success Tracking

**Problem**: Tracking whether all components initialized successfully.

**Wrong Approach** (starts false, always returns false):
```cpp
bool Entity::InitializeComponents(const WorldState& ws) {
    bool b = false;  // BUG: starts pessimistic
    auto comps = val.FindAll<Component>();
    for(auto& comp : comps) {
        b = comp->Initialize(ws) && b;  // true && false = false!
    }
    return b;
}
```

**Correct Approach** (start optimistic, track failures):
```cpp
bool Entity::InitializeComponents(const WorldState& ws) {
    bool b = true;  // Start optimistic
    auto comps = val.FindAll<Component>();
    for(auto& comp : comps) {
        if (!comp->IsInitialized()) {
            bool success = comp->Initialize(ws);
            if (!success) {
                LOG("Entity::InitializeComponents: component "
                    << comp->GetTypeName() << " failed to initialize");
                b = false;
            }
            comp->SetInitialized(success);
        }
    }
    return b;
}
```

### Engine Initialization Sequence

The Engine follows this exact sequence (see `uppsrc/Vfs/Ecs/Engine.cpp:31-107`):

1. **System Initialize()**: Initialize all engine systems
2. **System PostInitialize()**: Post-initialize systems (two-phase init)
3. **Entity Component Initialize()**: Initialize all components in all entities
4. **Component PostInitialize()**: Post-initialize all components
5. **System Start()**: Start all systems (begin running)

### Common Errors and Solutions

**Error**: "Could not find entity with path '/world/ball'"
- **Cause**: Using `FindPath<Entity>()` with untyped intermediate VfsValue nodes
- **Fix**: Use manual path traversal (Pattern 2)

**Error**: "System not found" when calling `e->val.Find<System>()`
- **Cause**: Searching for system in entity scope instead of engine scope
- **Fix**: Use `GetEngine().TryGet<System>()` (Pattern 3)

**Error**: "Entity initialization failed" even when individual Initialize() calls succeed
- **Cause**: Boolean accumulation logic bug (starting with `false`)
- **Fix**: Start with `true`, only set to `false` on actual failure (Pattern 4)

### Best Practices

1. **Always defer cross-references**: Never resolve entity paths or find other entities in Arg()
2. **Use Initialize() for registration**: Register with systems, resolve paths, find components in Initialize()
3. **Check initialization order**: If Component A depends on Component B's Initialize() side effects, use PostInitialize() in A
4. **Return false on real failure**: Initialize() should return false only when the component cannot function
5. **Mirror registration/unregistration**: If you register with a system in Initialize(), unregister in Uninitialize()
6. **Log failures clearly**: Include component name, what failed, and why in error messages

### Debugging Tips

**Enable runtime logging**: Use `RTLOG()` to see initialization flow:
```cpp
RTLOG("ChaseCam::Initialize: resolving target path '" << target_path << "'");
```

**Check VfsValue tree structure**: Print tree to see type_hash values:
```cpp
RTLOG("VfsValue id='" << val.id << "' type_hash=" << val.type_hash
      << " sub.GetCount()=" << val.sub.GetCount());
```

**Verify initialization sequence**: Check Engine output shows correct order:
```
Engine::Start: initializing all systems
Engine::Start: found 3 entities
Engine::Start: initializing all entities and components
Engine::Start: found 12 components
Engine::Start: post-initializing all components
```

---


## Code Readability & AI-Friendly Refactoring Philosophy

This project actively welcomes improvements to code readability and error detectability. The goal is to make problems **obvious** to both AI agents and human developers.

### Core Principles

1. **AI Readability is a Priority**
   - Code should be intuitive for pattern-matching systems to understand
   - Hidden complexity (macros, implicit conversions, shared namespaces) should be made explicit
   - Error messages should guide toward the root cause

2. **Rename Operations Are Welcome**
   - Find-and-replace across all files is a **standard, accepted operation**
   - Always commit before large renames to enable easy reversion
   - Rename to improve clarity, reduce ambiguity, or prevent common mistakes

3. **Suggestions Always Welcome**
   - If naming, structure, or error messages make problems hard to detect → propose improvements
   - If macros hide important information → consider renaming or restructuring
   - If similar names cause confusion → disambiguate them

### Examples of Good Improvements

**Example 1: Disambiguating Shared Namespaces**
- **Problem**: `REGISTER_COMPONENT` and `REGISTER_SYSTEM_ECS` both register into the same global factory with `eon_name` as the key, but different macros falsely suggest separate namespaces
- **Solution**: Renamed to `REGISTER_EON_COMPONENT` and `REGISTER_EON_SYSTEM` to emphasize the shared `EON_*` namespace
- **Why**: Makes it obvious they share the same key space and can collide

**Example 2: Type-Specific Lookups**
- **Problem**: `FindFactoryEon(eon_name)` could match both components and systems with the same name
- **Solution**: Added type parameter: `FindFactoryEon(eon_name, type)` to disambiguate
- **Why**: Prevents incorrect matches and makes intent explicit at call site

**Example 3: Better Error Messages**
- **Bad**: `"failed to create component 'physics'"`
- **Good**: `"failed to create component 'physics': found factory with eon_name='physics' but type=VFSEXT_SYSTEM_ECS (expected VFSEXT_COMPONENT)"`
- **Why**: Immediately reveals the root cause (name collision between types)

### When to Propose Refactoring

Propose refactoring when:
- **Naming is misleading**: Variable/class/macro names that suggest wrong behavior
- **Errors are cryptic**: Error messages that don't explain what went wrong
- **Collisions are possible**: Shared namespaces without type disambiguation
- **Macros hide information**: Critical details buried in macro expansion
- **Similar names differ subtly**: `GetAdd()` vs `AstGetAdd()` with different semantics

### How to Propose Changes

1. **Explain the confusion**: What made the problem hard to detect?
2. **Show the fix**: Concrete rename or restructure proposal
3. **Verify scope**: Use `grep -r` to find all affected locations
4. **Commit first**: Always commit before find-and-replace operations
5. **Test after**: Rebuild and verify tests pass after rename

### Meta-Awareness for AI Agents

When you encounter a bug that was hard to detect, ask:
- Why did the code structure hide this problem?
- Would renaming make the issue obvious?
- Would better error messages catch this earlier?
- Are there other places with similar ambiguity?

**Remember**: This codebase prioritizes clarity over brevity. Making problems obvious is more valuable than elegant abstraction.

## Memory Management & Pooling

### RecyclerPool and BiVectorRecycler

For high-performance scenarios involving frequent allocation/deallocation of fixed-size objects (like network buffers or image frames), prefer `RecyclerPool` and `BiVectorRecycler` over standard containers (`Vector`, `Array`).

#### RecyclerPool<T, keep_as_constructed>
- **Purpose**: Manages a pool of allocated objects of type `T`.
- **`keep_as_constructed=true`**: If true, the destructor of `T` is NOT called when returning to the pool, and the constructor is NOT called when allocating new items (after the initial allocation). This is ideal for reuse of complex objects like `Vector` buffers where you want to retain capacity.
- **Thread Safety**: Internally synchronized (can be used from multiple threads).

#### BiVectorRecycler<T, keep_as_constructed>
- **Purpose**: A double-ended queue (deque) that automatically manages object reuse via an internal `RecyclerPool`.
- **Usage**:
  ```cpp
  BiVectorRecycler<RawDataBlock, true> queue;
  RawDataBlock* block = queue.AddTail(); // Allocates or reuses
  // ... use block ...
  queue.DropHead(); // Returns to pool
  ```
- **Ownership**: The container owns the *pointers* and manages their lifecycle relative to the pool. Use `pick()` to transfer ownership of the active queue items to another `BiVectorRecycler` (e.g., passing data between threads).
- **Move Semantics**: Supports moving (`pick`, `std::move`). When moved, the source container becomes empty, and the destination takes over the active items. The underlying pools remain separate, but items can safely cross between compatible pools.

**Use Case Example (SoftHMD Camera)**:
- Replaced `std::vector` and manual shifting with `BiVectorRecycler<RawDataBlock, true>`.
- `RawDataBlock` contains a `Vector<byte>`.
- `keep_as_constructed=true` ensures the internal capacity of `Vector<byte>` is preserved when blocks are recycled, minimizing heap allocations.