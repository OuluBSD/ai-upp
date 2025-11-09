# TASKS.md

## IN PROGRESS

### THREAD: GraphLib Node Editor Features
**Goal**: Enhance GraphLib package with advanced node editor features based on imgui-node-editor pseudocode
**Features to implement**: Node pin system, interactive editing, visual feedback, animations, context menus, and clipboard operations

#### Phase 1: Core Node Editor Components
- [x] Add Pin support to Nodes
  - [x] Extend Node class to support input/output pins
  - [x] Create Pin class with position, type, and connection validation  
  - [x] Implement pin rendering in the renderer
- [x] Enhance Edge/Link System
  - [x] Modify edges to connect from pin to pin instead of node to node
  - [x] Add bezier curve rendering for aesthetic connections
  - [x] Implement link creation workflow (drag from pin to pin)
- [x] Implement Node Groups
  - [x] Add group node type that can contain other nodes
  - [x] Implement group bounding box and header rendering
  - [x] Add group sizing and positioning logic

#### Phase 2: Interactive Editing Features
- [x] Add Interactive Editing Features
  - [x] Implement node creation/deletion via UI
  - [x] Add link creation/deletion functionality
  - [x] Create node/link selection system
- [x] Enhance UI Interaction System
  - [x] Add drag and drop for nodes
  - [x] Implement box selection for multiple nodes/links
  - [x] Add keyboard shortcuts (Ctrl+C, Ctrl+V, etc.)
- [x] Add Visual Feedback System
  - [x] Implement selection highlighting
  - [x] Add visual feedback during link creation
  - [x] Create hover effects for nodes and links

#### Phase 3: Advanced Features
- [x] Enhance Navigation System
  - [x] Add smooth zooming/panning
  - [x] Implement navigation animations
  - [x] Add focus on selected elements
- [x] Add Context Menus
  - [x] Implement node context menus
  - [x] Add link context menus
  - [x] Create background context menu
- [x] Implement Animation System
  - [x] Add link flow animations
  - [x] Implement node movement animations
  - [x] Add navigation animations

#### Phase 4: Productivity Features
- [x] Add Clipboard Operations
  - [x] Implement cut/copy/paste functionality
  - [x] Add node duplication feature
- [x] Settings Persistence
  - [x] Implement node position saving/loading
  - [x] Add editor state persistence
- [x] Update Existing Layout Algorithms
  - [x] Adapt algorithms to work with new pin-based system
  - [x] Ensure backward compatibility with current features

#### Reference Materials
- [x] Use ~/Dev/shadertoy/pseudocode/src/thirdparty_imgui-node-editor/ as reference implementation
- [x] Copy and analyze GraphLib tutorial examples (GraphLib1-4) for reference design patterns

### THREAD: ShaderToy
**Goal**: Implement ShaderToy package that uses the enhanced GraphLib node editor to create and manipulate shader pipelines
**Features to implement**: Complete shader pipeline editor with node-based editing, texture handling, rendering pipeline, STTF import/export, and shader toy compatibility

#### Phase 1: Core Pipeline Editor Components
- [x] Create Pipeline Editor Structure
  - [x] Implement main PipelineEditor class based on shadertoy NodeEditor
  - [x] Create EditorNode base class and derived node types (EditorShader, EditorTexture, EditorCubeMap, etc.)
  - [x] Implement pin connections and link management for shader pipelines
- [x] Node Types Implementation
  - [x] EditorShader node with GLSL code editor
  - [x] EditorTexture node for texture inputs
  - [x] EditorCubeMap node for cubemap textures
  - [x] EditorVolume node for 3D textures
  - [x] EditorKeyboard node for keyboard input
  - [x] EditorRenderOutput node for pipeline output
  - [x] EditorLastFrame node for frame feedback
- [x] Pin and Connection System
  - [x] Implement specialized pin types for shader connections (UV coordinates, channels, etc.)
  - [x] Create connection validation logic for shader graph
  - [x] Implement link creation/deletion workflow specific to shader pipelines

#### Phase 2: Rendering and Pipeline System
- [ ] Backend Implementation
  - [ ] Implement OpenGL backend based on shadertoy/Backend.hpp
  - [ ] Create shader compilation and linking system
  - [ ] Implement rendering pipeline with node connections
