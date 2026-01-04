# QWEN - Simple AI Agent Guide

**For**: Qwen AI - Simple, clear instructions
**Important**: This guide uses simple language and repeats important rules
**See also**: [AGENTS.md](AGENTS.md) has ALL the rules. Read AGENTS.md first!

---

## Step 1: Read These Files First (Always!)

Before you do ANYTHING, read these files:

1. **[AGENTS.md](AGENTS.md)** - Read this file! It has all the rules!
2. **[CODESTYLE.md](CODESTYLE.md)** - Read this file! It has code style rules!
3. **[task/](task/)** - Look in this folder for current tasks

**Important**: You MUST read AGENTS.md before doing any work!

---

## Step 2: Understand What You're Working On

This is an Ultimate++ (U++) codebase.

**What is Ultimate++?**
- It's a C++ framework
- It has special rules about headers (read AGENTS.md!)
- It has special flags like `flagV1` (read AGENTS.md!)
- It uses a special build system (read AGENTS.md!)

**Current tasks** are in the `task/` folder:
- `task/Vfs.md` - Fixing VFS and Eon tests (HIGH PRIORITY)
- `task/VfsShell.md` - Shell and IDE work (MEDIUM PRIORITY)
- `task/stdsrc.md` - Making U++ work with STL (MEDIUM PRIORITY)

---

## Step 3: Follow the Rules

### Rule 1: Every .cpp File MUST Start With This

```cpp
#include "PackageName.h"
```

**Why?** Because U++ uses BLITZ. BLITZ needs this pattern.

**Example**:
```cpp
// In uppsrc/Geometry/Model.cpp
#include "Geometry.h"  // ALWAYS FIRST!

// other includes can go after if needed
```

**What NOT to do**:
```cpp
// WRONG! Don't do this!
#include "Model.h"      // Wrong! Use main header!
#include <vector>        // Wrong! System headers first!
#include "Geometry.h"    // Too late! Must be FIRST!
```

### Rule 1.5: CRITICAL - Never Nest Namespace Upp!

**Problem**: If you put headers inside `namespace Upp`, you get `Upp::Upp::` error!

**Why this happens**:
- U++ macros use `UPP::` which becomes `Upp::`
- If already inside `namespace Upp`, it becomes `Upp::Upp::`
- This breaks `LOG`, `INITBLOCK`, and other macros!

**The compile-time check**: Core/Defs.h has `extern int Upp;`. If you try to declare `namespace Upp` after including Core, you get "redefinition as different kind of symbol" error. This helps you catch the mistake!

**Simple rules**:
1. **Main header only**: Only `PackageName.h` should have `NAMESPACE_UPP`
2. **Sub-headers**: No `NAMESPACE_UPP` in headers included from main header
3. **No includes in sub-headers**: Sub-headers cannot include other headers
4. **Use main header**: `.cpp` files must include main header, not sub-headers

**CORRECT**:
```cpp
// PackageName.h (main header)
#include <Core/Core.h>     // Includes BEFORE namespace!

NAMESPACE_UPP
#include "SubHeader.h"     // SubHeader has NO namespace!
END_UPP_NAMESPACE

// SubHeader.h
#ifndef _SubHeader_h_
#define _SubHeader_h_
// NO includes here!
// NO NAMESPACE_UPP here!
class MyClass { };
#endif

// MyCode.cpp
#include "PackageName.h"   // Use MAIN header!
```

**WRONG** (causes Upp::Upp::):
```cpp
// WRONG: SubHeader.h with its own namespace
NAMESPACE_UPP              // WRONG! Will nest!
#include <Core/Core.h>     // WRONG! Sub-headers can't include!
class MyClass { };
END_UPP_NAMESPACE

// WRONG: .cpp using sub-header
#include "SubHeader.h"     // WRONG! Use main header!
```

**Remember**: If you see `Upp::Upp::` error, check your includes and namespaces!

### Rule 2: Understand flagV1

`flagV1` is a special flag.

**When you see this**:
```cpp
#ifdef flagV1
    // This is ORIGINAL U++ code
    // This code came from upstream
#else
    // This is OUR CUSTOM code
    // This code is ai-upp additions
#endif
```

**Remember**:
- `flagV1` = Original Ultimate++ code
- NOT `flagV1` = Our custom code
- Keep them separate!

### Rule 3: Don't Build in Sandbox

**Before running build scripts**, check if you can write to `~/.cache`:

```bash
# Check first!
mkdir -p ~/.cache/upp.out

# If that fails, DON'T run build scripts!
# Tell the user you can't build because of sandbox
```

**Build scripts** are in `script/` folder:
- `script/build_ide_console.sh`
- `script/build_upptst_eon06.sh`
- `script/build_node_editor.sh`

### Rule 4: Read Package AGENTS.md

Every package has an `AGENTS.md` file.

**Before editing files in a package**:
1. Look for `AGENTS.md` in that package folder
2. Read it!
3. Follow its rules

**Example**:
- Editing `uppsrc/Geometry/Model.cpp`?
- Read `uppsrc/Geometry/AGENTS.md` first!

### Rule 5: Update Task Files

When you work on something:

1. **Before starting**: Read `task/*.md` for that thread
2. **While working**: Take notes
3. **After finishing**: Update the task file

**Example**:
- Working on VFS fixes?
- Read `task/Vfs.md` first
- Update it when done

---

## Step 4: Common Tasks

### Task: Fix a Bug

1. Read AGENTS.md (has all rules!)
2. Read the task file (like `task/Vfs.md`)
3. Find the file to fix
4. Read that package's AGENTS.md
5. Make the fix
6. Follow Rule 1 (include "PackageName.h" first!)
7. Test the fix
8. Update the task file

### Task: Add a Feature

