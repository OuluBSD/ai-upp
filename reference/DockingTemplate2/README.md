# DockingTemplate2

Reusable U++ docking GUI template. Start future GUI tools from this instead of
`DockingTemplate1`.

## What it demonstrates

- `DockWindow`-based main window with `MenuBar` + `ToolBar` frames
- Main `TabCtrl` with three tabs (Tab 1, Tab 2, Debug)
- Per-tab `ToolBar` whose content changes with the active tab
- `Edit` menu that mirrors active-tab toolbar actions via shared `Bar&` functions
- Five-section `MenuBar`: App, Edit, View, Windows, Help
- Three dock panels (`Dock A`, `Dock B`, `Dock C`) registered as `DockableCtrl`-derived classes
- C++ programmatic default dock layout (no `.brc` file needed)
- `InitDockers()` / `OnResetDockLayout()` / `CacheDefaultLayout()` pattern from nide
- In-memory default layout via `StringStream` + `default_layout_data_`
- `AppRegistry` persistence: dock layout blob + `AppState` JSON per session
- Active tab restored by stable name (primary) and index (fallback)
- Two editor modes: `View` (read-only) and `Edit` (interactive)
- Cursor linking between dock views using callbacks, no `MainWindow` backref
- `Debug` tab that logs all UI and persistence events in real time
- `DockManager()` for built-in named layout management
- Reset-to-default action that reapplies the C++ default layout

## File layout

| File | Purpose |
|------|---------|
| `main.cpp` | `GUI_APP_MAIN` entry point |
| `MainWindow.h` | Class declaration + central include header |
| `MainWindow.cpp` | Constructor, `DockInit`, `Close`, layout helpers, persistence |
| `MenuSetup.cpp` | All `Bar&` menu callbacks |
| `ToolBarSetup.cpp` | Per-tab toolbar builder functions |
| `EditMode.h/.cpp` | `ModeManager`: modes and cursor events |
| `DockViews.h/.cpp` | `DockViewA/B/C`: sample dock panels with cursor linking |
| `DebugTab.h/.cpp` | `DebugLog`: timestamped event log tab |
| `AppState.h` | `AppState` struct with `Jsonize()` |

## Build

```sh
umk reference/DockingTemplate2 DockingTemplate2 CLANG -br
```

Or open in TheIDE.

## Persistence

`AppRegistry` stores two things:
1. **Dock layout blob** — binary `SerializeWindow` output, one per tab.
   Key: `dock.layout.tab1`, `dock.layout.tab2`, `dock.layout.debug`.
2. **App state JSON** — active tab name/index + editor mode.
   Key: `app.state`.

Files on Windows: `%APPDATA%\AiUpp\DockingTemplate2\default\`
Files on Linux: `~/.config/AiUpp/DockingTemplate2/default/`

## Adding new dock windows

1. Add a `DockViewX` member to `MainWindow`.
2. Wire it in `InitDockers()`: set title/size, `Register()`, add cursor wiring.
3. Add a `DockRight(dock_x_)` / `DockLeft` call in `OnResetDockLayout()`.
4. Add a show/hide menu item in `MenuView()`.
5. No `.brc` regeneration needed.

## Adding new tabs

1. Add a `ParentCtrl` member for the tab content.
2. Add a `main_tabs_.Add(...)` call in the constructor.
3. Add a `ToolBarTabN(Bar&)` function in `ToolBarSetup.cpp`.
4. Add a `case N:` in `UpdateToolBar()` and `MenuEdit()`.
5. Add a `case N:` in `sTabLayoutKey()` in `MainWindow.cpp`.

## Old working labels

Labels such as `desktop-parser`, `desktop-website`, and `desktop-diary` are
historical planning notes. They are not final package names and must not appear
in code derived from this template.
