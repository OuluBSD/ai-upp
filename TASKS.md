# TASKS.md

## IN PROGRESS

- [x] CRITICAL: Analyze and implement correct overlay architecture: MetaEnvironment needs VirtualNode root that navigates multiple VfsOverlay instances per file
- [x] CRITICAL: Each VfsOverlay must contain VfsValue root for individual files only, not MetaEnvironment global root
- [x] CRITICAL: Implement VirtualNode-to-Overlays navigation system for unified view across all active overlays
- [x] CRITICAL: Maintain backward compatibility while restructuring MetaEnvironment from single VfsValue root to overlay-based system
- [x] CRITICAL: Ensure file-level isolation where each .cpp/.h file has its own overlay with its AST data
- [x] CRITICAL: Ensure VfsValue::file_hash matches Overlay's file_hash for proper correlation
- [x] CRITICAL: Update serial handling - each overlay tracks rolling revision numbers (serial), not global system
- [x] CRITICAL: Implement overlay serial tracking system to maintain comparison capabilities between overlays

### THREAD: vfs-ast-fix (rename to eon-tests later)
**Goal**: Fix VFS tree structure and get all upptst/Eon* tests running properly with correct VFS-AST
**Problem**: Program was running but with wrong VFS-AST structure (drivers nested inside loops instead of siblings)

#### Current Work: Eon03 VFS-AST Fix
- [x] Fixed VFS tree ordering: Changed from DriverBeforeLoopLess to LoopBeforeDriverLess
- [x] Fixed remapping logic to only nest when driver is complete prefix of loop path
- [x] Enhanced FindOwnerWithCastDeep to search sibling containers
- [x] Added error checking for AddAtom and AddLoop failures
- [x] Fixed infinite loop in address replacement code in test harness
- [ ] Complete testing: Program should exit cleanly on initialization failure with proper error code
- [ ] Verify VFS tree structure matches expected output in all Eon03 tests
- [ ] Ensure context finding works correctly with new sibling structure

#### Get All Eon Tests Running
- [ ] Run and fix upptst/Eon00 tests
  - [ ] Test all Eon00 variants with correct VFS-AST structure
  - [ ] Verify state machine and event handling
- [ ] Run and fix upptst/Eon01 tests
  - [ ] Test MIDI event handling
  - [ ] Verify meta tests functionality
- [ ] Run and fix upptst/Eon02 tests
  - [ ] Test audio pipeline functionality
  - [ ] Verify all Eon02 test variants
- [ ] Run and fix upptst/Eon03 tests (current focus)
  - [x] Test 03a (X11 video basic)
  - [x] Test 03b (GLX video)
  - [x] Test 03c (audio file playback)
  - [x] Test 03d (audio file 2)
  - [x] Test 03e (X11 video sw3d)
  - [x] Test 03f (X11 video OGL)
  - [x] Test 03g (X11 video sw3d linked) - runs but has framerate issues
  - [x] Test 03h (X11 video OGL linked)
  - [x] Test 03i (X11 video sw3d bufferstages) - runs but has framerate issues
  - [x] Test 03j (X11 video OGL bufferstages)
  - [x] Test 03k (X11 video sw3d stereo) - runs but left image doesn't change
  - [x] Test 03l (X11 video OGL stereo) - runs but left image doesn't change
  - [x] Test 03m (X11 video OGL PBR) - runs but has texture corruption issues
  - [ ] Test 03n (Win32 video)

#### Eon03 Known Issues to Fix
- [ ] Fix framerate issues in tests 03g and 03i
  - [ ] Add FPS printing to X11 stop or uninitialize functions
  - [ ] Investigate why framerate is low in linked and bufferstages tests
  - [ ] Compare with working tests to identify performance bottleneck
- [ ] Fix stereo rendering in tests 03k and 03l
  - [ ] Left image is stuck/not updating while right image works correctly
  - [ ] Both images should show scene from slightly different camera positions
  - [ ] Issue likely related to framebuffer handling in stereo mode
- [ ] Fix texture corruption in test 03m (PBR with skybox)
  - [ ] Background cube skybox has corrupted texture data
  - [ ] Gun model appears black with no textures
  - [ ] Bug occurs somewhere in image-to-shader transfer pipeline
  - [ ] Check cubemap loading, texture upload, and shader binding
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

