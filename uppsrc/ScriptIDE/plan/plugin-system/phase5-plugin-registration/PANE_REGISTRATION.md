# Custom Dockable Pane Registration

## Overview
Plugins may need custom side-panels (e.g., a "Game Log" or "Asset Library"). These must integrate with ScriptIDE's `DockWindow` architecture and be toggleable via the `Window` menu.

## The `IDockPaneProvider` Interface
```cpp
class IDockPaneProvider {
public:
    virtual ~IDockPaneProvider() {}
    virtual int    GetPaneCount() const = 0;
    virtual String GetPaneID(int index) const = 0;    // Unique ID for persistence
    virtual String GetPaneTitle(int index) const = 0; // Display name
    virtual Ctrl&  GetPaneCtrl(int index) = 0;        // The U++ widget
};
```

## Integration Workflow
1. **Discovery**: `PluginManager` aggregates all active `IDockPaneProvider` instances.
2. **Registration**: For each pane returned by a provider:
   - `PythonIDE` wraps the `Ctrl` in a `DockableCtrl`.
   - The pane is added to the internal `plugin_panes` map.
   - Initial docking position is determined (defaulting to `DockBottom`).
3. **Menu**: `PythonIDE::WindowMenu` iterates through `plugin_panes` and adds a toggle item for each.
4. **Persistence**: `SerializeWindow` automatically saves the dock state (floating, size, position) into `docking-layout.bin`.

## Safe Unregistration
When a plugin is disabled:
1. `PythonIDE` iterates through `plugin_panes`.
2. Any pane belonging to the disabled plugin is removed from the `DockWindow`.
3. The entry is removed from the `plugin_panes` map.
4. The `Window` menu is refreshed.

## Risks: Identity Collisions
Pane IDs must be unique. The recommended convention is `PluginID:PaneID` (e.g., `CardGame:Log`).
