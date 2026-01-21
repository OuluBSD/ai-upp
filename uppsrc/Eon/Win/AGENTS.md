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
- Prefer ownership via `One<>`/`Array<T>` with clear single owners; avoid `std::shared_ptr` and `std::weak_ptr`.
- Use `Ptr<>` only when the target class inherits `Pte<>` and lifetime is owned elsewhere; otherwise use raw refs/ptrs with explicit unregister in `Uninitialize()`/`Stop()`.
- `Ptr<>` is non-owning and requires `Pte<>`; prefer `One<>` for ownership and `Array<T>` over `Vector<One<>>`.

Known Issues
- **Build Failure (hstring.h)**: The `UWP` build method (`UWP_INTERNAL`) appears to include Clang headers (`upp/bin/clang/include/hstring.h`) even when building with MSVC (`flagMSC`), causing syntax errors due to incompatible or missing macro definitions (e.g., `DECLARE_HANDLE`). This suggests a misconfiguration in the build environment or `UWP.bm` include paths.
- **Missing Shaders**: `ShaderBytecode.h` contains placeholder byte arrays. Compiled HLSL shaders are required for rendering to work.
