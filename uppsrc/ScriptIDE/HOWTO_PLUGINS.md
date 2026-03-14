# ScriptIDE Plugin HOWTO

This document explains how to create plugins for `ScriptIDE`, especially from another repository.

The intended model is:

- `ScriptIDE` defines the plugin API
- plugin packages depend on `ScriptIDE`
- plugins register themselves from their own `.icpp`
- `ScriptIDE` remains the application that provides `main`
- the plugin package contributes features by being linked into the final `ScriptIDE` binary

This is not a runtime DLL plugin system. It is a compile-time extension system based on U++ packages and static registration.

## 1. What ScriptIDE Exposes

The public GUI plugin-facing interfaces live in:

- [PluginInterfacesGUI.h](/common/active/sblo/Dev/ai-upp/uppsrc/ScriptIDE/PluginInterfacesGUI.h)

The plugin manager and dispatch live in:

- [PluginManager.h](/common/active/sblo/Dev/ai-upp/uppsrc/ScriptIDE/PluginManager.h)
- [PluginManager.cpp](/common/active/sblo/Dev/ai-upp/uppsrc/ScriptIDE/PluginManager.cpp)

The important design rule is:

- `uppsrc/ScriptIDE/` must not include headers from concrete plugins
- concrete plugins only include `ScriptIDE` headers
- registration happens from the plugin package itself

That is what allows plugins to live outside this repository.

## 2. Current Extension Points

Today a plugin can provide any of these:

- file handlers for document viewer/editor hosts
- dockable panes
- preferences pages
- run-state listeners
- python binding providers
- custom execute providers

Important interfaces:

- `IPlugin`
- `IPluginContext`
- `IPluginContextGUI`
- `IPluginRegistry`
- `IPluginRegistryGUI`
- `IDocumentHost`
- `IFileTypeHandler`
- `IDockPaneProvider`
- `IPluginPreferencesProvider`
- `IPluginPreferencesPage`
- `IRunStateListener`
- `IVideoRenderSource`

## 3. Build Model

`ScriptIDE` is still the application. Your plugin package is linked into `ScriptIDE`.

That means:

1. Your plugin package depends on `ScriptIDE`.
2. The final application package depends on both `ScriptIDE` and your plugin package.
3. Your plugin registers itself through a static registration macro in an `.icpp`.
4. When `ScriptIDE` starts, the plugin manager discovers the registered plugin factory and can enable it.

There is still only one `main`, and it is `ScriptIDE/Main.cpp`.

## 4. Recommended Package Layout

For an external plugin repository, use something like:

```text
MyPluginRepo/
  uppsrc/
    MyScriptIDEPlugin/
      AGENTS.md
      MyScriptIDEPlugin.upp
      MyScriptIDEPlugin.h
      MyScriptIDEPlugin.cpp
      MyScriptIDEPluginInit.icpp
```

Recommended `MyScriptIDEPlugin.upp` dependencies:

- `ScriptIDE`
- any additional U++ packages you actually need

Your package should not add its own `main`.

## 5. Minimal Plugin Skeleton

### Header

```cpp
#ifndef _MyScriptIDEPlugin_MyScriptIDEPlugin_h_
#define _MyScriptIDEPlugin_MyScriptIDEPlugin_h_

class MyScriptIDEPlugin : public IPlugin {
public:
	virtual String GetID() const override { return "example.my_plugin"; }
	virtual String GetName() const override { return "My Plugin"; }
	virtual String GetDescription() const override { return "Example ScriptIDE plugin."; }
	virtual void   Init(IPluginContext& context) override;
	virtual void   Shutdown() override;
};

#endif
```

### Main implementation

```cpp
#include "MyScriptIDEPlugin.h"

NAMESPACE_UPP

void MyScriptIDEPlugin::Init(IPluginContext& context)
{
	if(IPluginContextGUI* gui = dynamic_cast<IPluginContextGUI*>(&context)) {
		gui->GetIDE().Log("My plugin initialized.");
	}
}

void MyScriptIDEPlugin::Shutdown()
{
}

END_UPP_NAMESPACE
```

### Registration unit

```cpp
#include "MyScriptIDEPlugin.h"

NAMESPACE_UPP

REGISTER_SCRIPTIDE_PLUGIN(MyScriptIDEPlugin)

END_UPP_NAMESPACE
```

