# EonApiEditor Modernization - Changes Made

## Summary

EonApiEditor has been updated to work with the restructured ai-upp repository. All paths, dependencies, and code generation have been modernized.

## Major Changes

### 1. Commandline Argument Support (main.cpp)
- Added root path as optional first argument
- Fallback to `E:\active\sblo\Dev\ai-upp` or `~/Dev/ai-upp` if not specified
- Added validation and error messages

### 2. Path System Overhaul (Interface.cpp)
**Old paths:**
- Hardcoded `C:\git\libtopside`
- Generated to `src/IHal`, `src/IScreen`, etc.
- Generated atoms to `src/AtomMinimal/Generated.*`

**New paths:**
- Root from cmdline/default
- API packages to `uppsrc/api/Hal`, `uppsrc/api/Screen`, etc.
- Atom files to `uppsrc/Eon/Lib/GeneratedMinimal.{h,cpp}`, etc.
- Single `uppsrc/Eon/Lib/Lib.icpp` for all registrations

### 3. Code Generation Updates
**Header files (.h):**
- Changed `COPY_PANIC()` to `ATOM_CTOR_()`
- Changed commented `//RTTI_DECL1()` to `//ATOMTYPE()`
- Removed `namespace Upp { namespace Parallel {` wrapper
- Updated header guards to `_EonLib_Generated*_h_` format

**Implementation files (.cpp):**
- Changed `namespace Upp { ... }` to `NAMESPACE_UPP / END_UPP_NAMESPACE`
- Changed `AsTypeCls<T>()` to `SUB_ATOM_CLS`
- Changed `vis.VisitThis<Base>(this)` to `VIS_THIS(Base)`
- Include `Lib.h` instead of package-specific headers

**Init system:**
- Removed per-package `Init.cpp` files
- Generate single `Lib.icpp` with all `REGISTER_ATOM()` calls
- Organized by package: Audio, Handle, Minimal, VR
- Uses `VfsValueExtFactory::RegisterAtom<>()`

### 4. Deleted Obsolete Code
- Removed `ParallelMach/Generated.h` generation (obsolete)
- Removed `ParallelMach/GenAtom.inl` generation (obsolete)
- Removed `flag_headers` logic (no longer needed)

### 5. Dependency Updates

**All packages:**
- Removed `Dependency("ParallelLib")` (deleted package)
- Changed color to standard `0, 128, 0`

**Hal:**
- Changed from `IGraphics` to `api/Graphics`
- Updated SDL2 library specifications to match current .upp

**Screen:**
- Changed from `IGraphics` to `api/Graphics`
- Added `opengl32 glu32` library for WIN32 & OGL
- Added D3DCompiler to DX11 libraries

**Audio:**
- Changed from `ParallelLib` to `Eon` and `Sound`
- Removed `ports/portaudio`, handled separately

**Synth:**
- Changed `AudioCore` to `SoftAudio`
- Changed `AudioHost` to `api/AudioHost`
- Changed `ports/fluidlite` to `plugin/fluidlite`
- Changed `ports/lilv` to `plugin/lilv`

**Camera:**
- Changed from `IMedia` to `api/Media`

**Volumetric:**
- No dependencies (just removed ParallelLib)

**Holograph:**
- Changed from `IMedia` to `api/Media`
- Changed `ports/hcidump` to `plugin/hcidump`
- Changed `LocalHMD` to `SoftHMD` with SOFTHMD condition

**Effect:**
- Changed `AudioCore` to `SoftAudio`
- Changed `AudioHost` to `api/AudioHost`
- Changed `ports/lilv` to `plugin/lilv`

**MidiHw:**
- Changed `ports/portmidi` to `plugin/portmidi`

**AudioFileOut:**
- No dependencies (just removed ParallelLib)

## API Package Generation

