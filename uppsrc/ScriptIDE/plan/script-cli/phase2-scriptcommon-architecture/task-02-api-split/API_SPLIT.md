# API Split Result: Plugin Core vs GUI Adapters

## Purpose
Define concrete API ownership split for current `PluginInterfaces.h` and `PluginManager.*` so ScriptCLI/MCP can use plugin core without GUI dependencies.

## Current Problem
`PluginInterfaces.h` currently mixes headless and GUI-coupled contracts:
- GUI-coupled types in core contracts: `Ctrl`, `Bar`, `PythonIDE`.
- `PluginManager` combines headless registry logic and dock-pane GUI integration.

This prevents clean `ScriptCommon` use from ScriptCLI.

## Target Ownership

### ScriptCommon (Headless Core)
- Plugin lifecycle
- File-type routing
- Python binding providers
- Custom execute providers
- Plugin registry state

### ScriptIDE (GUI Frontend Adapter)
- Dock panes (`DockableCtrl`, `Ctrl`)
- Menu/toolbar contributions (`Bar`)
- Document-host controls (`Ctrl` ownership)
- Direct `PythonIDE` integration

## Proposed Interface Split

### New ScriptCommon interfaces

```cpp
class IScriptPlugin {
public:
    virtual ~IScriptPlugin() {}
    virtual String GetID() const = 0;
    virtual String GetName() const = 0;
    virtual String GetDescription() const = 0;
    virtual void   Init(class IPluginRuntimeContext& ctx) = 0;
    virtual void   Shutdown() = 0;
};

class IPluginRuntimeContext {
public:
    virtual ~IPluginRuntimeContext() {}
    virtual PyVM* GetVM() = 0;
};

class IFileTypeHandlerCore {
public:
    virtual ~IFileTypeHandlerCore() {}
    virtual String GetExtension() const = 0;
    virtual String GetFileDescription() const = 0;
    virtual bool   CanHandle(const String& path) const = 0;
};

class IPythonBindingProvider {
public:
    virtual ~IPythonBindingProvider() {}
    virtual void SyncBindings(PyVM& vm) = 0;
};

class ICustomExecuteProvider {
public:
    virtual ~ICustomExecuteProvider() {}
    virtual bool CanExecute(const String& path) = 0;
    virtual void Execute(const String& path) = 0;
};
```

### New ScriptIDE adapter interfaces

```cpp
class IGuiPluginContext {
public:
    virtual ~IGuiPluginContext() {}
    virtual PythonIDE& GetIDE() = 0;
    virtual void RegisterDockPane(const String& id, const String& title, Ctrl& ctrl) = 0;
    virtual void UnregisterDockPane(const String& id) = 0;
};

class IGuiDocumentHost {
public:
    virtual ~IGuiDocumentHost() {}
    virtual Ctrl&  GetCtrl() = 0;
    virtual bool   Load(const String& path) = 0;
    virtual bool   Save() = 0;
    virtual bool   SaveAs(const String& path) = 0;
    virtual String GetPath() const = 0;
    virtual bool   IsModified() const = 0;
    virtual void   SetFocus() = 0;
};

class IGuiPaneProvider {
public:
    virtual ~IGuiPaneProvider() {}
    virtual int    GetPaneCount() const = 0;
    virtual String GetPaneID(int index) const = 0;
    virtual String GetPaneTitle(int index) const = 0;
    virtual Ctrl&  GetPaneCtrl(int index) = 0;
};
```

## Manager Split

### ScriptCommon
- `PluginRegistryCore.h/.cpp`
- Responsibilities:
  - load plugin factories
  - enable/disable state
  - core provider registration and lookup
  - binding sync dispatch

### ScriptIDE
- `PluginUiBridge.h/.cpp`
- Responsibilities:
  - register/unregister dock panes
  - GUI document host integration
  - menu/window wiring for plugin UI

## Compatibility Layer Plan
1. Keep old `PluginInterfaces.h` temporarily as a facade.
2. Introduce adapter shims mapping legacy plugin types to new split interfaces.
3. Migrate built-in plugins (`GameStatePlugin`, `CardGamePlugin`) in two steps:
   - move core logic to ScriptCommon plugin classes
   - keep UI wrappers in ScriptIDE plugins
4. Remove legacy facade after migration is complete.

## Impacted Existing Files
- `uppsrc/ScriptIDE/PluginInterfaces.h`
- `uppsrc/ScriptIDE/PluginManager.h`
- `uppsrc/ScriptIDE/PluginManager.cpp`
- `uppsrc/ScriptIDE/GameStatePlugin.*`
- `uppsrc/ScriptIDE/CardGamePlugin.*`

## Acceptance
- [x] Headless plugin contracts identified.
- [x] GUI adapter contracts isolated.
- [x] Manager split responsibilities defined.
- [x] Compatibility transition strategy documented.
