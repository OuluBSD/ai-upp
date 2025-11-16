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