### THREAD: stdsrc
**Goal**: Implement wrapper library in stdsrc that implements U++ Core functions using STL std c++ libraries

- [x] Keep working on wrapper library in stdsrc that implements U++ Core functions using STL std c++ libraries
- [x] Implement stdsrc/{Draw, CtrlCore, CtrlLib} wrapper libraries for WXWidgets/Gtk/Qt and native platform APIs
- [x] Update stdtst packages to test all wrapper library features comprehensively
- [x] Convert Eon/Win VR ECS engine to work with OpenVR and OpenHMD in addition to current WinRT implementation
- [x] Address WinRT limitations and ensure UWP (Universal Windows Platform) compatibility for Eon/Win
- [x] Create CMake files to enable Visual Studio compilation for Eon/Win project
- [ ] Implement missing classes in stdsrc/Core to match uppsrc/Core functionality:
  - [x] Atomic.h
  - [x] BiCont.h
  - [x] BinUndoRedo.h
  - [x] CharSet.h
  - [x] CoAlgo.h
  - [x] CoSort.h
  - [x] CoWork.h
  - [x] CritBitIndex.h
  - [x] Cpu.h
  - [x] Daemon.h
  - [x] Debug.h
  - [x] Diag.h
  - [x] Dli.h
  - [x] FileMapping.h
  - [x] FilterStream.h
  - [x] FixedMap.h
  - [x] Fn.h
  - [x] Heap.h
  - [x] Huge.h
  - [x] Inet.h
  - [x] Ini.h
  - [x] InMap.hpp
  - [x] InVector.h
  - [x] LinkedList.h
  - [x] LocalProcess.h
  - [x] Mt.h
  - [x] NetNode.h
  - [x] Ops.h
  - [x] Other.h
  - [x] PackedData.h
  - [x] Parser.h
  - [x] Random.h
  - [x] Range.h
  - [x] Recycler.h
  - [x] Shared.h
  - [x] SIMD.h
  - [x] Socket.h
  - [x] Sorted.h
  - [x] Speller.h
  - [x] SplitMerge.h
  - [x] St.h
  - [x] Topic.h
  - [x] Topt.h
  - [x] UnicodeInfo.h
  - [x] Utf.h
  - [x] ValueCache.h
  - [x] ValueUtil.h
  - [x] Vcont.hpp
  - [x] WebSocket.h
  - [x] Win32Util.h
  - [x] Xmlize.h
  - [x] xxHsh.h
  - [x] z.h
  - [x] AString.hpp
  - [x] Convert.hpp
  - [x] Index.hpp
  - [x] Map.hpp
  - [x] Other.hpp
  - [x] Tuple.h
  - [x] Utf.hpp
  - [x] Value.hpp
  - [x] Xmlize.hpp
  - [x] InVector.hpp
  - [x] CharFilter.h
- [ ] Implement missing classes in stdsrc/Draw to match uppsrc/Draw functionality:
  - [ ] Cham.h
  - [ ] DDARasterizer.h
  - [ ] Display.h (Note: different from CtrlCore Display.h)
  - [ ] DrawUtil.h
  - [ ] ImageOp.h
  - [x] Iml.h
  - [ ] Palette.h
  - [ ] Raster.h
  - [ ] SDraw.h
  - [ ] SIMD.h
  - [ ] Uhd.h
  - [ ] Drawing.h
  - [ ] DrawRasterData.h
  - [ ] DrawText.h
  - [ ] DrawTextUtil.h
  - [ ] ImageAnyDraw.h
  - [ ] ImageBlit.h
  - [ ] ImageChOp.h
  - [ ] ImageScale.h
  - [ ] MakeCache.h
  - [ ] Mify.h
  - [ ] RasterEncoder.h
  - [ ] RasterFormat.h
  - [ ] RasterWrite.h
  - [ ] RescaleFilter.h
  - [ ] SColors.h
  - [ ] SDrawClip.h
  - [ ] SDrawPut.h
  - [ ] SDrawText.h
  - [ ] SImageDraw.h
