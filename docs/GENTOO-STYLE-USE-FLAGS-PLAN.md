# Gentoo-Style USE Flags System for U++

**Date**: 2026-01-10
**Status**: Planning Phase
**Priority**: High - Reduces rebuild times and improves build clarity

---

## Executive Summary

Current U++ "acceptflags" system **does not actually filter flags** - it only filters dot-prefixed flags (like `.TEST`), while regular flags are passed to all packages unconditionally. This causes unnecessary rebuilds when mainconfig flags change, even for packages that don't use those flags.

**Goal**: Implement Gentoo Portage-style USE flag system with:
- Proper flag filtering per package
- Negative flag support
- Verbose build planning (emerge -pv style)
- Plugin/external dependency management (ebuild-like)
- Optimized builds with shared libraries

---

## Current Situation Analysis

### How "accepts" Currently Works

**Location**: `uppsrc/ide/Core/Workspace.cpp:113-128`

```cpp
Vector<String> SplitFlags(const char *flags, bool main, const Vector<String>& accepts)
{
    Vector<String> v = SplitFlags0(flags);
    Vector<String> h;
    for(int i = 0; i < v.GetCount(); i++)
        if(v[i][0] == '.') {              // Only for DOT-PREFIXED flags
            String f = v[i].Mid(1);
            if(main || FindIndex(accepts, f) >= 0)  // Filter by accepts
                h.Add(v[i].Mid(1));
        }
        else
            h.Add(v[i]);                  // Regular flags ALWAYS passed through
    if(main)
        h.Add("MAIN");
    return h;
}
```

**Problem**:
- `acceptflags ABC;` in HelloWorld2.upp
- Mainconfig with `TEST` flag
- Result: HelloWorld2 still gets `TEST` flag!
- Cause: Regular flags bypass accepts filtering

**Impact**:
- Package `HelloWorld2 ( TEST CLANG DEBUG DEBUG_FULL BLITZ WIN32 )` rebuilds even though it doesn't use TEST
- Build output directory: `CLANGx64.Debug.Debug_Full.Test\HelloWorldStd.exe`
- All packages share same flag combination = lots of duplicate work

### Related Code Locations

- **Package struct**: `uppsrc/ide/Core/Core.h` (lines 393+)
- **Package parsing**: `uppsrc/ide/Core/Package.cpp:284-287` (acceptflags parsing)
- **Package saving**: `uppsrc/ide/Core/Package.cpp:477` (acceptflags writing)
- **Build system**: `uppsrc/ide/Builders/Build.cpp:30` (uses SplitFlags with GetAllAccepts)
- **Workspace**: `uppsrc/ide/Core/Workspace.cpp:251-265` (GetAllAccepts recursively collects from dependencies)
- **Package Editor**: `uppsrc/ide/Organizer.cpp:132` (UI for editing accepts)
- **Layout**: `uppsrc/ide/ide.lay:122` (UI field "&Accepts")

---

## The Ambitious Plan

### Core Principles

1. **Flag filtering is enforced** - packages only receive flags they declare
2. **Build cache per effective flags** - no rebuilds if package's flags unchanged
3. **Explicit requirements** - packages can require/reject/accept flags
4. **Negative flag support** - `-NOGUI`, `+REQUIRED`, `?OPTIONAL` syntax
5. **Verbose planning** - see exactly what gets built with which flags
6. **External dependency management** - ebuild-like system for plugins

---

## Phase 1: Core Flag Filtering

### Goals
- Extend Package struct with requires/rejects/defaults
- Implement proper flag filtering
- Update .upp parser
- Ensure backward compatibility

### Extended .upp Format

```cpp
// New sections:
requireflags TEST, MAIN;           // Package REQUIRES these flags (error if missing)
acceptflags GUI, OPENGL, AUDIO;    // Package CAN USE these flags (existing)
rejectflags SERVER, HEADLESS;      // Package REJECTS these flags (error if present)
defaultflags GUI;                  // Default flags if not in mainconfig
```

### Implementation

**File**: `uppsrc/ide/Core/Core.h`
```cpp
struct Package {
    Vector<String> accepts;    // Existing
    Vector<String> requires;   // NEW: Must have these
    Vector<String> rejects;    // NEW: Cannot have these
    Vector<String> defaults;   // NEW: Default flags
    // ...
};
```

