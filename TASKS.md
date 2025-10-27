# TASKS.md

## IN PROGRESS

- [ ] CRITICAL: Analyze and implement correct overlay architecture: MetaEnvironment needs VirtualNode root that navigates multiple VfsOverlay instances per file
- [ ] CRITICAL: Each VfsOverlay must contain VfsValue root for individual files only, not MetaEnvironment global root
- [ ] CRITICAL: Implement VirtualNode-to-Overlays navigation system for unified view across all active overlays
- [ ] CRITICAL: Maintain backward compatibility while restructuring MetaEnvironment from single VfsValue root to overlay-based system
- [ ] CRITICAL: Ensure file-level isolation where each .cpp/.h file has its own overlay with its AST data
- [ ] CRITICAL: Ensure VfsValue::file_hash matches Overlay's file_hash for proper correlation
- [ ] CRITICAL: Update serial handling - each overlay tracks rolling revision numbers (serial), not global system
- [ ] CRITICAL: Implement overlay serial tracking system to maintain comparison capabilities between overlays

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
