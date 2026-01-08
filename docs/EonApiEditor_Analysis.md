# EonApiEditor Modernization Analysis

## Overview

EonApiEditor is a code generator that creates API interface files for the Eon system. The tool is **severely outdated** after major repository restructuring. Running it without updates would write to wrong locations and use obsolete dependencies.

## Critical Issues

### 1. Path Problems

#### Old paths (hardcoded in Interface.cpp:130-171)
```cpp
#ifdef flagWIN32
String prj_dir = "C:\\git\\libtopside";
#else
String prj_dir = AppendFileName(GetHomeDirectory(), "libtopside");
#endif
String par_dir = AppendFileName(prj_dir, "src");
```

**Problems:**
- Hardcoded `C:\\git\\libtopside` path doesn't exist
- Uses old `src` directory that was restructured
- API packages generated to `src/IHal`, `src/IScreen`, etc.
- Atom files generated to `src/ParallelMach` (doesn't exist)

#### Current paths (actual structure)
- API packages: `uppsrc/api/Hal`, `uppsrc/api/Screen`, etc.
- Generated atom files: `uppsrc/Eon/Lib/Generated*.{h,cpp}`
- No `ParallelMach` directory exists

### 2. Dependency Issues

#### InterfaceBuilder::Add*() functions have outdated dependencies

**Hal.cpp (line 9):**
```cpp
Dependency("ParallelLib");  // ❌ DOESN'T EXIST
Dependency("IGraphics");     // ❌ Wrong name
```

**Current uppsrc/api/Hal/Hal.upp:**
```upp
uses
    api/Graphics;  // ✅ Correct
```

**Screen.cpp (lines 9-10):**
```cpp
Dependency("ParallelLib");  // ❌ DOESN'T EXIST
Dependency("IGraphics");     // ❌ Wrong
```

**Current uppsrc/api/Screen/Screen.upp:**
```upp
uses
    api/Graphics;
```

**Audio.cpp (lines 9-11):**
```cpp
Dependency("ParallelLib");              // ❌ DOESN'T EXIST
Dependency("ports/portaudio", "...");   // ❌ Wrong structure
Library("portaudio", "PORTAUDIO");
```

**Current uppsrc/api/Audio/Audio.upp:**
```upp
uses
    Eon,
    Sound;
```

**Pattern:** ALL Add*() functions reference `ParallelLib` which no longer exists.

### 3. Missing .upp File Configurations

Current `uppsrc/api/*/*.upp` files have evolved significantly:

#### Graphics.upp additions not in EonApiEditor:
- `uses Geometry, plugin/MingwDx, SoftRend, Painter`
- Conditional `uses(WIN32 & GCC) plugin/glew_`
- Complex library dependencies for FFMPEG, codecs, etc.
- `noblitz;` directive

#### Screen.upp additions:
- Extra vendors: `WinOgl` (not in Screen.cpp)
- Different library conditionals
- `Impl.inl` file reference

#### Hal.upp changes:
- Color changed from `198,127,200` to `0,128,0`
- Completely different library specifications
- SDL2 library conditionals changed

#### Synth.upp:
- `uses SoftInstru, SoftSynth, SoftAudio, api/AudioHost`
- Conditional `uses(FLUIDLITE) plugin/fluidlite`
- Conditional `uses(LV2) plugin/lilv`
- Much more complex than AddSynth() generates

### 4. Manual Changes to Generated Files

Comparing EonApiEditor output vs current `GeneratedMinimal.{h,cpp}`:

#### Header file changes (.h):

**EonApiEditor generates:**
```cpp
public:
    //RTTI_DECL1(CenterCustomer, CustomerBase)
    COPY_PANIC(CenterCustomer)
    static String GetAction();
    ...
```

**Current GeneratedMinimal.h:**
```cpp
public:
    ATOM_CTOR_(CenterCustomer, CustomerBase)
    //ATOMTYPE(CenterCustomer)
    static String GetAction();
    ...
```

**Changes:**
- Replaced `COPY_PANIC()` with `ATOM_CTOR_()` macro
- Changed commented RTTI to commented ATOMTYPE

#### Implementation file changes (.cpp):

**EonApiEditor generates:**
```cpp
namespace Upp {
namespace Parallel {

AtomTypeCls CenterCustomer::GetAtomType() {
    AtomTypeCls t;
    t.sub = AsTypeCls<CenterCustomer>();
    t.role = AtomRole::CUSTOMER;
    ...
}

void CenterCustomer::Visit(Vis& vis) {
    vis.VisitThis<CustomerBase>(this);
}
```

**Current GeneratedMinimal.cpp:**
```cpp
NAMESPACE_UPP

AtomTypeCls CenterCustomer::GetAtomType() {
    AtomTypeCls t;
    t.sub = SUB_ATOM_CLS; //CENTER_CUSTOMER;
    t.role = AtomRole::CUSTOMER;
    ...
}

void CenterCustomer::Visit(Vis& v) {
    VIS_THIS(CustomerBase);
}
```

**Changes:**
- Uses U++ macros (`NAMESPACE_UPP`, `VIS_THIS`) instead of raw C++
- `t.sub = SUB_ATOM_CLS` instead of `AsTypeCls<...>()`
- Added commented atom type enums

#### #ifdef Guards

Both versions have conditional compilation guards, but current uses simpler format:

**EonApiEditor:** `#if defined flagPORTAUDIO`
**Current:** `#if defined flagPORTAUDIO` (same)

User notes these were manually added because EonApiEditor didn't include enough guards initially.

### 5. Headers.cpp Complexity

The `Headers.cpp` file defines ~100+ atom types with:
- Input/output port specifications
- Action names
- Link types
- HINT_PKG for categorization (AtomMinimal, AtomAudio, AtomVR, AtomHandle)
- Conditional compilation requirements

This appears correctly structured but relies on the outdated base classes from Add*() functions.

## Required Changes

### High Priority

1. **Fix paths in Interface.cpp Generate():**
   - Remove hardcoded `C:\\git\\libtopside`
   - Change `src` to `uppsrc`
   - API packages: `uppsrc/api/{name}` not `src/I{name}`
   - Generated files: `uppsrc/Eon/Lib/Generated{PkgName}.{h,cpp}` not `src/{PkgName}/Generated.*`
   - Remove `ParallelMach` references entirely

2. **Update all InterfaceBuilder::Add*() functions:**
   - Remove ALL `Dependency("ParallelLib")` lines
   - Change `Dependency("IGraphics")` to appropriate packages
   - Update all library specifications to match current .upp files
   - Add new conditional dependencies (plugin/glew_, plugin/fluidlite, etc.)
   - Update vendor lists

3. **Sync .upp generation with current patterns:**
   - Add `noblitz;` where present
   - Handle `uses(condition) package;` syntax
   - Support complex library() conditionals with multiple flags
   - Include `Impl.inl` references

4. **Update code generation templates:**
   - Use `NAMESPACE_UPP` instead of `namespace Upp {`
   - Use `ATOM_CTOR_()` instead of `COPY_PANIC()`
   - Use `VIS_THIS()` macro instead of `vis.VisitThis<>()`
   - Use `SUB_ATOM_CLS` instead of `AsTypeCls<>()`
   - Preserve commented atom type enums

### Medium Priority

5. **Backport manual .upp changes:**
   - Graphics: Geometry, SoftRend, Painter, MingwDx dependencies
   - Screen: WinOgl vendor, updated libraries
   - Synth: SoftInstru, SoftSynth, SoftAudio, AudioHost dependencies
   - MidiHw: Current structure
   - Effect: LV2 plugin support

6. **Package-specific Updates:**
   - **Hal**: Different color, updated libraries, remove ParallelLib
   - **Screen**: Add WinOgl vendor, D3DCompiler in DX11 libs
   - **Audio**: Change to Eon/Sound dependencies
   - **Synth**: Add fluidlite/lilv conditional uses
   - **MidiHw**: Update portmidi handling
   - **Effect**: Add LV2 support
   - **Camera**: Check against current .upp
   - **Volumetric**: Check against current .upp
   - **Holographic**: Check against current .upp
   - **AudioFileOut**: Check against current .upp
   - **MidiFile**: Ensure matches current

### Low Priority

7. **Build script updates:**
   - `build_uppsrc_EonApiEditor.ps1` delegates to `build_upptst_eon_generic.ps1`
   - Verify paths and package references
   - Test build after changes

8. **Testing strategy:**
   - Generate to temporary directory first
   - Compare output with current files
   - Verify #ifdef patterns
   - Check for missing/extra atoms
   - Validate .upp file syntax

## Specific File Mapping

### API Packages (InterfaceBuilder::Add* functions)

| Function | Old Output | Current Location | Status |
|----------|-----------|------------------|--------|
| AddHal() | src/IHal/* | uppsrc/api/Hal/* | ❌ Wrong path, deps |
| AddScreen() | src/IScreen/* | uppsrc/api/Screen/* | ❌ Wrong path, deps |
| AddAudio() | src/IAudio/* | uppsrc/api/Audio/* | ❌ Wrong path, deps |
| AddVolumetric() | src/IVolumetric/* | uppsrc/api/Volumetric/* | ❌ Wrong path, deps |
| AddCamera() | src/ICamera/* | uppsrc/api/Camera/* | ❌ Wrong path, deps |
| AddHolographic() | src/IHolographic/* | uppsrc/api/Holograph/* | ❌ Wrong path, deps |
| AddSynth() | src/ISynth/* | uppsrc/api/Synth/* | ❌ Wrong path, deps |
| AddEffect() | src/IEffect/* | uppsrc/api/Effect/* | ❌ Wrong path, deps |
| AddMidiHw() | src/IMidiHw/* | uppsrc/api/MidiHw/* | ❌ Wrong path, deps |
| AddAudioFileOut() | src/IAudioFileOut/* | uppsrc/api/AudioFileOut/* | ❌ Wrong path, deps |

### Generated Atom Files (from Headers.cpp HINT_PKG)

| HINT_PKG | Old Output | Current Location | Status |
|----------|-----------|------------------|--------|
| AtomMinimal | src/AtomMinimal/Generated.* | uppsrc/Eon/Lib/GeneratedMinimal.* | ❌ Wrong path, format |
| AtomAudio | src/AtomAudio/Generated.* | uppsrc/Eon/Lib/GeneratedAudio.* | ❌ Wrong path, format |
| AtomVR | src/AtomVR/Generated.* | uppsrc/Eon/Lib/GeneratedVR.* | ❌ Wrong path, format |
| AtomHandle | src/AtomHandle/Generated.* | uppsrc/Eon/Lib/GeneratedHandle.* | ❌ Wrong path, format |

### Removed/Non-existent

| Old Reference | Status |
|---------------|--------|
| src/ParallelMach/Generated.h | ❌ Directory doesn't exist |
| src/ParallelMach/GenAtom.inl | ❌ Directory doesn't exist |
| ParallelLib package | ❌ Package deleted |

## Dependencies Summary

### To Remove:
- `ParallelLib` (everywhere)
- `IGraphics` (use `api/Graphics` instead)
- Old `ports/portaudio` style (restructured)

### To Add/Update per Package:

**Graphics:**
- Geometry, plugin/MingwDx, SoftRend, Painter
- Conditional plugin/glew_, plugin/glext

**Audio:**
- Eon, Sound

**Hal:**
- api/Graphics only

**Screen:**
- api/Graphics only

**Synth:**
- SoftInstru, SoftSynth, SoftAudio, api/AudioHost
- Conditional plugin/fluidlite, plugin/lilv

**MidiHw:**
- MidiFile
- Conditional plugin/portmidi

**Effect:**
- Conditional plugin/lilv

**Holograph:**
- Conditional plugin/hcidump, SoftHMD

## Recommendations

1. **DO NOT run current EonApiEditor** - it will write to wrong locations
2. **Start with path fixes** - make it write to correct directories
3. **Update dependencies incrementally** - one Add*() function at a time
4. **Compare generated vs current** - ensure compatibility
5. **Preserve manual changes** - the macros (ATOM_CTOR_, VIS_THIS, etc.) are improvements
6. **Test incrementally** - build each package after generation
7. **Version control** - commit working state before regenerating

## Next Steps

1. Fix Interface.cpp Generate() paths (lines 130-140, 171, 642)
2. Update Hal.cpp dependencies as template
3. Generate and compare with current Hal.upp
4. Repeat for each Add*() function
5. Update code generation macros
6. Regenerate and test build
7. Document changes in CURRENT_TASK.md

## Files to Modify

- `uppsrc/EonApiEditor/Interface.cpp` - Generate() function paths
- `uppsrc/EonApiEditor/Hal.cpp` - Dependencies, libraries
- `uppsrc/EonApiEditor/Screen.cpp` - Dependencies, libraries, vendors
- `uppsrc/EonApiEditor/Audio.cpp` - Dependencies completely changed
- `uppsrc/EonApiEditor/Volumetric.cpp` - Check and update
- `uppsrc/EonApiEditor/Camera.cpp` - Check and update
- `uppsrc/EonApiEditor/Holographic.cpp` - Check and update
- `uppsrc/EonApiEditor/Synth.cpp` - Major updates needed
- `uppsrc/EonApiEditor/Effect.cpp` - LV2 additions
- `uppsrc/EonApiEditor/MidiHw.cpp` - Restructure
- `uppsrc/EonApiEditor/AudioFileOut.cpp` - Check and update

## Warning

The gap between generated code and current manual edits is SIGNIFICANT. Consider whether:
1. Generator should match current macro style
2. Manual changes should be preserved via custom templates
3. Generator needs extension points for customization
4. Some files should remain fully manual

User explicitly noted generator was used initially but files have been manually maintained since. This suggests generator might be deprecated in favor of manual editing, OR generator needs major upgrades to match current patterns.
