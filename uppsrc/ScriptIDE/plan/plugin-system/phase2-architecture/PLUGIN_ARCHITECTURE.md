# ScriptIDE Plugin Architecture Design

## Core Interfaces

### IPlugin
The base interface for all plugins.
```cpp
class IPlugin {
public:
    virtual ~IPlugin() {}
    virtual String GetID() const = 0;          // Unique identifier
    virtual String GetName() const = 0;        // Display name
    virtual String GetDescription() const = 0; // Short description
    virtual void   Init(IPluginContext& ctx) = 0;
    virtual void   Shutdown() = 0;
};
```

### IPluginContext
Provided to the plugin during `Init`. Allows registration of capabilities.
```cpp
class IPluginContext {
public:
    virtual ~IPluginContext() {}
    virtual PythonIDE& GetIDE() = 0;
    virtual PyVM*      GetVM() = 0;
    virtual void       RegisterDockPane(const String& id, const String& title, Ctrl& ctrl) = 0;
    virtual void       UnregisterDockPane(const String& id) = 0;
};
```

## Plugin Manifest (Metadata)
Plugins are registered via a static factory. The manifest is inferred from the `IPlugin` virtual methods.
```cpp
struct PluginManifest {
    String id;
    String name;
    String version;
    String description;
};
```

## Registration Mechanism
A static macro `REGISTER_PLUGIN(T)` will add a factory function to a global `Vector<PluginFactory>`.
```cpp
#define REGISTER_PLUGIN(T) \
static IPlugin* T##_Factory() { return new T(); } \
static bool T##_Registered = []() { \
    GetInternalPluginFactories().Add(&T##_Factory); \
    return true; \
}();
```

## Lifecycle Management
1. **Discovery**: `PluginManager` iterates through `GetInternalPluginFactories()`.
2. **Enable**: Plugin is instantiated, `Init(context)` is called.
3. **Registration**: During `Init`, the plugin calls `context.Register...` methods.
4. **Disable/Shutdown**: `Shutdown()` is called, registry is cleared for that plugin.
