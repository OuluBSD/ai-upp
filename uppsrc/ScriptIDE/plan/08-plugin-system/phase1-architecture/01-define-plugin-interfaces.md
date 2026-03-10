# Task: Define Plugin Interfaces

## Goal
Design and define the base interfaces for the ScriptIDE plugin system to support various extension points.

## Background
To support custom file types, dockable panes, and VM bindings, we need a unified interface that all plugins must implement. This ensures lifecycle management (enable/disable) and discovery.

## Strategy
1. **Define IPlugin**: The root interface for any plugin.
2. **Define Extension Point Interfaces**:
   - `IFileTypeHandler`: For handling custom extensions (e.g., `.gamestate`).
   - `IDocumentViewFactory`: For creating custom `Ctrl`s for specific file types.
   - `IDockPaneProvider`: For registering plugin-provided dockable panes.
   - `IPythonBindingProvider`: For exposing C++ functions to the ByteVM.
   - `ICustomExecuteProvider`: For overriding the "Run" behavior for specific documents.

## Implementation Details

### Plugin Interface (Sketch)
```cpp
class IPlugin {
public:
    virtual ~IPlugin() {}
    virtual String  GetName() const = 0;
    virtual void    OnEnable() = 0;
    virtual void    OnDisable() = 0;
};
```

### Extension Points
- Plugins will return instances or callbacks for these extension points during registration.

## Success Criteria
- [ ] Header file `PluginInterfaces.h` defined.
- [ ] All required extension points covered by abstract interfaces.
- [ ] Lifecycle methods (`OnEnable`, `OnDisable`) included.