- [ ] Implement missing classes in stdsrc/CtrlCore to match uppsrc/CtrlCore functionality:
  - [ ] CtrlAttr.h
  - [ ] CtrlChild.h
  - [ ] CtrlClip.h
  - [ ] CtrlDraw.h
  - [ ] CtrlFrame.h
  - [ ] CtrlKbd.h
  - [ ] CtrlMouse.h
  - [ ] CtrlMt.h
  - [ ] CtrlPos.h
  - [ ] CtrlTimer.h
  - [ ] DHCtrl.h
  - [ ] Frame.h
  - [ ] SystemDraw.h
  - [ ] Util.h
  - [ ] MKeys.h
  - [ ] stdids.h
  - [ ] LocalLoop.h
  - [ ] MetaFile.h
  - [ ] EncodeRTF.h
  - [ ] ParseRTF.h
- [ ] Implement missing classes in stdsrc/CtrlLib to match uppsrc/CtrlLib functionality:
  - [x] Splitter.h - Implemented
  - [x] ScrollBar.h - Implemented
  - [x] SliderCtrl.h - Implemented
  - [ ] Bar.h
  - [ ] Ch.h
  - [ ] ChatCtrl.h
  - [ ] ColorPopup.h
  - [ ] ColorPusher.h
  - [ ] ColumnList.h
  - [ ] DateTimeCtrl.h
  - [ ] DisplayPopup.h
  - [ ] DlgColor.h
  - [ ] DropChoice.h
  - [ ] DropList.h
  - [ ] DropTree.h
  - [ ] EditCtrl.h
  - [ ] FileList.h
  - [ ] FileSel.h
  - [ ] FrameSplitter.h
  - [ ] HeaderCtrl.h
  - [ ] LabelBase.h
  - [ ] LineEdit.h
  - [ ] MenuBar.h
  - [ ] MenuItem.h
  - [ ] MultiButton.h
  - [ ] PageCtrl.h
  - [ ] PopUpList.h
  - [ ] PopupTable.h
  - [ ] Progress.h
  - [ ] PushCtrl.h
  - [ ] RichText.h
  - [ ] RichTextView.h
  - [ ] Splitter.h (already implemented above)
  - [ ] ScrollBar.h (already implemented above)
  - [ ] SliderCtrl.h (already implemented above)
  - [ ] StaticCtrl.h
  - [ ] StatusBar.h
  - [ ] SuggestCtrl.h
  - [ ] TabCtrl.h
  - [ ] ToolBar.h
  - [ ] ToolButton.h
  - [ ] TreeCtrl.h
  - [ ] TimelineCtrl.h

## TODO

### IdeDropdownTerminal Enhancement Tasks
- [ ] Create and implement KillCaret and SetCaret functions that are missing from TabbedTerminalExample
- [ ] Compile TabbedTerminalExample using script/build_tabbed_terminal_example.sh and fix all errors
- [ ] Fork uppsrc/TabbedTerminalExample to IdeDropdownTerminal or related classes
- [ ] Make the dropdown terminal work with system bash initially
- [ ] Later bind the terminal to internal shell (builtin), like busybox bash
- [ ] Investigate and fix the IdeDropdownTerminal crash when pressing toggle visibility button
- [ ] Implement proper visibility options using native hide/show instead of destroy/create window
- [ ] Ensure dropdown terminal is removed from taskbar when hidden
- [ ] Add semi-transparency to dropdown terminal window
- [ ] Implement partial window transparency (only terminal area) if possible

### V4L Webcam Support Improvements
- [ ] Enhance current V4L2 implementation in Media module with better device enumeration
- [ ] Add support for additional V4L2 device properties and controls
- [ ] Implement more robust error handling for V4L2 device access
- [ ] Add support for more V4L2 pixel formats
- [ ] Improve performance with better buffer management

### Physics Wrapper for ODE
- [ ] Complete unfinished Physics wrapper functionality for ODE
- [ ] Add support for advanced physics features (constraints, joints, collision detection)
- [ ] Implement physics prefab system improvements
- [ ] Add more physics primitive types (mesh colliders, compound shapes)
- [ ] Test and validate physics simulation accuracy