**File**: `uppsrc/ide/Core/Package.cpp`
- Add parsing for `requireflags`, `rejectflags`, `defaultflags`
- Mirror the existing `acceptflags` parsing pattern (lines 284-287)
- Add saving logic (after line 477)

**File**: `uppsrc/ide/Core/Workspace.cpp`
```cpp
// NEW FUNCTION
Vector<String> Workspace::GetEffectiveFlags(int pk, const Vector<String>& mainflags) {
    Vector<String> effective;
    const Package& pkg = package[pk];

    // 1. Add all requires (error if missing from mainflags)
    for(auto& req : pkg.requires) {
        if(FindIndex(mainflags, req) < 0)
            throw Error(Format("Package %s requires flag %s", package.GetKey(pk), req));
        effective.Add(req);
    }

    // 2. Check for rejects (error if present)
    for(auto& rej : pkg.rejects) {
        if(FindIndex(mainflags, rej) >= 0)
            throw Error(Format("Package %s rejects flag %s", package.GetKey(pk), rej));
    }

    // 3. Add accepted flags from mainflags
    for(auto& f : mainflags) {
        if(FindIndex(pkg.accepts, f) >= 0)
            effective.Add(f);
    }

    // 4. Add defaults if not overridden
    for(auto& def : pkg.defaults) {
        if(FindIndex(effective, def) < 0 && FindIndex(mainflags, def) < 0)
            effective.Add(def);
    }

    return effective;
}

// MODIFY EXISTING
Vector<String> SplitFlags(const char *flags, bool main, const Vector<String>& accepts) {
    // NEW LOGIC: Actually enforce accepts filtering for ALL flags
    Vector<String> v = SplitFlags0(flags);
    Vector<String> h;

    if(main) {
        // Main package gets all flags
        h = v;
        h.Add("MAIN");
    }
    else {
        // Non-main packages: filter by accepts
        for(int i = 0; i < v.GetCount(); i++) {
            String flag = v[i];
            if(flag[0] == '.')
                flag = flag.Mid(1);  // Remove dot prefix

            if(FindIndex(accepts, flag) >= 0)
                h.Add(flag);
        }
    }

    return h;
}
```

**File**: `uppsrc/ide/Builders/Build.cpp:30`
- Replace `SplitFlags(mainparam, package == 0, wspc.GetAllAccepts(package))`
- With: `wspc.GetEffectiveFlags(package, SplitFlags0(mainparam))`

**File**: `uppsrc/ide/Organizer.cpp`
- Add UI fields for requireflags, rejectflags, defaultflags
- Update PackageEditor dialog

### Backward Compatibility
- Empty requires/rejects/defaults = current behavior
- Existing .upp files work unchanged
- acceptflags still works (now actually enforced!)

### Testing
1. Create test package with `acceptflags ABC;`
2. Set mainconfig to `TEST DEBUG`
3. Verify package does NOT receive TEST flag
4. Verify build cache key excludes irrelevant flags

---

## Phase 2: Build Planning & Verbosity

### Goals
- Implement build planner that shows what will be built
- Display flag combinations per package
- Show dependency tree with flags
- Compute cache optimization opportunities

### New Command (umk CLI)

```bash
umk --show-plan MyApp "TEST CLANG DEBUG"
```

**Output**:
```
Build Plan for MyApp [TEST CLANG DEBUG]:
================================

[main] MyApp [TEST CLANG DEBUG MAIN]
  └─ [uses] Core [CLANG DEBUG]
  └─ [uses] Draw [GUI CLANG DEBUG]
       └─ [uses] plugin/png [CLANG DEBUG]
  └─ [uses] AudioCore [AUDIO CLANG DEBUG TEST]
       └─ [uses] SoftAudio [CLANG DEBUG]

Packages to build: 6
Flag combinations:
  - [CLANG DEBUG]: Core, plugin/png, SoftAudio (3 packages)
  - [GUI CLANG DEBUG]: Draw (1 package)
  - [AUDIO CLANG DEBUG TEST]: AudioCore (1 package)
  - [TEST CLANG DEBUG MAIN]: MyApp (1 package)

Total unique builds: 4
Cache optimization: 2 builds saved (33% reduction)
```

