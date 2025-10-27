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
- [x] Keep working on wrapper library in stdsrc that implements U++ Core functions using STL std c++ libraries
- [x] Implement stdsrc/{Draw, CtrlCore, CtrlLib} wrapper libraries for WXWidgets/Gtk/Qt and native platform APIs
- [x] Update stdtst packages to test all wrapper library features comprehensively
- [x] Convert Eon/Win VR ECS engine to work with OpenVR and OpenHMD in addition to current WinRT implementation
- [x] Address WinRT limitations and ensure UWP (Universal Windows Platform) compatibility for Eon/Win
- [x] Create CMake files to enable Visual Studio compilation for Eon/Win project
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