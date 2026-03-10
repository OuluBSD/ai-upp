# Plugin Enable/Disable Impact Analysis

## UI Impact

### File Handlers & Tabs
- **Disabling**:
  - `PluginManager` removes the file extension association.
  - `PythonIDE` must iterate through all open `FileInfo` entries.
  - If a tab is using a document host provided by the disabled plugin, the user is prompted to save.
  - The tab is then closed.
- **Enabling**:
  - `PluginManager` registers the extension.
  - Future "Open" calls for that extension will use the plugin's host.

### Dockable Panes
- **Disabling**:
  - Panes registered by the plugin are unregistered from the `DockWindow`.
  - The `Window` menu is rebuilt to remove the entries.
  - The panes are destroyed.
- **Enabling**:
  - Panes are added to the `DockWindow` (initially hidden or in their last saved position).
  - `Window` menu is rebuilt.

## VM Impact

### Python Bindings
- **Disabling**:
  - `ByteVM` globals registered by the plugin must be removed.
  - If a script is currently running, it might crash if it calls a revoked function. To mitigate this, a "Registry Epoch" or simple clear-on-stop strategy is used.
- **Enabling**:
  - Bindings are injected into the next VM instance created or synced.

## Menu & Toolbars
- Any custom menu items or toolbar buttons added by the plugin must be removed/hidden upon disabling.

## Persistence
- The `enabled` state of each plugin is stored in `ide_settings.bin` via the `PluginSettings` struct.
- Layout of plugin panes is stored in `docking-layout.bin` via standard U++ `SerializeWindow`.