**Generated files per package:**
- `uppsrc/api/{Name}/{Name}.upp` - Package descriptor
- `uppsrc/api/{Name}/{Name}.h` - Main header with vendor/interface macros
- `uppsrc/api/{Name}/IfaceFuncs.inl` - Interface function declarations

**Header guard format:** `_I{Name}_I{Name}_h_` (preserved for compatibility)
**Includes:** Always adds `#include <Eon/Eon.h>` before dependencies

## Atom Class Generation

**Generated files:**
- `uppsrc/Eon/Lib/GeneratedMinimal.h` (was src/AtomMinimal/Generated.h)
- `uppsrc/Eon/Lib/GeneratedMinimal.cpp`
- `uppsrc/Eon/Lib/GeneratedAudio.h`
- `uppsrc/Eon/Lib/GeneratedAudio.cpp`
- `uppsrc/Eon/Lib/GeneratedHandle.h`
- `uppsrc/Eon/Lib/GeneratedHandle.cpp`
- `uppsrc/Eon/Lib/GeneratedVR.h`
- `uppsrc/Eon/Lib/GeneratedVR.cpp`
- `uppsrc/Eon/Lib/Lib.icpp` - All atom registrations

## Usage

### Build
```powershell
cd E:\active\sblo\Dev\ai-upp
.\script\win\build_uppsrc_EonApiEditor.ps1
```

### Run
```powershell
# With default path
.\bin\EonApiEditor.exe

# With custom path
.\bin\EonApiEditor.exe C:\path\to\project

# On Linux/Mac
./bin/EonApiEditor ~/Dev/ai-upp
```

### Output
The generator will:
1. Create/update `uppsrc/api/*/` directories with .upp and .h files
2. Create/update `uppsrc/Eon/Lib/Generated*.{h,cpp}` files
3. Create/update `uppsrc/Eon/Lib/Lib.icpp` with all registrations
4. Log all file paths being written

## Testing

After running:
1. Check generated `uppsrc/api/Hal/Hal.upp` matches current
2. Check generated `uppsrc/api/Hal/Hal.h` matches current structure
3. Check `uppsrc/Eon/Lib/GeneratedMinimal.h` uses `ATOM_CTOR_()` macro
4. Check `uppsrc/Eon/Lib/GeneratedMinimal.cpp` uses `VIS_THIS()` and `SUB_ATOM_CLS`
5. Check `uppsrc/Eon/Lib/Lib.icpp` has all atom registrations
6. Try building affected packages

## Notes

- Generator now matches manual editing style (macros, formatting)
- All paths are relative to root directory
- ParallelLib references completely removed
- Library specifications updated to match current .upp files
- Init system consolidated into single Lib.icpp file
- Colors standardized to `0, 128, 0` for all packages

## Files Modified

### Core Generator
- `uppsrc/EonApiEditor/main.cpp` - Added cmdline arg handling
- `uppsrc/EonApiEditor/Interface.h` - Added `SetRootPath()` method
- `uppsrc/EonApiEditor/Interface.cpp` - Complete path and generation overhaul

### Package Definitions
- `uppsrc/EonApiEditor/Hal.cpp` - Updated dependencies and libraries
- `uppsrc/EonApiEditor/Screen.cpp` - Updated dependencies and libraries
- `uppsrc/EonApiEditor/Audio.cpp` - Updated dependencies
- `uppsrc/EonApiEditor/Volumetric.cpp` - Removed ParallelLib
- `uppsrc/EonApiEditor/Camera.cpp` - Updated dependencies
- `uppsrc/EonApiEditor/Holographic.cpp` - Updated dependencies
- `uppsrc/EonApiEditor/Synth.cpp` - Updated dependencies and libraries
- `uppsrc/EonApiEditor/Effect.cpp` - Updated dependencies
- `uppsrc/EonApiEditor/MidiHw.cpp` - Updated dependencies and libraries
- `uppsrc/EonApiEditor/AudioFileOut.cpp` - Removed ParallelLib