- [ ] Pipeline Building
  - [ ] Create pipeline builder that translates node graph to render sequence
  - [ ] Implement node evaluation order calculation
  - [ ] Add error handling for shader compilation failures
- [ ] Texture and Resource Management
  - [ ] Implement texture loading and management system
  - [ ] Add support for different texture formats and types
  - [ ] Create resource cleanup system

#### Phase 3: UI and Interaction Features
- [ ] Shader Code Editor
  - [ ] Integrate text editor with GLSL syntax highlighting
  - [ ] Add shader compilation feedback in UI
  - [ ] Implement live shader preview
- [ ] Node Content Rendering
  - [ ] Implement renderContent methods for each node type
  - [ ] Create texture preview for texture nodes
  - [ ] Add keyboard visualization for keyboard nodes
- [ ] Specialized UI Elements
  - [ ] Implement custom drawing functions for node icons
  - [ ] Create context menus for different node types
  - [ ] Add keyboard input visualization

#### Phase 4: Import/Export and Compatibility
- [ ] STTF Format Support
  - [ ] Implement STTF (ShaderToy Transfer Format) parser
  - [ ] Create node serialization/deserialization for STTF
  - [ ] Add support for loading STTF files into the editor
- [ ] ShaderToy Compatibility
  - [ ] Implement ShaderToy project import functionality
  - [ ] Map ShaderToy inputs to node-based equivalents
  - [ ] Convert ShaderToy expressions to node graph
- [ ] File Operations
  - [ ] Add STTF save functionality
  - [ ] Create project file management system
  - [ ] Add export pipeline to different formats

#### Phase 5: Advanced Features
- [ ] Layout and Organization
  - [ ] Implement automatic layout algorithms for node graphs
  - [ ] Add node grouping and organization features
  - [ ] Create node search and filtering system
- [ ] Performance Optimization
  - [ ] Implement pipeline caching system
  - [ ] Add performance monitoring and debugging tools
  - [ ] Optimize rendering for complex node graphs
- [ ] Extension and Plugin System
  - [ ] Create API for custom node types
  - [ ] Implement plugin system for extending functionality
  - [ ] Add example custom nodes for demonstration

#### Reference Materials
- [ ] Use ~/Dev/shadertoy/pseudocode/src/shadertoy/ as reference implementation
- [ ] Study imgui-node-editor integration patterns
- [ ] Analyze STTF format specifications from reference materials

### THREAD: vfs-ast-fix (rename to eon-tests later)
**Goal**: Fix VFS tree structure and get all upptst/Eon* tests running properly with correct VFS-AST
**Problem**: Program was running but with wrong VFS-AST structure (drivers nested inside loops instead of siblings)

#### Current Work: Eon03 VFS-AST Fix
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
  - [ ] Test 03n (Win32 video)

#### Eon03 Known Issues to Fix
Note: Eon03 builds with "script/build_upptst_eon03.sh" and runs with "bin/Eon03". Test 03a runs with "bin/Eon03 0 0" and test 03b with "bin/Eon03 1 0" etc.
- [x] Fix framerate issues in tests 03g and 03i
  - [x] Add FPS printing to X11 stop or uninitialize functions
  - [x] Added comprehensive packet profiling to PacketTracker
  - [x] Per-atom buffer profiling shows PollerLink.sink[0] wait time: 767ms avg in linked vs 125ms in baseline
  - [x] Confirmed: Framerate is low due to debug mode (-O0) with intensive fragment shaders - expected behavior
  - [x] Packet profiling tool completed: tracks buffer residence time, exchange timing, and packet formats
- [x] Fix stereo rendering in tests 03k and 03l ✓ FIXED
  - [x] Packet profiling shows normal packet flow (4 packets, all buffers <0.2ms, exchange 7.6ms avg)
  - [x] Confirmed: Issue is NOT in packet/pipeline system
  - [x] Root cause #1: ObjViewProg.cpp had hardcoded static stereo camera positions
    - Stereo eyes were at fixed positions instead of following animated camera
    - Fixed: Calculate eye offsets relative to animated camera position using perpendicular offset
  - [x] Root cause #2: TBufferStage.cpp SetStereo() bug caused right eye to use mono rendering
    - SetStereo(1) set flag on fb[1] but Process() checks fb[0]
    - Right eye rendered with wrong viewport (mono instead of stereo aspect ratio)
    - Fixed: Always set flags on fb[0] which is actually used by rendering
  - [x] Both eyes now animate correctly with matching aspect ratios
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

