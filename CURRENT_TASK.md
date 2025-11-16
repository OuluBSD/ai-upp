Repo-wide CURRENT TASK

VfsValue Rewrite: Core split, overlays, and serialization

Objectives
- Collect Vfs/VfsValue-related code into focused Vfs packages under `uppsrc/Vfs/*` and minimize spread.
- Replace merge/unmerge with a virtual overlay model retaining per-source fragments and provenance.
- Redesign serialization to store per-file fragments plus overlay index; ensure backward compatibility with IDE data.
- Provide clean Env API adapters in `uppsrc/ide/Vfs` over the new core.

Scope & Ownership
- Core data structures: `uppsrc/Vfs/Core` (VfsValue, VfsValueExt, AstValue, enums, paths).
- Factory/registration: `uppsrc/Vfs/Factory`.
- Overlay/union view and provenance: `uppsrc/Vfs/Overlay`.
- Serialization: `uppsrc/Vfs/Storage`.
- IDE integration: `uppsrc/ide/Vfs` (adapters, UI; no core logic).

Design Notes
- Keep multiple potential sources for a node; treat logical tree as union with conflict resolution policies.
- Provenance: track `{pkg_hash, file_hash, local_path/id, priority, flags}` per contribution.
- Extensions: maintain 1:1 mapping between `VfsValueExt` (Core) and `VfsValueExtCtrl` (Ctrl); register via Factory.
- Header policy (U++ BLITZ): source files include only package main header first.

Deliverables (phased)
1) Documentation updates (this file, AGENTS) and package scaffolding plan.
2) Extract minimal `Vfs/Core` API surface (headers only, no behavior change) and adapt includes.
3) Introduce `Overlay` structs (`SourceRef`, `OverlayView`) with no behavior change; add tests/examples later.
4) Define JSON/Stream schema in `Vfs/Storage` and implement no-op loaders that accept old format.
5) Wire IDE Env adapters to call overlay APIs; deprecate physical un-merge routines.

Open Questions
- Staged refactor (adapters) vs. hard move of includes?
- Deterministic overlay ordering (workspace-configured priority) vs. default first-wins?
- Persist overlay index to disk or compute in-memory on load?

Status
- Header scaffold for `Vfs/Core`, `Vfs/Factory`, `Vfs/Overlay`, `Vfs/Storage` in place.
- `VfsValueExtFactory` core methods moved from `Core2/VfsValue.cpp` into new `Vfs/Factory` package (still depending on legacy structures).

Next
- Continue migrating remaining `VfsValue` helpers into the new packages while keeping builds stable.
- Flesh out overlay implementation using precedence provider.
- Implement serialization format and adapters, then update IDE Env to consume it.

Secondary Task: VfsShell Overlay Implementation: Implement a VFS shell with `/vfs/` overlay for internal VFS filesystem access

Objectives
- Create a VfsShell that works like a standard Bourne shell with system filesystem as default
- Implement a `/vfs/` overlay path that provides access to the internal VFS filesystem
- Update all command implementations to properly handle both system paths and VFS overlay paths
- Ensure consistent behavior across both path systems while maintaining backward compatibility
- Document the dual-path system and usage patterns

Current Status
- Updated VfsShell to use system filesystem as default with string-based current working directory
- Implemented path conversion utilities to detect VFS paths (starting with `/vfs/`)
- Updated pwd, cd, ls, tree, mkdir, and touch commands to work with both system and VFS paths
- Remaining work: Update all other commands (rm, mv, link, export, cat, grep, etc.) to handle both path systems

Next Steps
- Complete implementation of remaining commands to handle both system paths and VFS overlay
- Test functionality with both system and VFS files to ensure proper separation
- Document the dual-path system and usage patterns

Objectives
- Collect Vfs/VfsValue-related code into focused Vfs packages under `uppsrc/Vfs/*` and minimize spread.
- Replace merge/unmerge with a virtual overlay model retaining per-source fragments and provenance.
- Redesign serialization to store per-file fragments plus overlay index; ensure backward compatibility with IDE data.
- Provide clean Env API adapters in `uppsrc/ide/Vfs` over the new core.

Scope & Ownership
- Core data structures: `uppsrc/Vfs/Core` (VfsValue, VfsValueExt, AstValue, enums, paths).
- Factory/registration: `uppsrc/Vfs/Factory`.
- Overlay/union view and provenance: `uppsrc/Vfs/Overlay`.
- Serialization: `uppsrc/Vfs/Storage`.
- IDE integration: `uppsrc/ide/Vfs` (adapters, UI; no core logic).

