# Topside API Development Guide

## Overview

The Topside system uses a custom API wrapper system called "atoms" for the eon system that wraps various system components. The architecture is built around:

1. **uppsrc/api/** - Contains the API wrapper implementations
2. **uppsrc/EonApiEditor/** - Generates the binding code for new APIs

## API Architecture

### Directory Structure
```
uppsrc/api/
├── Audio/          - Audio processing and output
├── Camera/         - Camera and video capture
├── Graphics/       - Graphics rendering (OpenGL, DirectX, etc.)
├── Hal/            - Hardware abstraction layer
├── Holograph/      - VR/AR support (OpenHMD, OpenVR)
├── Media/          - Media processing and streaming
├── MidiHw/         - MIDI hardware support
├── Physics/        - Physics simulation (ODE)
├── Screen/         - Screen/window management
├── Synth/          - Synthesizer functions
├── Volumetric/     - 3D volumetric processing
└── .../
```

Each API module follows a consistent pattern:
- `.h` - Main header file
- `.upp` - U++ project configuration
- `IfaceFuncs.inl` - Interface function declarations
- Vendor-specific implementation files (e.g., `Portaudio.cpp`, `OpenHMD.cpp`)

### API Generation Process

The system uses a code generation approach:

1. **Define API in uppsrc/api/** - Implement vendor-specific code
2. **Register API in uppsrc/EonApiEditor/** - Add to interface builder
3. **Generate bindings** - Run EonApiEditor to create binding code
4. **Compile and use** - Generated code integrates with the system

### Interface Builder Pattern

In `uppsrc/EonApiEditor/EonApiEditor.cpp`, APIs are registered:

```cpp
// Example from Audio.cpp
void InterfaceBuilder::AddAudio() {
    Package("Audio", "Aud");           // API name and abbreviation
    SetColor(226, 212, 0);            // UI color
    Dependency("ParallelLib");        // Required dependencies
    Dependency("ports/portaudio", "BUILTIN_PORTAUDIO");
    Library("portaudio", "PORTAUDIO"); // Link libraries conditionally
    HaveNegotiateFormat();            // Feature flags
    
    Interface("SinkDevice");          // Define interfaces
    Interface("SourceDevice");
    
    Vendor("Portaudio", "BUILTIN_PORTAUDIO|PORTAUDIO"); // Implementation vendors
}
```

## Adding New APIs

### Step 1: Create API Definition in uppsrc/api/

1. Create a new directory: `uppsrc/api/MyApi/`
2. Add required files:
   - `MyApi.h` - Main header with declarations
   - `MyApi.upp` - U++ project configuration
   - `IfaceFuncs.inl` - Interface function declarations
   - Vendor implementation files (e.g., `MyImplementation.cpp`)

### Step 2: Define Interface in EonApiEditor

1. Add function declaration to `Interface.h`:
   ```cpp
   void AddMyApi();
   ```

2. Implement in new file `uppsrc/EonApiEditor/MyApi.cpp`:
   ```cpp
   #include "EonApiEditor.h"

   NAMESPACE_UPP

   void InterfaceBuilder::AddMyApi() {
       Package("MyApi", "MyA");
       SetColor(100, 150, 200);
       Dependency("ParallelLib");
       // Add dependencies and libraries as needed
       
       Interface("MyInterface");
       Vendor("MyImplementation", "MY_CONDITION");
   }

   END_UPP_NAMESPACE
   ```

3. Register in `uppsrc/EonApiEditor/main.cpp`:
   ```cpp
   ib.AddMyApi();
   ```

4. Add to the include list in `uppsrc/EonApiEditor/EonApiEditor.upp`:
   ```cpp
   file
       ...
       MyApi.cpp,
   ```

### Step 3: Generate Bindings

1. Compile `uppsrc/EonApiEditor/` project
2. Run the executable once - it generates:
   - API header files in appropriate directories
   - Interface function implementations
   - Parallel machine integration code
   - Registration code for the new atoms

### Step 4: Use the New API

The generated code allows creating atoms that use your new API with vendor-specific implementations.

## Code Generation Output

The EonApiEditor generates several types of files:

1. **API Headers** - Per-package headers with vendor implementations
2. **Interface Functions** - Function declarations for vendor implementations
3. **Parallel Machine Headers** - Integration with the parallel processing system
4. **Atom Registration** - Registration of new atom types in the system

## Best Practices

1. **Consistent Naming**: Use the established naming patterns (e.g., `PackageNameVendorInterface`)
2. **Conditional Compilation**: Use flag macros for platform-specific code
3. **Memory Management**: Follow RAII patterns for resource management
4. **Error Handling**: Implement proper error checking and logging
5. **Documentation**: Document vendor-specific behavior and limitations
6. **Task Management**: Don't necessarily complete one task entirely before starting another; move tasks to IN PROGRESS section when intermediate goals are achieved, allowing for iterative progress and parallel development on related features

## Vendor Implementation Guidelines

Each vendor file (e.g., `Portaudio.cpp`, `OpenHMD.cpp`) must implement:

- `Create`/`Destroy` functions for resource management
- `Initialize`/`PostInitialize` for setup
- `Start`/`Stop` for execution control
- `Uninitialize` for cleanup
- `Send`/`Recv` for data flow (if applicable)
- `IsReady`, `Update`, etc. based on the interface requirements

The function signatures are generated by the InterfaceBuilder and defined in `IfaceFuncs.inl`.