Package CURRENT TASK: Vfs

Theme
- Consolidate VfsValue and related infrastructure into `uppsrc/Vfs/*` with clear layering.

Goals (Phase 1)
- Define target package boundaries and responsibilities (Core, Factory, Overlay, Storage, Ctrl).
- Do not break existing includes: prepare staged adapter approach from Core2.
- Document header include policy and Env adapter expectations.

Planned Steps
1) Author design docs in `AGENTS.md` and this file (done).
2) Introduce `Vfs/Core` main header with forward decls and typedefs matching current usage.
3) Add `Vfs/Factory` header with factory list and registration API (names preserved).
4) Add `Vfs/Overlay` headers for `SourceRef`, `OverlayView` (API only).
5) Add `Vfs/Storage` headers for new serialization schema; provide migration notes.
6) Update `uppsrc/ide/Vfs` to depend on these headers gradually.

Notes
- Keep GUI controls in a separate package (Vfs/Ctrl) to respect BLITZ and reduce coupling.
- Maintain backward compatibility: old IDE Vfs dumps accepted by `Storage` loaders.

Next
- Create minimal headers (no behavior change) and wire `Core2/Core.h` to include them as a transitional step.