### Implementation

**File**: `uppsrc/Core/BuildPlanner.h` (NEW)
```cpp
struct BuildNode {
    String package;
    Vector<String> flags;
    Array<BuildNode> dependencies;
    bool is_cached;
    String cache_key;
};

class BuildPlanner {
    Workspace& wspc;
    VectorMap<String, BuildNode> nodes;  // package name -> node
    VectorMap<String, Vector<String>> flag_groups;  // flag combo -> packages

public:
    void ComputePlan(const String& main_pkg, const Vector<String>& mainflags);
    void ShowPlan(Stream& out, bool verbose = false);
    void OptimizeBuilds();
    int GetUniqueBuilds() const;
    int GetTotalPackages() const;
};
```

**File**: `uppsrc/umk/main.cpp`
- Add `--show-plan` option
- Parse mainconfig flags
- Create BuildPlanner
- Display results

### Benefits
- Immediately see what TEST flag affects
- Debug unnecessary rebuilds
- Plan refactoring (move flags to accepts lists)
- Validate package metadata

---

## Phase 3: Negative Flags & Advanced Expressions

### Syntax Extensions

**Mainconfig**:
```
TEST DEBUG -NOGTK +AUDIO
```
- `-NOGTK`: Remove NOGTK from flags if present
- `+AUDIO`: Assert AUDIO is present (error otherwise)

**Package .upp**:
```cpp
requireflags +GUI, +MAIN;       // Required (+ optional)
acceptflags ?OPENGL, ?AUDIO;    // Optional (? optional)
rejectflags -HEADLESS;          // Rejected (- optional)
```

**When expressions** (existing feature, enhance):
```cpp
uses(GUI && !HEADLESS) DrawDraw;
library(AUDIO || PULSEAUDIO) "-lasound";
flags(TEST && DEBUG) TESTING, MOCK_ENABLED;
```

### Implementation
- Update `SplitFlags0()` to handle +/- prefixes
- Extend `MatchWhen()` for complex boolean expressions
- Add validation in GetEffectiveFlags()

---

## Phase 4: Plugin System (umk CLI)

**IMPORTANT**: Implement in `uppsrc/umk`, NOT in `uppsrc/ide`

### Directory Structure

```
uppsrc/plugin-repo/          # Metadata only (checked into git)
├── png/
│   ├── png.ebuild          # Download & patch instructions
│   ├── patches/
│   │   └── 01-upp-integration.patch
│   └── metadata.upp        # Package metadata
├── freetype/
│   └── ...

uppsrc/plugin-src/           # Downloaded source (.gitignore)
├── png-1.6.40/             # Extracted here
└── freetype-2.13.2/

uppsrc/plugin/               # Integrated & patched (as now, but generated)
├── png/
│   ├── png.h               # Copied/symlinked from plugin-src
│   ├── png.c
│   └── png.upp             # Generated from metadata.upp
```

### Ebuild Format

**File**: `uppsrc/plugin-repo/png/png.ebuild`
```bash
# U++ Plugin Ebuild Format
PACKAGE="png"
VERSION="1.6.40"
HOMEPAGE="http://www.libpng.org/"
SRC_URI="https://downloads.sourceforge.net/libpng/libpng-${VERSION}.tar.gz"
LICENSE="PNG"

upp_fetch() {
    wget "${SRC_URI}" -O "${UPP_DISTFILES}/libpng-${VERSION}.tar.gz"
}

upp_extract() {
    tar xzf "${UPP_DISTFILES}/libpng-${VERSION}.tar.gz" -C "${UPP_PLUGIN_SRC}"
    mv "${UPP_PLUGIN_SRC}/libpng-${VERSION}" "${UPP_PLUGIN_SRC}/${PACKAGE}-${VERSION}"
}

upp_patch() {
    cd "${UPP_PLUGIN_SRC}/${PACKAGE}-${VERSION}"
    patch -p1 < "${UPP_EBUILD_DIR}/patches/01-upp-integration.patch"
}

upp_integrate() {
    mkdir -p "${UPP_PLUGIN}/${PACKAGE}"
    cp "${UPP_PLUGIN_SRC}/${PACKAGE}-${VERSION}/"*.{h,c} "${UPP_PLUGIN}/${PACKAGE}/"

    # Generate .upp from metadata
    upp_generate_package "${PACKAGE}"
}
```

