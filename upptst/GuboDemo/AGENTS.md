# AGENTS - GuboDemo Package

## Scope
Applies to `upptst/GuboDemo` and its subtree.

## Purpose
GuboDemo is a demonstration application showcasing the Gubo 3D GUI system. It serves as a minimal example of how to use the GuboCore and GuboLib packages to create 3D graphical user interfaces.

## Architecture
- Main header: `GuboDemo.h` (umbrella). All `.cpp` files in this package must include this header first.
- The demo implements a `TopGubo`-based application with animated 3D objects
- Uses `Draw3` interface for 3D rendering operations
- Contains a single executable target demonstrating basic 3D rendering capabilities

## Dependencies
- `GuboCore` - 3D GUI core infrastructure
- `GuboLib` - Manager system for handling rendering and events
- `Draw/Cuboid` - 3D drawing primitives
- `CtrlLib` - Control library for GUI components

## Header Include Policy (U++ BLITZ)
Every implementation file in this package must include only `#include "GuboDemo.h"` as the first include.
Do not add intra-package `#include` lines in implementation files beyond the umbrella; keep additional rare includes local to that `.cpp`.

Keep headers other than the main header free of third-party/system includes; aggregation and `NAMESPACE_UPP` wrapping happens in `GuboDemo.h`.

## What Was Done
1. Created the basic GuboDemo structure:
   - `GuboDemo.h` with a main class inheriting from `TopGubo`
   - `GuboDemo.cpp` with implementation of 3D drawing in the `Paint(Draw3& d)` method
   - `GuboDemo.upp` package definition file

2. Implemented a simple 3D scene with:
   - A ground plane
   - Three animated cubes (moving, bouncing, and rotating)
   - Proper initialization using GUI_APP_MAIN

3. Created this AGENTS.md file documenting the package

## Build Status
The code is structurally correct and follows patterns from GuboTests, but the build fails due to complex dependency chain in the U++ ecosystem. The Gubo system depends on many interconnected packages that need to be built in the correct order.

## How to Continue in Next Session
1. First, ensure all dependencies are properly built:
   - Build Core packages: `Core`, `Core/Cuboid`
   - Build Draw packages: `Draw`, `Draw/Cuboid`
   - Build Gubo packages: `GuboCore`, `GuboLib`
   
2. Then attempt to build GuboDemo:
   ```
   cd /path/to/ai-upp
   ./bin/umk . upptst/GuboDemo -d
   ```

3. If build issues persist, examine the dependency chain more carefully and build packages in the correct order.

4. Once built successfully, run the demo to verify 3D rendering works properly.

## Notes
- The Gubo system is designed to work with the ECS (Entity Component System) architecture
- The system can work independently of the full Eon stack, making it suitable for standalone 3D GUI applications
- The demo showcases the 3D GUI capabilities without the complexity of the full Eon system