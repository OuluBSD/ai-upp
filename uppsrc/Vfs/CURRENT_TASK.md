Package CURRENT TASK: Vfs

Theme
- Consolidate VfsValue and related infrastructure into `uppsrc/Vfs/*` with clear layering.

Goals (Phase 1)
- Define target package boundaries and responsibilities (Core, Factory, Overlay, Storage, Ctrl).
- Do not break existing includes: prepare staged adapter approach from Core2.
- Document header include policy and Env adapter expectations.

Progress
- ✅ Docs established (`AGENTS.md`, this file).
- ✅ Header scaffolding for Core/Factory/Overlay/Storage created.
- ✅ IDE package now depends on new scaffolds and ships default precedence stub.
- ✅ Factory implementations (`IsType`, `Create`, `Clone`, etc.) moved out of `Core2/VfsValue.cpp` into `Vfs/Factory`.

Planned Steps (next phase)
1) Gradually migrate remaining `VfsValueExtFactory` definitions (registration helpers, data maps) into `Vfs/Factory`.
2) Begin extracting non-UI portions of `VfsValue` (struct definition, helpers) into `Vfs/Core`.
3) Implement overlay view logic leveraging `SourceRef` and precedence provider interfaces.
4) Implement JSON serialization in `Vfs/Storage` and add round-trip tests.
5) Update IDE Env adapters to construct overlays using the precedence provider and new storage APIs.

Notes
- Keep GUI controls in a separate package (Vfs/Ctrl) to respect BLITZ and reduce coupling.
- Maintain backward compatibility: old IDE Vfs dumps accepted by `Storage` loaders.

Next
- Create minimal headers (no behavior change) and wire `Core2/Core.h` to include them as a transitional step.