### umk Commands

```bash
# In uppsrc/umk/main.cpp - add these commands:

umk plugin list                    # Show all available plugins
umk plugin status                  # Show installed/source-imported/missing
umk plugin fetch png               # Download source for png
umk plugin fetch --all             # Download all missing
umk plugin integrate png           # Apply patches & integrate
umk plugin update png [version]    # Update to newer version
umk plugin clean png               # Remove source & integrated files
```

### Benefits
- Clean separation: metadata (git) vs source (local)
- Easy updates (change ebuild, re-run)
- Patch management (like Gentoo)
- No more huge binary blobs in repo
- Standard process for adding new external deps

---

## Phase 5: Shared Libraries & Build Optimization

### 5.1 Compilation Cache with Flag-Based Keys

**Current**: `out/ai-uppsrc_upptst/CLANGx64.Debug.Debug_Full.Test/`

**Proposed**:
```
out/cache/
├── Core[CLANG.DEBUG]/
│   ├── *.o
│   └── .metadata.json  # Flags used, timestamps, dependencies
├── Core[CLANG.DEBUG.TEST]/
│   └── ...
└── Draw[GUI.CLANG.DEBUG]/
    └── ...
```

**Key insight**: Only rebuild when package's **effective flags** change

### 5.2 Shared Library Mode

**Note**: This is NOT a .upp file section - it's a build mode option

**umk command**:
```bash
umk --linkmode shared MyApp "DEBUG GUI"
```

**theide GUI**: Build settings dialog option

**Result**:
- Core compiled as Core.so (or .dll on Windows)
- Draw compiled as Draw.so
- MyApp links against shared libraries
- Multiple apps can share same .so files

**Benefits**:
- Faster incremental builds (only changed .so)
- Smaller disk usage (shared libs)
- Deployment flexibility
- RAM savings (shared memory)

### 5.3 Asset Compilation

**Note**: Assets go in regular `file` section, NOT a new section

**Use existing custom build steps**:
```cpp
// In .upp file:
file
    data/sprites.atlas,
    data/shaders/main.glsl;

custom "*.atlas"
    "python scripts/compile_sprites.py $INPUT",
    "$OUTDIR/$BASENAME.pak";

custom "*.glsl"
    "glslangValidator -V $INPUT -o $OUTDIR/$BASENAME.spv",
    "$OUTDIR/$BASENAME.spv";
```

**Benefits**:
- Reuses existing custom step system
- No new .upp syntax needed
- Standard build dependency tracking

---

## Phase 6: UI & Integration

### TheIDE Package Editor Enhancements

**New tabs/sections**:
1. **Flags Tab**:
   - Visual flag selector
   - Show requires/accepts/rejects/defaults
   - Validate flag combinations
   - Preview effective flags for current mainconfig

2. **Dependencies Tab**:
   - Dependency graph viewer
   - Show flag propagation through deps
   - Highlight circular dependencies

3. **Build Plan Tab**:
   - Preview what would be built
   - Show cache hits/misses
   - Estimate build time

4. **Plugins Tab**:
   - Browse available plugins
   - Show installed vs available
   - One-click fetch/integrate
   - Update notifications

### Flag Validation

Real-time checks:
- "Flag XYZ accepted by no packages" warning
- "Package ABC requires flag DEF but it's not set" error
- "Conflicting flags: GUI and HEADLESS" error
- Suggestions: "Add OPENGL to accepts to enable feature"

### Build Configuration Manager

Visual tool to:
- Create/edit mainconfigs with flag picker
- See which packages affected by each flag
- Compare build plans between configs
- Optimize flag usage

---

## Implementation Roadmap

### Milestone 1: Core Flag Filtering
**Priority**: Critical
**Effort**: 2-3 weeks

Tasks:
- [ ] Extend Package struct (requires, rejects, defaults)
- [ ] Update .upp parser for new sections
- [ ] Implement GetEffectiveFlags()
- [ ] Modify SplitFlags to actually enforce filtering
- [ ] Update build system to use per-package flags
- [ ] Update PackageEditor UI
- [ ] Write tests for flag filtering
- [ ] Document new .upp syntax