Design Notes
- Keep multiple potential sources for a node; treat logical tree as union with conflict resolution policies.
- Provenance: track `{pkg_hash, file_hash, local_path/id, priority, flags}` per contribution.
- Extensions: maintain 1:1 mapping between `VfsValueExt` (Core) and `VfsValueExtCtrl` (Ctrl); register via Factory.
- Header policy (U++ BLITZ): source files include only package main header first.

Deliverables (phased)
1) Documentation updates (this file, AGENTS) and package scaffolding plan.
2) Extract minimal `Vfs/Core` API surface (headers only, no behavior change) and adapt includes.
3) Introduce `Overlay` structs (`SourceRef`, `OverlayView`) with no behavior change; add tests/examples later.
4) Define JSON/Stream schema in `Vfs/Storage` and implement no-op loaders that accept old format.
5) Wire IDE Env adapters to call overlay APIs; deprecate physical un-merge routines.

Open Questions
- Staged refactor (adapters) vs. hard move of includes?
- Deterministic overlay ordering (workspace-configured priority) vs. default first-wins?
- Persist overlay index to disk or compute in-memory on load?

Status
- Header scaffold for `Vfs/Core`, `Vfs/Factory`, `Vfs/Overlay`, `Vfs/Storage` in place.
- `VfsValueExtFactory` core methods moved from `Core2/VfsValue.cpp` into new `Vfs/Factory` package (still depending on legacy structures).

Next
- Continue migrating remaining `VfsValue` helpers into the new packages while keeping builds stable.
- Flesh out overlay implementation using precedence provider.
- Implement serialization format and adapters, then update IDE Env to consume it.

Secondary Task: VfsShell Overlay Implementation

Objective: Implement a VfsShell that works like a standard Bourne shell with a `/vfs/` overlay for internal VFS filesystem access

- System filesystem is the default, but `/vfs/` path provides access to the internal VFS filesystem
- Update all command implementations to properly handle both system paths and VFS overlay paths
- Ensure consistent behavior across both path systems
- Maintain backward compatibility with existing shell functionality

## GameEngine Development Progress

### Status: IN-PROGRESS (Secondary Task)

#### Current Implemented Features
- [x] **GameLib Package**: Core game engine functionality layer with basic structure
- [x] **GameEngine Package**: Higher-level game engine features with basic structure
- [x] **Geometry Integration**: Integration with uppsrc/Geometry for vectors, matrices, and 3D math operations
- [x] **Screen API Integration**: Leverage uppsrc/api/Screen for cross-platform windowing and input handling
- [x] **Graphics API Integration**: Integration with uppsrc/api/Graphics for rendering capabilities (OpenGL, DirectX, etc.)
- [x] **GameWindow Class**: Design GameWindow class as an easy-to-use solution for game windows
- [x] **Game Loop Architecture**: Implement basic game loop architecture in GameEngine
- [ ] **ECS Framework**: Create entity-component-system (ECS) framework in GameEngine (using existing ai-upp ECS code after upptst/Eon* packages work properly)
- [x] **Asset Management**: Add support for asset management and resource loading (AssetManager with memory budget tracking)
- [x] **2D/3D Rendering Pipeline**: Implement basic 2D and 3D rendering pipelines
- [x] **Camera System**: Create camera system with support for different projection types
- [x] **Shader Management**: Implement shader management system
- [x] **Sprite and Mesh Rendering**: Implement sprite and mesh rendering
- [x] **Material System**: Create material system
- [x] **Post-Processing Effects**: Implement post-processing effects
- [x] **Texture Streaming**: Add texture streaming and management
- [x] **Render Batching**: Implement render batching and optimization
- [ ] **Input Handling**: Develop input handling system unified across different platforms
- [ ] **Audio Integration**: Add audio integration using uppsrc/api/Audio package
- [ ] **Physics Integration**: Implement basic physics integration using uppsrc/api/Physics package
- [x] **Scene Management**: Design scene management system (Scene and SceneManager classes)
- [ ] **UI/HUD Rendering**: Create basic UI/HUD rendering capabilities
- [x] **Resource Management**: Design resource management with proper memory handling (via AssetManager)
- [x] **Comprehensive Tests**: Write comprehensive tests for GameLib and GameEngine packages (gametst package)

### Roadmap Phases
1. **Phase 1: Foundation** (COMPLETED) - Basic structure and core systems
2. **Phase 2: ECS Integration** (PENDING) - Integration with existing ai-upp ECS after upptst/Eon* fixes
3. **Phase 3: Rendering and Graphics** (COMPLETED) - Advanced rendering capabilities
4. **Phase 4: Audio and Input** (PLANNED) - Audio and input systems
5. **Phase 5: Physics and Animation** (PLANNED) - Physics and animation
6. **Phase 6: Advanced Features** (PLANNED) - UI, networking, etc.

For full roadmap details, see GameEngine.md and task/GameEngine.md
