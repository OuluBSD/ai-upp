AGENTS

Scope
- Applies to `uppsrc/Eon/Win` and its sub-tree.

Purpose
- UWP/DX12 DemoRoom integration for Eon using the Vfs/Ecs runtime.
- Source baseline: MRTK SpatialInput DemoRoom + required libs (Pbr/Neso/Gltf/SpatialInputUtilities) adapted to Eon.

Conventions
- Do not reintroduce the old Neso ECS (`Engine`, `EntityStore`, `ComponentStore`). Use `Vfs/Ecs` systems/components.
- Keep UWP entrypoint via `AppView` and `CoreApplication::Run`.
- Register all DemoRoom systems/components in a `.icpp` with `REGISTER_EON_SYSTEM`/`REGISTER_EON_COMPONENT`.
- Assets must be packaged for UWP and referenced via `ms-appx:///` paths.