The registration must live in a compilation unit that is linked into the final executable. A separate `.icpp` is the normal place.

## 6. How Registration Works

`REGISTER_SCRIPTIDE_PLUGIN(T)` expands to the internal plugin registration mechanism already used by `ScriptIDE`.

At startup:

1. `PluginManager::LoadPlugins()` collects factories from the internal registry
2. it instantiates those plugins
3. enabled plugins receive `Init(...)`
4. disabled plugins exist in the plugin list but are not initialized

This is why your plugin package must be linked into the final program. If it is not linked, its registration unit never runs and the plugin does not exist to `ScriptIDE`.

## 7. Enabling the Plugin in the Final Build

There are two common ways to use an external plugin package.

### Option A: Build a custom ScriptIDE app package

Create a small app package, for example:

```text
MyScriptIDEApp/
```

that depends on:

- `ScriptIDE`
- `MyScriptIDEPlugin`

and whose `main` simply delegates to ScriptIDE's main package logic if your app structure is built that way.

This is the cleanest route when you want a branded/custom IDE binary.

### Option B: Add the plugin package as a dependency of `ScriptIDE`

If you are working in one integrated source tree, you can add your plugin package to `ScriptIDE.upp` `uses`.

That is what the in-tree `VideoRecord` plugin currently does.

For truly external repository usage, Option A is the better long-term design because it avoids modifying upstream `ScriptIDE.upp`.

## 8. Using GUI Context

If your plugin needs GUI access, cast the incoming context:

```cpp
void MyPlugin::Init(IPluginContext& context)
{
	IPluginContextGUI* gui = dynamic_cast<IPluginContextGUI*>(&context);
	if(!gui)
		return;

	gui->GetIDE().Log("GUI plugin ready.");
}
```

What `IPluginContextGUI` currently gives you:

- `PythonIDE& GetIDE()`
- `RegisterDockPane(id, title, ctrl)`
- `UnregisterDockPane(id)`

Use this for GUI-specific integration only. Keep the plugin interface side generic where possible.

## 9. Adding a Preferences Page

Implement:

- `IPluginPreferencesPage`
- `IPluginPreferencesProvider`

Then register the provider in `Init(...)` through `IPluginRegistryGUI`.

Minimal pattern:

```cpp
class MyPrefsPage : public ParentCtrl, public IPluginPreferencesPage {
public:
	virtual Ctrl& GetCtrl() override { return *this; }
	virtual void  LoadConfig() override {}
	virtual void  SaveConfig() override {}
	virtual void  ApplyConfig(IDEContext& ctx) override {}
	virtual void  SetDefaults() override {}
	virtual bool  IsModified() const override { return false; }
};

class MyPlugin : public IPlugin, public IPluginPreferencesProvider {
	// ...
};
```

Then:

```cpp
registry->RegisterPreferencesProvider(*this);
```

Important methods:

- `GetPreferencesPageCount()`
- `GetPreferencesPageCategory(index)`
- `GetPreferencesPageID(index)`
- `GetPreferencesPageTitle(index)`
- `GetPreferencesPageIcon(index)`
- `GetPreferencesPage(index)`

Use stable page ids. The Preferences window now remembers the last selected page by page id.

## 10. Adding a Dockable Pane

The simplest route is direct pane registration:

```cpp
gui->RegisterDockPane("example.my_plugin.pane", "My Pane", my_ctrl);
```

And in `Shutdown()`:

```cpp
gui->UnregisterDockPane("example.my_plugin.pane");
```

Rules:

- use a globally unique id
- keep the `Ctrl` alive for as long as the plugin is enabled
- unregister in `Shutdown()`

If your plugin gets disabled from Preferences, the pane should disappear immediately.

## 11. Reacting to Run State

Implement `IRunStateListener` if your plugin needs to know whether the active script is running, paused, debugged, or stopped.

Register with:

```cpp
registry->RegisterRunStateListener(*this);
```

You will receive:

```cpp
void OnRunStateChanged(PythonIDE& ide, const ScriptRunState& state) override;
```

Current useful fields:

- `state.host`
- `state.path`
- `state.mode`
- `state.running`
- `state.paused`
- `state.can_run`

This is how `VideoRecord` knows when a `.gamestate` host is recordable.

## 12. Adding File Handlers

Implement `IFileTypeHandler`.

