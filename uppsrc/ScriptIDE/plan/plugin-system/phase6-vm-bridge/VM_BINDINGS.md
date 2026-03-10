# Python VM Binding Bridge

## Overview
Plugins must be able to expose C++ functionality (e.g., game board manipulation) to Python scripts running in the `ByteVM`.

## The `IPythonBindingProvider` Interface
```cpp
class IPythonBindingProvider {
public:
    virtual ~IPythonBindingProvider() {}
    virtual void SyncBindings(PyVM& vm) = 0; // Injects functions into the VM
};
```

## Synchronization Workflow
Bindings are not persistent; they must be re-injected whenever a script starts:
1. `PythonIDE::OnRun` (or custom execute provider) initializes/resets the `PyVM`.
2. `PluginManager::SyncBindings(vm)` is called.
3. It iterates through all active `IPythonBindingProvider` instances.
4. Each plugin calls `vm.SetGlobal("func_name", PyValue::Function(...))`.

## Naming & Scoping
To prevent collisions between plugins:
- **Direct Globals**: Discouraged.
- **Namespaced Modules**: Recommended.
  ```cpp
  // Plugin implementation
  PyValue module = PyValue::Dict();
  module.Set("move_card", PyValue::Function(...));
  vm.SetGlobal("hearts_view", module);
  ```
- This allows Python scripts to use: `hearts_view.move_card(...)`.

## Event Callbacks (UI to Python)
If a plugin needs to trigger Python code from C++ (e.g., `on_click`):
1. The plugin stores a reference to the `PyVM`.
2. It looks up a specific function name in `vm.GetGlobals()`.
3. If found and callable, it executes the function using `vm.Call(...)`.

## Safety & Lifetimes
- C++ callbacks must verify that the UI objects they manipulate still exist (use `Ptr<Ctrl>` or checked handles).
- Plugins must not store `PyValue` objects across VM resets without proper reference management.
