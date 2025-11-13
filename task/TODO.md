# TODO Thread

**Goal**: Collection of future enhancement tasks and feature additions

## Status: BACKLOG

---

## IdeDropdownTerminal Enhancement Tasks
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

---

## V4L Webcam Support Improvements
- [ ] Enhance current V4L2 implementation in Media module with better device enumeration
- [ ] Add support for additional V4L2 device properties and controls
- [ ] Implement more robust error handling for V4L2 device access
- [ ] Add support for more V4L2 pixel formats
- [ ] Improve performance with better buffer management

---

## Physics Wrapper for ODE
- [ ] Complete unfinished Physics wrapper functionality for ODE
- [ ] Add support for advanced physics features (constraints, joints, collision detection)
- [ ] Implement physics prefab system improvements
- [ ] Add more physics primitive types (mesh colliders, compound shapes)
- [ ] Test and validate physics simulation accuracy

---

## Graphics API Enhancements
- [ ] Implement DirectX support (DX11/DX12) for cross-platform graphics
- [ ] Add Vulkan support for high-performance graphics rendering
- [ ] Add Metal support for macOS graphics acceleration
- [ ] Implement advanced shader support and rendering pipelines
- [ ] Add compute shader support for GPGPU operations

---

## Screen API Improvements
- [ ] Add basic Windows OpenGL screen support for Screen module
- [ ] Implement proper context management for OpenGL contexts
- [ ] Add support for multiple screen configurations
- [ ] Enhance cross-platform window management

---

## Media API Improvements
- [ ] Add comprehensive gstreamer support for advanced media handling
- [ ] Implement better audio/video synchronization
- [ ] Add support for additional media formats and codecs
- [ ] Enhance streaming capabilities for real-time processing
- [ ] Add media filtering and processing capabilities

---

## Holograph API Updates
- [ ] Update OpenHMD support to latest version
- [ ] Enhance OpenVR support for newer SDK versions
- [ ] Add OpenXR support for cross-platform VR
- [ ] Implement support for newer VR devices
- [ ] Add AR (Augmented Reality) support

---

## Hal API Extensions
- [ ] Add SDL3 support for modern input handling
- [ ] Implement SDL1 support for legacy compatibility
- [ ] Add other similar libraries (Allegro, GLFW) for broader hardware abstraction
- [ ] Enhance input device detection and handling
- [ ] Add gamepad/controller support improvements

---

## Audio API Expansions
- [ ] Add LV2 plugin support for audio effects
- [ ] Implement LADSPA plugin support for audio processing
- [ ] Add CLAP plugin support for modern audio plugins
- [ ] Add VST plugin support for professional audio workflows
- [ ] Enhance AudioHost/Audio/Effects modules with plugin architecture

---

## Volumetric API Improvements
- [ ] Add support for point cloud libraries (PCL, etc.)
- [ ] Implement point cloud processing and visualization features
- [ ] Add support for 3D scanning data formats
- [ ] Enhance volumetric rendering capabilities

---

## MidiHw API Enhancements
- [ ] Add native MIDI support for Windows
- [ ] Implement native MIDI support for Linux (ALSA)
- [ ] Add native MIDI support for macOS (CoreMIDI)
- [ ] Enhance MIDI device enumeration and management

---

## Additional New API Modules to Consider
- [ ] WebRTC API for real-time communication
- [ ] WebGPU API for next-generation GPU access (run script/build_apigen.sh)
- [ ] Neural network/machine learning API wrapper (TensorFlow, PyTorch)
- [ ] WebAssembly runtime API
- [ ] Bluetooth/Bluetooth LE API
- [ ] Network protocols API (WebSocket, etc.)

---

## Eon Project Conversions
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

---

## ECS Development
- [ ] Create "ECS Hello World" project with essential ECS features from Eon07
- [ ] Debug ECS usage in Eon07 (entities, components, systems)
- [ ] Test ECS rendering system functionality
- [ ] Validate ECS physics system integration
- [ ] Test ECS camera and viewport systems
- [ ] Verify ECS event handling

---

## Gubo (3D GUI) Development
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

---

## SoftAudio Library Enhancements
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

---

## SoftRend Library Enhancements
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

---