### Graphics API Enhancements
- [ ] Implement DirectX support (DX11/DX12) for cross-platform graphics
- [ ] Add Vulkan support for high-performance graphics rendering
- [ ] Add Metal support for macOS graphics acceleration
- [ ] Implement advanced shader support and rendering pipelines
- [ ] Add compute shader support for GPGPU operations

### Screen API Improvements
- [ ] Add basic Windows OpenGL screen support for Screen module
- [ ] Implement proper context management for OpenGL contexts
- [ ] Add support for multiple screen configurations
- [ ] Enhance cross-platform window management

### Media API Improvements
- [ ] Add comprehensive gstreamer support for advanced media handling
- [ ] Implement better audio/video synchronization
- [ ] Add support for additional media formats and codecs
- [ ] Enhance streaming capabilities for real-time processing
- [ ] Add media filtering and processing capabilities

### Holograph API Updates
- [ ] Update OpenHMD support to latest version
- [ ] Enhance OpenVR support for newer SDK versions
- [ ] Add OpenXR support for cross-platform VR
- [ ] Implement support for newer VR devices
- [ ] Add AR (Augmented Reality) support

### Hal API Extensions
- [ ] Add SDL3 support for modern input handling
- [ ] Implement SDL1 support for legacy compatibility
- [ ] Add other similar libraries (Allegro, GLFW) for broader hardware abstraction
- [ ] Enhance input device detection and handling
- [ ] Add gamepad/controller support improvements

### Audio API Expansions
- [ ] Add LV2 plugin support for audio effects
- [ ] Implement LADSPA plugin support for audio processing
- [ ] Add CLAP plugin support for modern audio plugins
- [ ] Add VST plugin support for professional audio workflows
- [ ] Enhance AudioHost/Audio/Effects modules with plugin architecture

### Volumetric API Improvements
- [ ] Add support for point cloud libraries (PCL, etc.)
- [ ] Implement point cloud processing and visualization features
- [ ] Add support for 3D scanning data formats
- [ ] Enhance volumetric rendering capabilities

### MidiHw API Enhancements
- [ ] Add native MIDI support for Windows
- [ ] Implement native MIDI support for Linux (ALSA)
- [ ] Add native MIDI support for macOS (CoreMIDI)
- [ ] Enhance MIDI device enumeration and management

### Additional New API Modules to Consider
- [ ] WebRTC API for real-time communication
- [ ] WebGPU API for next-generation GPU access (run script/build_apigen.sh)
- [ ] Neural network/machine learning API wrapper (TensorFlow, PyTorch)
- [ ] WebAssembly runtime API
- [ ] Bluetooth/Bluetooth LE API
- [ ] Network protocols API (WebSocket, etc.)

### Eon Project Conversions
- [ ] Create C++ stubs for Eon01 (event state, MIDI events, meta tests)
- [ ] Create C++ stubs for Eon02
- [ ] Create C++ stubs for Eon03
- [ ] Create C++ stubs for Eon04
- [ ] Create C++ stubs for Eon05
- [ ] Create C++ stubs for Eon06
- [ ] Create C++ stubs for Eon07 (ECS features)
- [ ] Create C++ stubs for Eon08 (GUI, 3D, VR)
- [ ] Add command-line startup for Eon01
- [ ] Add command-line startup for Eon02
- [ ] Add command-line startup for Eon03
- [ ] Add command-line startup for Eon04
- [ ] Add command-line startup for Eon05
- [ ] Add command-line startup for Eon06
- [ ] Add command-line startup for Eon07
- [ ] Add command-line startup for Eon08

### ECS Development
- [ ] Create "ECS Hello World" project with essential ECS features from Eon07
- [ ] Debug ECS usage in Eon07 (entities, components, systems)
- [ ] Test ECS rendering system functionality
- [ ] Validate ECS physics system integration
- [ ] Test ECS camera and viewport systems
- [ ] Verify ECS event handling

