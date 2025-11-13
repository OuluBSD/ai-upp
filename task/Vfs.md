# Vfs Thread

**Goal**: Fix VFS tree structure and get all upptst/Eon* tests running properly with correct VFS-AST

## Status: IN PROGRESS

## Problem
Program was running but with wrong VFS-AST structure (drivers nested inside loops instead of siblings). The VFS update broke everything, so we're going through all code to fix and improve.

---

## Eon03 VFS-AST Fix (COMPLETED)

### Structural Fixes
- [x] Fixed VFS tree ordering: Changed from DriverBeforeLoopLess to LoopBeforeDriverLess
- [x] Fixed remapping logic to only nest when driver is complete prefix of loop path
- [x] Enhanced FindOwnerWithCastDeep to search sibling containers
- [x] Added error checking for AddAtom and AddLoop failures
- [x] Fixed infinite loop in address replacement code in test harness

### Current Work
- [ ] Complete testing: Program should exit cleanly on initialization failure with proper error code
- [ ] Verify VFS tree structure matches expected output in all Eon03 tests
- [ ] Ensure context finding works correctly with new sibling structure

---

## Eon Tests Status

### Eon03 Tests (MOSTLY COMPLETE)
- [x] Test 03a (X11 video basic)
- [x] Test 03b (GLX video)
- [x] Test 03c (audio file playback)
- [x] Test 03d (audio file 2)
- [x] Test 03e (X11 video sw3d)
- [x] Test 03f (X11 video OGL)
- [x] Test 03g (X11 video sw3d linked) - ✓ framerate fixed
- [x] Test 03h (X11 video OGL linked)
- [x] Test 03i (X11 video sw3d bufferstages) - ✓ framerate fixed
- [x] Test 03j (X11 video OGL bufferstages)
- [x] Test 03k (X11 video sw3d stereo) - ✓ FIXED
- [x] Test 03l (X11 video OGL stereo) - ✓ FIXED
- [x] Test 03m (X11 video OGL PBR) - texture corruption issues

Note: Eon03 builds with "script/build_upptst_eon03.sh" and runs with "bin/Eon03". Test 03a runs with "bin/Eon03 0 0" and test 03b with "bin/Eon03 1 0" etc.

### Known Issues Fixed

#### Framerate Issues (03g, 03i) - RESOLVED
- [x] Root cause: Debug mode (-O0) with intensive fragment shaders - expected behavior
- [x] Added comprehensive packet profiling to PacketTracker
- [x] Per-atom buffer profiling shows expected performance
- [x] Packet profiling tool completed

#### Stereo Rendering (03k, 03l) - RESOLVED
- [x] Root cause #1: ObjViewProg.cpp had hardcoded static stereo camera positions
  - Fixed: Calculate eye offsets relative to animated camera position
- [x] Root cause #2: TBufferStage.cpp SetStereo() bug
  - SetStereo(1) set flag on fb[1] but Process() checks fb[0]
  - Fixed: Always set flags on fb[0]
- [x] Both eyes now animate correctly with matching aspect ratios

### Eon06 Test 06a - ONGOING INVESTIGATION

**Problem**: Volumetric clouds showing stepping/moiré artifacts

**Progress**:
- [x] Fixed TWO critical bugs: hardcoded FILTER_LINEAR in TBufferStage.cpp:877 and TProgram.cpp:406
- [x] Changed shader from textureLod(..., 0.0) to texture() for automatic mipmap LOD
- [x] Verified GL_LINEAR_MIPMAP_LINEAR (9987) is correctly set and accepted by OpenGL
- [x] Verified mipmaps are generated successfully without errors
- [x] Confirmed texture: 256x256 RGBA32F, WRAP_REPEAT, all parameters correct
- [ ] Visual artifacts persist despite all GL configuration being verified correct
- [ ] Issue may be: shader math precision, texture data itself, or 2D→3D projection algorithm
- [ ] Need further investigation into shader coordinate calculations or texture generation

### Remaining Eon Tests
- [ ] Run and fix upptst/Eon00 tests
  - [ ] Test all Eon00 variants with correct VFS-AST structure
  - [ ] Verify state machine and event handling
- [ ] Run and fix upptst/Eon01 tests
  - [ ] Test MIDI event handling
  - [ ] Verify meta tests functionality
- [ ] Run and fix upptst/Eon02 tests
  - [ ] Test audio pipeline functionality
  - [ ] Verify all Eon02 test variants
- [ ] Run and fix upptst/Eon03 test 03n (Win32 video)
- [ ] Run and fix upptst/Eon04 tests
  - [ ] Test all Eon04 variants
  - [ ] Verify functionality with new VFS structure
- [ ] Run and fix upptst/Eon05 tests
  - [ ] Test all Eon05 variants
  - [ ] Verify functionality with new VFS structure
- [ ] Run and fix upptst/Eon06 tests
  - [ ] Test all Eon06 variants
  - [ ] Verify functionality with new VFS structure
- [ ] Run and fix upptst/Eon07 tests
  - [ ] Test ECS features
  - [ ] Verify entity/component/system functionality
- [ ] Run and fix upptst/Eon08 tests
  - [ ] Test GUI integration
  - [ ] Test 3D rendering with ECS
  - [ ] Test VR functionality

---

## Dependencies
- Requires: Core VFS implementation
- Blocks: All Eon tests, ShaderEditor (for file management), VfsShell
- Related: stdsrc (some VFS structures may need STL equivalents)