## Other Soft Libraries
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

---

## Standard Library Wrapper Development
- [ ] Create stdsrc/Draw wrapper for graphics functionality (WXWidgets/Gtk/Qt native implementations)
- [ ] Create stdsrc/CtrlCore wrapper for core UI functionality (WXWidgets/Gtk/Qt native implementations)
- [ ] Create stdsrc/CtrlLib wrapper for UI controls (WXWidgets/Gtk/Qt native implementations)
- [ ] Support native platform APIs: Windows, macOS, and other platform-specific implementations

---

## stdinc Directory Implementation
- [ ] Create stdinc directory structure by reversing stdsrc content
- [ ] Implement standard STL c++ headers by reversing stdsrc implementation (super fast job)
- [ ] Create wrapper layer that allows STL code to compile using stdinc + stdsrc
- [ ] Ensure stdinc + stdsrc still uses system's STL with U++ wrapper in middle
- [ ] Test stdinc headers with sample STL code and add more features as needed
- [ ] Add comprehensive tests for stdinc functionality in stdtst package

---

## Code Translation and Conversion Tasks
- [ ] Research and implement clang AST parsing for STL code analysis
- [ ] Develop system to analyze how STL projects use U++ via stdinc headers
- [ ] Create conversion tool to transform STL calls to U++ calls in low-level format
- [ ] Generate new project files that use U++ directly instead of STL
- [ ] Develop reverse conversion system: convert U++ files to use STL cleanly
- [ ] Test conversion tools on real-world STL and U++ projects
- [ ] Optimize conversion process for efficiency and correctness
- [ ] Create validation suite to verify converted code functionality

---

## VR ECS Engine Enhancement
- [ ] Convert Eon/Win VR ECS engine to work with OpenVR in addition to current WinRT implementation
- [ ] Add OpenHMD support to Eon/Win VR ECS engine
- [ ] Address WinRT limitations for UWP (Universal Windows Platform) compatibility
- [ ] Remove filesystem function restrictions to enable normal file operations in UWP
- [ ] Create CMake files for Visual Studio compilation support
- [ ] Maintain existing functionality while adding cross-platform VR support

---

## Eon/GuiGlue Integration
- [ ] Enable Eon/GuiGlue to work with U++ GUI components
- [ ] Bind U++ TopWindow to Eon engine for seamless integration
- [ ] Create proper interface layer between U++ windows and ECS systems
- [ ] Ensure event handling flows correctly between U++ GUI and Eon engine
- [ ] Maintain compatibility with existing Eon functionality

---

## Desktop Suite Development (LOWEST PRIORITY)
- [ ] Develop uppsrc/DesktopSuite applications for X11 desktop programs following FreeDesktop specifications
- [ ] Implement window manager component for DesktopSuite
- [ ] Create login manager component for DesktopSuite
- [ ] Develop task bar component for DesktopSuite
- [ ] Create control panel component for DesktopSuite
- [ ] Implement file manager component for DesktopSuite
- [ ] Develop session manager component for DesktopSuite
- [ ] Ensure compliance with FreeDesktop standards and specifications
- [ ] Support X11 windowing system integration

---

## Geometry Library Improvements
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

---

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
- [ ] Create entity-component-system (ECS) framework in GameEngine (using existing ai-upp ECS code after upptst/Eon* packages work properly)
- [x] Add support for asset management and resource loading
- [ ] Implement basic 2D and 3D rendering pipelines
- [ ] Create camera system with support for different projection types
- [ ] Develop input handling system unified across different platforms
- [ ] Add audio integration using uppsrc/api/Audio package
- [ ] Implement basic physics integration using uppsrc/api/Physics package
- [x] Design scene management system
- [ ] Create basic UI/HUD rendering capabilities
- [ ] Add cross-platform file system abstraction for game assets
- [ ] Implement basic animation system
- [ ] Design resource management with proper memory handling
- [ ] Ensure compatibility with existing U++/Eon architecture
- [ ] Write comprehensive tests for GameLib and GameEngine packages
- [ ] Document API design and usage patterns for game development

---

## Dependencies
- Requires: Various (depends on specific task)
- Blocks: None (backlog items)
- Related: All other threads (enhancement and expansion items)
