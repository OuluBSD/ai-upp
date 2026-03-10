# Task: Define Plugin Lifecycle and Registry Model

## Goal
Design the core architecture for the plugin manifest, lifecycle, and registration system within ScriptIDE.

## Background / Rationale
To support modular features like custom card games, ScriptIDE needs a robust plugin architecture. This involves defining how plugins are discovered, how they declare their capabilities (manifest), and how their lifecycle (initialization, shutdown) is managed by the host.

## Scope
- Designing the `IPlugin` interface lifecycle methods.
- Designing the plugin manifest format (metadata).
- Designing the `IPluginRegistry` for registering extension points (file types, panes, VM bindings).
- Defining failure handling for plugin load errors.

## Non-goals
- Implementing the actual plugins.
- Modifying UI components yet.

## Dependencies
- Existing ScriptIDE core architecture.

## Concrete Investigation Steps
1. Review how existing plugins (if any) or similar U++ systems handle registration.
2. Define the exact C++ interface for `IPlugin` and `IPluginContext`.
3. Design a static or dynamic registration mechanism (e.g., `REGISTER_PLUGIN` macro).
4. Specify how failure scenarios (e.g., missing dependencies) are handled during load.

## Affected Subsystems
- `uppsrc/ScriptIDE/PluginInterfaces.h`
- `uppsrc/ScriptIDE/PluginManager.h/cpp`

## Implementation Direction
Create an architecture document outlining the `IPlugin`, `IPluginContext`, and `IPluginRegistry` interfaces. Sketch the `PluginManager` class responsibilities.

## Risks
- Tight coupling between plugins and the IDE core could hinder modularity.
- Lifecycle mismanagement could lead to memory leaks on plugin disable.

## Acceptance Criteria
- [ ] Documented `IPlugin` lifecycle (Init/Shutdown).
- [ ] Documented plugin manifest requirements.
- [ ] Documented registry model for file-type routing, pane registration, and VM bindings.
- [ ] Documented failure handling strategy.