1. Read AGENTS.md (has all rules!)
2. Read the task file
3. Find where to add code
4. Read that package's AGENTS.md
5. Add the code
6. Follow Rule 1 (include "PackageName.h" first!)
7. Use Rule 2 (put custom code in NOT flagV1 section!)
8. Test it
9. Update the task file

### Task: Build and Test

1. Check Rule 3 (can you write to ~/.cache?)
2. If NO: Tell user you can't build
3. If YES: Run the build script

```bash
# Example: Build Eon06 test
script/build_upptst_eon06.sh

# Example: Run Eon06 test
bin/Eon06 0 0
```

---

## Step 5: Important Things to Remember

### Remember: Header Pattern

**Every .cpp file must start with**:
```cpp
#include "PackageName.h"
```

**Why?** U++ BLITZ needs this. Don't forget!

### Remember: flagV1

- `flagV1` = Original U++ code (from upstream)
- NOT `flagV1` = Our custom code (ai-upp additions)

**Don't mix them up!**

### Remember: Read AGENTS.md

**AGENTS.md has ALL the rules!**

Things in AGENTS.md:
- flagV1 convention (explained above)
- Header include policy (explained above)
- Build & sandbox policy (explained above)
- Subpackage independence
- Documentation standards
- Known graphics bugs
- FBO data flow
- And more!

**Read it before you do any work!**

### Remember: Current Tasks

Look in `task/` folder:
- `task/Vfs.md` - VFS and Eon tests (HIGH PRIORITY)
- `task/VfsShell.md` - Shell work (MEDIUM PRIORITY)
- `task/stdsrc.md` - STL wrapper (MEDIUM PRIORITY)
- `task/TODO.md` - Future work (LOW PRIORITY)

### Remember: Build Scripts

Build scripts are in `script/` folder.

**Before running them**:
1. Check sandbox (Rule 3!)
2. Make sure you can write to ~/.cache

**Common build scripts**:
- `script/build_ide_console.sh` - Build TheIDE console
- `script/build_upptst_eon06.sh` - Build Eon06 test
- `script/build_node_editor.sh` - Build node editor

---

## Step 6: What Files to Read

### Always Read These First

1. **[AGENTS.md](AGENTS.md)** ← Read this! It has ALL the rules!
2. **[CODESTYLE.md](CODESTYLE.md)** ← Code style rules
3. **[THREAD_DEPENDENCIES.md](THREAD_DEPENDENCIES.md)** ← What depends on what

### Read These for Tasks

- **[task/Vfs.md](task/Vfs.md)** - VFS fixes, Eon tests
- **[task/VfsShell.md](task/VfsShell.md)** - Shell, ConsoleIde
- **[task/stdsrc.md](task/stdsrc.md)** - U++ to STL wrapper
- **[task/TODO.md](task/TODO.md)** - Future work

### Read These for Packages

When working in a package:
- Look for `PackageName/AGENTS.md`
- Read it before changing code!

**Examples**:
- `uppsrc/Geometry/AGENTS.md`
- `uppsrc/Eon/AGENTS.md`
- `stdsrc/AGENTS.md`

---

## Quick Checklist

Before you do ANY work:

- [ ] Did I read AGENTS.md?
- [ ] Did I read CODESTYLE.md?
- [ ] Did I read the task file for what I'm working on?
- [ ] Did I read the package AGENTS.md?
- [ ] Do I understand Rule 1 (header pattern)?
- [ ] Do I understand Rule 2 (flagV1)?
- [ ] Do I understand Rule 3 (sandbox check)?
- [ ] Do I know where the build scripts are?

**If you answered NO to any of these, STOP and read those files first!**

---

## Summary

**Three most important things**:

1. **Read AGENTS.md** - It has ALL the rules. Read it first!

2. **Every .cpp file starts with**: `#include "PackageName.h"`
   - This is for U++ BLITZ
   - Don't forget this rule!

3. **flagV1 means original U++ code**, not flagV1 means our custom code
   - Keep them separate
   - Don't mix them up!

**Remember**: AGENTS.md has everything. Read it!

---

## If You're Confused

1. Read AGENTS.md again
2. Read CODESTYLE.md again
3. Read the task file again
4. Look for AGENTS.md in the package you're working on
5. Ask the user for help

**Most confusion comes from not reading AGENTS.md!**

---

## Understanding SdlOglKeyboardBase (Graphics Macro Pattern)

**What is it?** SdlOglKeyboardBase is a type alias created by a macro.

**How it works**:
1. There's a template class called `KeyboardBaseT<Gfx>`
2. A macro creates type aliases for different graphics backends
3. Example: `SdlOglKeyboardBase = KeyboardBaseT<SdlOglGfx>`

**The chain**:
- Step 1: Generic template → `KeyboardBaseT<Gfx>` (in Base.h)
- Step 2: Type alias → `SdlOglKeyboardBase` (created by GFXTYPE macro)
- Step 3: Concrete class → `SdlOglKeyboardSource : public SdlOglKeyboardBase`
- Step 4: Registered as → `"sdl.ogl.fbo.keyboard"` atom action

**Where to find it**:
- Template base: `uppsrc/api/Graphics/Base.h:141-161`
- Macro definition: `uppsrc/api/Graphics/Base.h:185-192`
- Concrete class: `uppsrc/Eon/Lib/GeneratedMinimal.h:1072-1084`
- Registration: `uppsrc/EonApiEditor/Headers.cpp:789-798`

**Why this pattern?**
- It works for multiple backends (SDL+OpenGL, X11+OpenGL, Windows+DirectX)
- The same base code works for all backends
- Easy to add new graphics backends

**Other classes using this pattern**:
- ShaderBase (for shaders)
- TextureBase (for textures)
- FboReaderBase (for framebuffer reading)
- AudioBase (for audio)