### Gubo (3D GUI) Development
- [ ] Implement basic 3D GUI controls in GuboCore
- [ ] Create 3D drawing primitives and functionality
- [ ] Implement 3D interaction systems (mouse, keyboard, touch)
- [ ] Add 3D UI component hierarchy system
- [ ] Implement 3D UI event handling
- [ ] Create 3D UI layout managers
- [ ] Add 3D UI rendering pipeline
- [ ] Implement 3D UI camera and projection systems
- [ ] Add 3D UI coordinate transformation utilities
- [ ] Create basic Gubo controls (buttons, sliders, etc.)
- [ ] Implement Gubo windows and dialogs
- [ ] Add Gubo drawing commands and rendering
- [ ] Create Gubo surface and frame management
- [ ] Implement Gubo top-level window system
- [ ] Add Gubo animation and effects
- [ ] Create Gubo event system for 3D interactions

### SoftAudio Library Enhancements
- [ ] Add more audio effect algorithms (reverb, delay, distortion)
- [ ] Implement audio synthesis improvements
- [ ] Add support for more audio formats
- [ ] Enhance audio graph system
- [ ] Improve audio performance and optimization
- [ ] Add real-time audio processing features
- [ ] Implement audio buffer management improvements
- [ ] Add MIDI to audio conversion utilities
- [ ] Create audio visualization components
- [ ] Add audio streaming capabilities
- [ ] Implement advanced filtering algorithms
- [ ] Add audio analysis and metering tools
- [ ] Create audio plugin architecture
- [ ] Add support for more audio file formats
- [ ] Enhance audio synchronization capabilities

### SoftRend Library Enhancements
- [ ] Implement complete software rendering pipeline
- [ ] Add shader compilation and execution support
- [ ] Enhance framebuffer and rendering target management
- [ ] Implement advanced pipeline features
- [ ] Add support for different rendering modes
- [ ] Optimize software rendering performance
- [ ] Add support for texture mapping
- [ ] Implement lighting and shading calculations
- [ ] Add support for various vertex formats
- [ ] Create rendering state management
- [ ] Implement rendering program system
- [ ] Add utility programs for common rendering tasks
- [ ] Create shader compilation utilities
- [ ] Add support for different rendering backends
- [ ] Implement rendering memory management

### Other Soft Libraries
- [ ] Implement SoftHMD (Head-Mounted Display) functionality
- [ ] Develop SoftInstru (Instruments) system
- [ ] Enhance SoftPhys (Physics) simulation capabilities
- [ ] Create SoftSynth (Synthesizer) components
- [ ] Develop SoftVR (Virtual Reality) features
- [ ] Add 3D audio positioning in SoftAudio
- [ ] Implement physics constraints in SoftPhys
- [ ] Add VR interaction in SoftVR
- [ ] Create audio synthesis in SoftSynth
- [ ] Add HMD tracking in SoftHMD
- [ ] Implement instrument mapping in SoftInstru

### Standard Library Wrapper Development
- [ ] Implement wrapper library in stdsrc using STL std c++ libraries to implement U++ Core functions
- [ ] Create stdsrc/Draw wrapper for graphics functionality (WXWidgets/Gtk/Qt native implementations)
- [ ] Create stdsrc/CtrlCore wrapper for core UI functionality (WXWidgets/Gtk/Qt native implementations) 
- [ ] Create stdsrc/CtrlLib wrapper for UI controls (WXWidgets/Gtk/Qt native implementations)
- [ ] Support native platform APIs: Windows, macOS, and other platform-specific implementations
- [ ] Ensure comprehensive stdtst package coverage for all wrapper library features

### stdinc Directory Implementation
- [ ] Create stdinc directory structure by reversing stdsrc content
- [ ] Implement standard STL c++ headers by reversing stdsrc implementation (super fast job)
- [ ] Create wrapper layer that allows STL code to compile using stdinc + stdsrc
- [ ] Ensure stdinc + stdsrc still uses system's STL with U++ wrapper in middle
- [ ] Test stdinc headers with sample STL code and add more features as needed
- [ ] Add comprehensive tests for stdinc functionality in stdtst package