- [ ] Implement missing classes in stdsrc/Core to match uppsrc/Core functionality:
  - All currently required Core classes have been implemented
- [ ] Implement missing classes in stdsrc/Draw to match uppsrc/Draw functionality:
  - [ ] Cham.h
  - [ ] DDARasterizer.h
  - [ ] Display.h (Note: different from CtrlCore Display.h)
  - [ ] DrawUtil.h
  - [ ] ImageOp.h
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

## DONE

### VFS Overlay Architecture (CRITICAL)
- [x] CRITICAL: Analyze and implement correct overlay architecture: MetaEnvironment needs VirtualNode root that navigates multiple VfsOverlay instances per file
- [x] CRITICAL: Each VfsOverlay must contain VfsValue root for individual files only, not MetaEnvironment global root
- [x] CRITICAL: Implement VirtualNode-to-Overlays navigation system for unified view across all active overlays
- [x] CRITICAL: Maintain backward compatibility while restructuring MetaEnvironment from single VfsValue root to overlay-based system
- [x] CRITICAL: Ensure file-level isolation where each .cpp/.h file has its own overlay with its AST data
- [x] CRITICAL: Ensure VfsValue::file_hash matches Overlay's file_hash for proper correlation
- [x] CRITICAL: Update serial handling - each overlay tracks rolling revision numbers (serial), not global system
- [x] CRITICAL: Implement overlay serial tracking system to maintain comparison capabilities between overlays

### VfsShell Implementation
- [ ] Create CLI using U++ conventions similar to ~/Dev/VfsBoot/src/VfsShell/
- [ ] Use uppsrc/ide package handling instead of "~/Dev/VfsBoot/src/VfsShell/upp*" code
- [ ] Use uppsrc/Vfs instead of "~/Dev/VfsBoot/src/VfsShell/vfs*" code
- [ ] Implement pwd command
- [ ] Implement cd [path] command
- [ ] Implement ls [path] command
- [ ] Implement tree [path] command
- [ ] Implement mkdir <path> command
- [ ] Implement touch <path> command
- [ ] Implement rm <path> command
- [ ] Implement mv <src> <dst> command
- [ ] Implement link <src> <dst> command
- [ ] Implement export <vfs> <host> command
- [ ] Implement cat [paths...] command (including stdin if no paths)
- [ ] Implement grep [-i] <pattern> [path] command
- [ ] Implement rg [-i] <pattern> [path] command
- [ ] Implement head [-n N] [path] command
- [ ] Implement tail [-n N] [path] command
- [ ] Implement uniq [path] command
- [ ] Implement count [path] command
- [ ] Implement history [-a | -n N] command
- [ ] Implement random [min [max]] command
- [ ] Implement true / false commands
- [ ] Implement echo <path> <data...> command

### Vfs Overlay Implementation
- [x] Implement SourceRef structure for tracking provenance (package hash, file hash, local path, priority, flags)
- [x] Implement OverlayView interface for virtual merge of per-file trees
- [x] Implement VfsOverlay class representing single source fragments
- [x] Implement OverlayManager for combining overlays with precedence
- [x] Integrate overlay system with MetaEnvironment
- [x] Implement precedence provider interfaces
- [x] Add overlay listing and merged value retrieval methods
- [x] Add overlay registration functionality to MetaEnvironment

### Eon03 VFS-AST Fix
- [x] Fixed VFS tree ordering: Changed from DriverBeforeLoopLess to LoopBeforeDriverLess
- [x] Fixed remapping logic to only nest when driver is complete prefix of loop path
- [x] Enhanced FindOwnerWithCastDeep to search sibling containers
- [x] Added error checking for AddAtom and AddLoop failures
- [x] Fixed infinite loop in address replacement code in test harness

### Eon03 Tests
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