**Success criteria**: Package with `acceptflags ABC;` does NOT receive unrelated flags

### Milestone 2: Build Planning
**Priority**: High
**Effort**: 1-2 weeks

Tasks:
- [ ] Implement BuildPlanner class
- [ ] Add --show-plan to umk
- [ ] Compute effective flags per package
- [ ] Group packages by flag combination
- [ ] Display dependency tree with flags
- [ ] Show cache optimization stats

**Success criteria**: `umk --show-plan` shows accurate build preview

### Milestone 3: Negative Flags
**Priority**: Medium
**Effort**: 1 week

Tasks:
- [ ] Add +/- prefix parsing
- [ ] Extend MatchWhen for complex expressions
- [ ] Add validation in GetEffectiveFlags
- [ ] Update documentation
- [ ] Add tests

**Success criteria**: `-NOGUI` correctly removes flag, `+REQUIRED` validates presence

### Milestone 4: Plugin System (umk)
**Priority**: High
**Effort**: 2-3 weeks

Tasks:
- [ ] Design ebuild format
- [ ] Create uppsrc/plugin-repo/ structure
- [ ] Implement plugin commands in umk
- [ ] Write ebuild interpreter
- [ ] Create metadata.upp schema
- [ ] Migrate 2-3 existing plugins as proof-of-concept
- [ ] Document ebuild authoring

**Success criteria**: `umk plugin fetch png` downloads and integrates libpng

### Milestone 5: Shared Libraries
**Priority**: Medium
**Effort**: 2-3 weeks

Tasks:
- [ ] Implement --linkmode shared in umk
- [ ] Modify builders to support .so/.dll output
- [ ] Update cache system for flag-based keys
- [ ] Handle shared library versioning
- [ ] Add shared lib support to all platforms
- [ ] Performance testing

**Success criteria**: App builds with shared libs, runs correctly, incremental build faster

### Milestone 6: UI Integration
**Priority**: Low (nice to have)
**Effort**: 1-2 weeks

Tasks:
- [ ] Add Flags tab to PackageEditor
- [ ] Implement flag validation UI
- [ ] Add build plan preview
- [ ] Create plugin manager UI
- [ ] Add flag conflict detection
- [ ] Polish and documentation

**Success criteria**: Users can visually manage flags and plugins

**Total Timeline**: 9-14 weeks for full implementation

---

## Migration Strategy

### Backward Compatibility

1. **No breaking changes**: Existing .upp files work unchanged
2. **Opt-in**: New features require explicit opt-in
3. **Graceful degradation**: Missing requires/rejects = current behavior
4. **Validation warnings**: Suggest improvements without breaking builds

### Gradual Adoption Path

**Week 1-2**: Internal testing
- Update Core package with requires/accepts
- Test flag filtering
- Verify no regressions

