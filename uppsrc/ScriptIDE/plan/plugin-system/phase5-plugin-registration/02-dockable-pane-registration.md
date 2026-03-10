# Task: Dockable Pane Registration

## Goal
Design the mechanism for plugins to register custom dockable panes that integrate seamlessly into the ScriptIDE layout and Window menu.

## Background / Rationale
Plugins may need persistent side-panels (e.g., a "Game State Inspector" or a "Card Library" for the `.form` editor). These panes must behave like native IDE panes: they should be dockable, tabbed, floatable, and toggleable via the Window menu.

## Scope
- Defining the `IDockPaneProvider` interface.
- Defining how the IDE discovers and lists plugin-provided panes in the Window menu.
- Defining persistence for plugin pane layouts (saving dock state across sessions).

## Non-goals
- Modifying the underlying U++ `Docking` package.

## Dependencies
- `01-plugin-lifecycle-manifest.md`

## Concrete Investigation Steps
1. Review how existing panes (like `FilesPane` or `HistoryPane`) are initialized and docked in `PythonIDE::InitLayout` and `PythonIDE::DockInit`.
2. Review how `PythonIDE::WindowMenu` constructs the list of panes.
3. Design a dynamic registration process in `PluginManager` that hooks into these IDE mechanisms.

## Affected Subsystems
- `uppsrc/ScriptIDE/PluginInterfaces.h`
- `uppsrc/ScriptIDE/PythonIDE.cpp` (Menu and Docking logic)

## Implementation Direction
Create an architecture plan. Sketch how `IDockPaneProvider::GetPaneCtrl()` is called and how the returned `Ctrl` is wrapped in a `DockableCtrl` and added to the IDE's layout manager.

## Risks
- Unique IDs for panes might collide if not properly scoped by the plugin name.
- Restoring layouts from `docking-layout.bin` might crash if a plugin providing a saved pane is disabled.

## Acceptance Criteria
- [ ] Documented `IDockPaneProvider` interface.
- [ ] Defined Window menu integration for dynamic panes.
- [ ] Documented layout persistence strategy for plugin panes, including safe fallback when disabled.
