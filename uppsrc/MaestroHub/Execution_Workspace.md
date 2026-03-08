# Execution & Debug Workspace Design (WinXP Style)

## 1. "Run Settings" Dialog
- **Style**: Standard modal dialog with property pages (Tabs).
- **Tab 1: Target**: 
  - GroupBox "Target Device" containing a DropList of ADB devices and a "Refresh" button.
  - GroupBox "Remote Host" for SSH configuration (Hostname, Port, User, Key Path).
- **Tab 2: Environment**: 
  - ArrayCtrl for environment variables (Key/Value pairs).
  - Checkbox "Launch in Debug Mode".
- **Buttons**: [OK] [Cancel] [Apply] [Help]

## 2. The Debug Toolkit (Main Window Layout)
- **ToolBar**: Classic icons for `Run`, `Pause`, `Stop`, `Step Into (F11)`, `Step Over (F10)`, `Step Out`.
- **Docked Views** (Win2003 IDE style):
  - **Output Window** (Bottom): Tabbed view for "Build", "Debug Log", "AI Trace".
  - **Locals / Watch** (Bottom Right): ArrayCtrl showing variable names, values, and types.
  - **Call Stack** (Bottom Left): TreeCtrl showing execution depth.
  - **Source Code** (Central): RichText with line numbers and breakpoint margin.

## 3. Quota & Cost Monitor (Status Bar integration)
- **Position**: Status Bar panes.
- **Pane 1**: Current Model (e.g. "Gemini 1.5 Pro").
- **Pane 2**: Quota remaining (Progress indicator in status bar).
- **Pane 3**: Connection status (Green/Red icon for SSH/ADB).

## 4. Remote Mirroring
- **View**: A list-based dashboard showing "Mirror Projects" synced via Git/SSH.
- **Sync Button**: Manual "Sync Now" button with a spinning icon.
- **Diff View**: Classic side-by-side comparison of local vs remote state.