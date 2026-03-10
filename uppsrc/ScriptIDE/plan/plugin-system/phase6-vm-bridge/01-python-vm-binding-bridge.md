# Task: Python VM Binding Bridge

## Goal
Design the bridge that allows plugins to safely bind C++ functions and data structures into the active ByteVM, making them accessible to Python scripts.

## Background / Rationale
For a Python game script to control a custom game view, the view must expose an API (e.g., `move_card(id, x, y)` or `get_score()`) to the Python environment. The `ByteVM` provides `SetGlobal` and `PyValue::Function`, but plugins need a standardized way to register these bindings during execution without causing naming collisions or memory leaks.

## Scope
- Defining the `IPythonBindingProvider` interface.
- Defining the lifecycle of a binding (when it is added, when it is removed).
- Defining document-scoped vs. application-scoped bindings.
- Designing the API shape for rendering commands and event callbacks from UI to Python.

## Non-goals
- Implementing the ByteVM C++ reflection system itself (we use the existing `PyValue` system).

## Dependencies
- `uppsrc/ByteVM/PyVM.h`
- `01-plugin-lifecycle-manifest.md`

## Concrete Investigation Steps
1. Review `PyValue::Function` and how native C++ callbacks are currently mapped in `uppsrc/ByteVM`.
2. Determine how `PythonIDE` can trigger a "SyncBindings" pass on all active plugins just before executing a `.gamestate` script.
3. Design a mechanism to namespace plugin bindings (e.g., placing them inside a Python module dictionary like `import my_game_plugin` rather than global scope) to prevent collisions.

## Affected Subsystems
- `uppsrc/ScriptIDE/PluginInterfaces.h`
- `uppsrc/ScriptIDE/PluginManager.cpp` (Binding sync logic)

## Implementation Direction
Create an architecture document detailing the `IPythonBindingProvider::SyncBindings(PyVM& vm)` interface. Outline a strategy for safe unbinding when a plugin tab closes or the plugin is disabled.

## Risks
- A C++ callback might try to access a UI element (like a card sprite) that has been destroyed if the user closes the tab while the script is running.
- Global namespace pollution in the Python VM.

## Acceptance Criteria
- [ ] Documented `IPythonBindingProvider` interface.
- [ ] Defined binding scope (application vs document).
- [ ] Documented safe unbinding and lifetime rules.
- [ ] Example API shape for rendering commands.