Important methods:

- `GetExtension()`
- `GetFileDescription()`
- `CanHandle(path)`
- `SupportsHostRole(role)`
- `CreateEditorHost()`
- `CreateViewerHost()`

`ScriptIDE` now prefers:

1. editor host
2. viewer host

based on the requested role and the file handler capability.

Example:

```cpp
class MyHandler : public IFileTypeHandler {
public:
	virtual String GetExtension() const override { return ".myext"; }
	virtual String GetFileDescription() const override { return "My File"; }
	virtual bool SupportsHostRole(HostRole role) const override {
		return role == HOSTROLE_EDITOR;
	}
	virtual IDocumentHost* CreateEditorHost() override {
		return new MyDocumentHost();
	}
};
```

Then register:

```cpp
registry->RegisterFileTypeHandler(handler);
```

## 13. Writing a Document Host

If your plugin opens a custom tab type, implement `IDocumentHost`.

At minimum:

- `GetCtrl()`
- `Load(path)`
- `Save()`
- `SaveAs(path)`
- `GetPath()`
- `IsModified()`
- `SetFocus()`

Optional integration:

- `ActivateUI()` / `DeactivateUI()`
- `MainMenu(Bar&)`
- `Toolbar(Bar&)`
- run/debug hooks
- debugger state population
- python stack dump

If the host participates in running code:

- implement `CanRun()`
- `Run()`
- `Debug()`
- `Profile()`
- `Pause()`
- `Stop()`
- `IsRunning()`
- `IsPaused()`
- `GetRunMode()`

If the host can provide frames for recording:

- implement `IVideoRenderSource`

## 14. External Repository Workflow

This is the expected workflow for a plugin outside this repository.

### 14.1. Create your plugin package

Your package depends on `ScriptIDE`.

### 14.2. Include only ScriptIDE public headers

Do not include concrete plugin headers from this repository.

You should normally include only:

- `<ScriptIDE/ScriptIDE.h>`

and your own plugin headers.

### 14.3. Register your plugin in your own `.icpp`

Do not modify `ScriptIDE` source just to register it.

### 14.4. Link your package into the final app package

This is the key step. If your package is not linked, registration never happens.

### 14.5. Run ScriptIDE

`ScriptIDE` is still the application entrypoint.

Your plugin is just another compiled-in contributor to that application.

## 15. Important Architectural Rule

The goal is for `uppsrc/ScriptIDE/` to depend only on plugin interfaces, not concrete plugin packages.

Good:

- ScriptIDE defines `IPluginPreferencesProvider`
- external plugin implements it

Bad:

- ScriptIDE includes `MyPlugin/MyPlugin.h`
- ScriptIDE constructs `MyPlugin` directly

The plugin manager must discover plugin implementations through registration, not through hard-coded includes.

## 16. Current Limitation

The current registration system is still compile-time/static.

That means:

- plugins can live outside this repository
- but they must still be compiled into the final executable
- they are not dynamically discovered from shared libraries at runtime

This is intentional for now.

## 17. In-Tree Example

The current in-tree example plugin is:

- `uppsrc/ScriptIDE/VideoRecord/`

Look there for:

- preferences page provider
- dock pane registration
- run-state listening
- use of `IVideoRenderSource`

There is also a minimal reference package:

- `uppsrc/ScriptIDE/PluginExample/`

Use that package as the smallest end-to-end example of:

- plugin-owned `.upp`
- plugin-owned registration `.icpp`
- one dock pane
- one preferences page
- one run-state listener

## 18. Checklist for a New Plugin

1. Create a package that depends on `ScriptIDE`.
2. Implement an `IPlugin`.
3. Register it in a dedicated `.icpp` using `REGISTER_SCRIPTIDE_PLUGIN(...)`.
4. If needed, also implement:
   - `IPluginPreferencesProvider`
   - `IRunStateListener`
   - `IFileTypeHandler`
5. Link the package into the final application package.
6. Start ScriptIDE and enable the plugin from Preferences if it is not enabled by default.

## 19. Recommended Next API Direction

For future external-plugin friendliness, keep pushing toward:

- no concrete plugin includes from `ScriptIDE`
- stable forward-declared interfaces in `ScriptIDE`
- plugin-owned registration and lifecycle
- application package decides which plugins are linked

That is the model this HOWTO assumes.