**Week 3-4**: Core packages
- Add metadata to Draw, plugin/*, uppsrc/* packages
- Run build plan analyzer
- Fix flag leakage issues

**Week 5-6**: User packages
- Documentation
- Migration guide
- Example packages
- Community feedback

**Week 7+**: Plugin system
- Convert existing plugins to ebuild format
- Community can add external deps easily

---

## User Corrections & Notes

1. **umk vs ide**: CLI ebuild support goes in `uppsrc/umk`, NOT `uppsrc/ide`
2. **Shared mode**: NOT a .upp section - it's a build mode option (umk flag or theide GUI)
3. **Assets**: Use existing `file` section + `custom` build steps, NO new `assets` section
4. **Flag types**: Need to check what flag-related fields already exist in Package struct

---

## Maestro Integration Notes

### Attempted Setup

**Track created**: `gentoo-style-use-flags-system`
**Status**: Partial success

**Issues encountered**:
1. Phase ID collision: All phases get ID "phase" by default
   - Need to use `--id` parameter explicitly
   - Example: `maestro phase add --id core-flag-filtering --track gentoo-style-use-flags-system "Phase 1: Core Flag Filtering"`

2. Unicode encoding error on `maestro track list`
   - Error: `UnicodeEncodeError: 'charmap' codec can't encode characters`
   - Windows console encoding issue (cp1252)
   - **Maestro bug**: Should handle encoding properly for Windows console

### Recommended Maestro Improvements

1. **Auto-generate unique phase IDs**:
   - Use sanitized version of name if `--id` not provided
   - Or increment: phase-1, phase-2, etc.

2. **Windows console compatibility**:
   - Force UTF-8 output encoding
   - Use `sys.stdout.reconfigure(encoding='utf-8')` in Python
   - Or add `--ascii` flag for safe output

3. **Bulk operations**:
   - `maestro phase import <track_id> phases.yaml` to add multiple phases
   - `maestro task import <phase_id> tasks.yaml` to add multiple tasks

4. **Better error messages**:
   - "Phase ID 'phase' already exists" should suggest: "Use --id to specify unique ID"

### Current Maestro State

```bash
# Track exists
gentoo-style-use-flags-system (Gentoo-Style USE Flags System)

# Phase exists (only one due to ID collision)
phase (Phase 1: Core Flag Filtering)
```

### To Continue with Maestro

```bash
# Add phases with explicit IDs
cd "E:\active\sblo\Dev\ai-upp"

python ..\Maestro\maestro.py phase add --id core-flags --track gentoo-style-use-flags-system "Phase 1: Core Flag Filtering"
python ..\Maestro\maestro.py phase add --id build-plan --track gentoo-style-use-flags-system "Phase 2: Build Planning & Verbosity"
python ..\Maestro\maestro.py phase add --id negative-flags --track gentoo-style-use-flags-system "Phase 3: Negative Flags & Expressions"
python ..\Maestro\maestro.py phase add --id plugin-system --track gentoo-style-use-flags-system "Phase 4: Plugin System (umk CLI)"
python ..\Maestro\maestro.py phase add --id shared-libs --track gentoo-style-use-flags-system "Phase 5: Shared Libraries & Build Optimization"
python ..\Maestro\maestro.py phase add --id ui-integration --track gentoo-style-use-flags-system "Phase 6: UI & Integration"

# Add tasks to each phase
python ..\Maestro\maestro.py task add --phase core-flags "Extend Package struct with requires/rejects/defaults"
python ..\Maestro\maestro.py task add --phase core-flags "Update .upp parser for new sections"
# ... etc
```

---

## Next Steps

### Immediate Actions

1. **Review & Refine Plan**: Get stakeholder approval on phases
2. **Priority Decision**: Which phase to start with?
   - Recommendation: Phase 1 (Core Flag Filtering) - highest impact
   - Alternative: Phase 2 (Build Planning) - best for debugging current system

3. **Prototype**: Build proof-of-concept for chosen phase
   - 2-3 day spike to validate approach
   - Identify technical risks early

4. **Maestro Setup**: Fix phase ID issues and populate full task tree

### Questions to Answer

1. Are there existing flag-related fields in Package we should reuse?
2. Should we support gradual rollout (packages opt-in to strict filtering)?
3. What's the performance impact of per-package flag computation?
4. How to handle builder-specific flags (CLANG vs GCC)?
5. Should plugin-repo/ be separate git repository or submodule?

---

## Appendix: Code References

### Key Files for Phase 1

1. `uppsrc/ide/Core/Core.h` - Package struct definition
2. `uppsrc/ide/Core/Package.cpp` - .upp parsing and saving
3. `uppsrc/ide/Core/Workspace.cpp` - SplitFlags implementation, GetAllAccepts
4. `uppsrc/ide/Builders/Build.cpp` - Build system that calls SplitFlags
5. `uppsrc/ide/Organizer.cpp` - PackageEditor UI
6. `uppsrc/ide/ide.lay` - UI layout definition

### Testing Strategy

Create test packages:
```
test/FlagFilter/
├── Main.upp              # requireflags TEST; acceptflags DEBUG;
├── NoFlags.upp          # acceptflags (empty)
└── AllFlags.upp         # acceptflags TEST, DEBUG, GUI;
```

Build with: `TEST DEBUG GUI EXTRA`

Expected results:
- Main gets: TEST, DEBUG, MAIN
- NoFlags gets: (empty)
- AllFlags gets: TEST, DEBUG, GUI

Current (broken) results:
- All get: TEST, DEBUG, GUI, EXTRA

---

**END OF PLAN**

For questions or updates, see: https://github.com/your-repo/issues