### Standard Library Wrapper (stdsrc)
- [x] Keep working on wrapper library in stdsrc that implements U++ Core functions using STL std c++ libraries
- [x] Implement stdsrc/{Draw, CtrlCore, CtrlLib} wrapper libraries for WXWidgets/Gtk/Qt and native platform APIs
- [x] Update stdtst packages to test all wrapper library features comprehensively
- [x] Convert Eon/Win VR ECS engine to work with OpenVR and OpenHMD in addition to current WinRT implementation
- [x] Address WinRT limitations and ensure UWP (Universal Windows Platform) compatibility for Eon/Win
- [x] Create CMake files to enable Visual Studio compilation for Eon/Win project
- [x] Implement all currently required classes in stdsrc/Core (Atomic.h, BiCont.h, BinUndoRedo.h, CharSet.h, CoAlgo.h, CoSort.h, CoWork.h, CritBitIndex.h, Cpu.h, Daemon.h, Debug.h, Diag.h, Dli.h, FileMapping.h, FilterStream.h, FixedMap.h, Fn.h, Heap.h, Huge.h, Inet.h, Ini.h, InMap.hpp, InVector.h, LinkedList.h, LocalProcess.h, Mt.h, NetNode.h, Ops.h, Other.h, PackedData.h, Parser.h, Random.h, Range.h, Recycler.h, Shared.h, SIMD.h, Socket.h, Sorted.h, Speller.h, SplitMerge.h, St.h, Topic.h, Topt.h, UnicodeInfo.h, Utf.h, ValueCache.h, ValueUtil.h, Vcont.hpp, WebSocket.h, Win32Util.h, Xmlize.h, xxHsh.h, z.h, AString.hpp, Convert.hpp, Index.hpp, Map.hpp, Other.hpp, Tuple.h, Utf.hpp, Value.hpp, Xmlize.hpp, InVector.hpp, CharFilter.h)
- [x] Implement Iml.h in stdsrc/Draw
- [x] Implement Splitter.h, ScrollBar.h, SliderCtrl.h in stdsrc/CtrlLib

### Geometry Library Improvements
- [x] Address issues in TODO.txt: shader components registration
- [x] Address issues in TODO.txt: data transfer to/from pipeline
- [x] Address issues in TODO.txt: GPU pipeline initialization
- [x] Optimize vertex data reuse in rendering
- [x] Add support for different index formats (1-byte indices)
- [x] Implement mesh splitting for large models
- [x] Add shader compilation flags and optimization settings

### General Project Analysis
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

## GameEngine Thread

### New Game Engine Library Development
- [ ] Create uppsrc/GameLib package for core game engine functionality
- [ ] Create uppsrc/GameEngine package for higher-level game engine features
- [ ] Integrate with existing uppsrc/Geometry for vectors, matrices, and 3D math operations
- [ ] Leverage uppsrc/api/Screen package for cross-platform windowing and input handling
- [ ] Integrate with uppsrc/api/Graphics package for rendering capabilities (OpenGL, DirectX, etc.)
- [ ] Use uppsrc/api/Hal package for hardware abstraction and input management
- [ ] Follow patterns from upptst/Eon{00,01,02,03,04,05} examples for proper eon code expansion
- [ ] Design GameWindow class as an easy-to-use solution for game windows
- [ ] Ensure GameWindow properly integrates with Screen and Graphics API packages
- [ ] Implement basic game loop architecture in GameEngine
- [ ] Create entity-component-system (ECS) framework in GameEngine
- [ ] Add support for asset management and resource loading
- [ ] Implement basic 2D and 3D rendering pipelines
- [ ] Create camera system with support for different projection types
- [ ] Develop input handling system unified across different platforms
- [ ] Add audio integration using uppsrc/api/Audio package
- [ ] Implement basic physics integration using uppsrc/api/Physics package
- [ ] Design scene management system
- [ ] Create basic UI/HUD rendering capabilities
- [ ] Add cross-platform file system abstraction for game assets
- [ ] Implement basic animation system
- [ ] Design resource management with proper memory handling
- [ ] Ensure compatibility with existing U++/Eon architecture
- [ ] Write comprehensive tests for GameLib and GameEngine packages
- [ ] Document API design and usage patterns for game development

### THREAD: Build and Test Node Editor
**Goal**: Create and maintain build script for GraphLib Node Editor with proper testing before committing changes

- [x] Use "script/build_node_editor.sh" to build and test before committing and marking as done
- [x] Verify Node Editor compiles without errors
- [x] Test all features work correctly before marking any feature as complete
- [x] Run regression tests to ensure no existing functionality was broken
- [x] Document any build issues and resolutions in the project documentation