### Code Translation and Conversion Tasks
- [ ] Research and implement clang AST parsing for STL code analysis
- [ ] Develop system to analyze how STL projects use U++ via stdinc headers
- [ ] Create conversion tool to transform STL calls to U++ calls in low-level format
- [ ] Generate new project files that use U++ directly instead of STL
- [ ] Develop reverse conversion system: convert U++ files to use STL cleanly
- [ ] Test conversion tools on real-world STL and U++ projects
- [ ] Optimize conversion process for efficiency and correctness
- [ ] Create validation suite to verify converted code functionality

### VR ECS Engine Enhancement
- [ ] Convert Eon/Win VR ECS engine to work with OpenVR in addition to current WinRT implementation
- [ ] Add OpenHMD support to Eon/Win VR ECS engine
- [ ] Address WinRT limitations for UWP (Universal Windows Platform) compatibility
- [ ] Remove filesystem function restrictions to enable normal file operations in UWP
- [ ] Create CMake files for Visual Studio compilation support
- [ ] Maintain existing functionality while adding cross-platform VR support

### Eon/GuiGlue Integration
- [ ] Enable Eon/GuiGlue to work with U++ GUI components
- [ ] Bind U++ TopWindow to Eon engine for seamless integration
- [ ] Create proper interface layer between U++ windows and ECS systems
- [ ] Ensure event handling flows correctly between U++ GUI and Eon engine
- [ ] Maintain compatibility with existing Eon functionality

### Desktop Suite Development (LOWEST PRIORITY)
- [ ] Develop uppsrc/DesktopSuite applications for X11 desktop programs following FreeDesktop specifications
- [ ] Implement window manager component for DesktopSuite
- [ ] Create login manager component for DesktopSuite
- [ ] Develop task bar component for DesktopSuite
- [ ] Create control panel component for DesktopSuite
- [ ] Implement file manager component for DesktopSuite
- [ ] Develop session manager component for DesktopSuite
- [ ] Ensure compliance with FreeDesktop standards and specifications
- [ ] Support X11 windowing system integration

### Geometry Library Improvements
- [ ] Add missing mathematical functions and operations
- [ ] Implement missing matrix operations and transformations
- [ ] Add more geometric primitives and shapes
- [ ] Improve 3D vector operations
- [ ] Add quaternion operations and utilities
- [ ] Enhance camera system with more features
- [ ] Implement advanced frustum culling
- [ ] Add more mesh operations and utilities
- [ ] Improve model loading and processing
- [ ] Add better material and texture support
- [ ] Implement more efficient spatial partitioning (Octree, Quadtree)
- [ ] Add geometric intersection algorithms
- [ ] Improve 3D transformation utilities
- [ ] Add geometric projection and unprojection functions
- [ ] Implement more efficient mesh processing
- [x] Address issues in TODO.txt: shader components registration
- [x] Address issues in TODO.txt: data transfer to/from pipeline
- [x] Address issues in TODO.txt: GPU pipeline initialization
- [x] Optimize vertex data reuse in rendering
- [x] Add support for different index formats (1-byte indices)
- [x] Implement mesh splitting for large models
- [x] Add shader compilation flags and optimization settings



### Vfs Overlay Implementation
- [x] Implement SourceRef structure for tracking provenance (package hash, file hash, local path, priority, flags)
- [x] Implement OverlayView interface for virtual merge of per-file trees
- [x] Implement VfsOverlay class representing single source fragments
- [x] Implement OverlayManager for combining overlays with precedence
- [x] Integrate overlay system with MetaEnvironment
- [x] Implement precedence provider interfaces
- [x] Add overlay listing and merged value retrieval methods
- [x] Add overlay registration functionality to MetaEnvironment

## DONE

- [x] Explored uppsrc/api/ directory structure
- [x] Mapped current API components (Audio, Camera, Graphics, Hal, Holograph, Media, MidiHw, Physics, Screen, Volumetric, etc.)
- [x] Analyzed EonApiEditor package structure and functionality
- [x] Identified improvement opportunities across all API modules
- [x] Documented API architecture and vendor implementation patterns
- [x] Explored upptst/Eon* projects and compared Eon00 conversion
- [x] Identified ECS features in Eon07 for debugging
- [x] Examined GuboCore and GuboLib structure
- [x] Investigated Soft* libraries structure
- [x] Analyzed Geometry library for quality issues