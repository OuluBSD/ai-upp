# Task: Refactor Right Panels to Docking Windows

## Goal
Move the right-side TabCtrls and splitters to U++ `Docking` system.
- "Variable Explorer", "Help", "Plots", "Files" should be separate docking windows, stacked top-right.
- "IPython Console" and "History" should be separate docking windows, stacked bottom-right.

## Strategy

1.  **Wrap Components in DockableCtrl**:
    Each component (VariableExplorer, Help Viewer, PythonConsole, etc.) should be wrapped in a `DockableCtrl` or similar. Since `PythonIDE` already has these as members, we can use `DockableCtrl` wrappers.

2.  **Register and Dock in `DockInit`**:
    In `PythonIDE::DockInit`, register each panel and use `DockRight`, `DockBottom`, etc., with stacking.

3.  **Remove Splitters**:
    Remove `main_split` and `right_split` from `PythonIDE.h` and `PythonIDE.cpp`. The `editor_area` should be the main center control.

## Implementation Details

### PythonIDE.h
```cpp
    // Panels wrapped in DockableCtrl
    WithDockable<VariableExplorer> var_panel;
    WithDockable<RichTextCtrl> help_panel;
    WithDockable<ParentCtrl> plots_panel;
    WithDockable<ParentCtrl> files_panel;
    WithDockable<PythonConsole> console_panel;
    WithDockable<ParentCtrl> history_panel;
```
Note: `WithDockable` is a common pattern in U++ Docking.

### PythonIDE.cpp - InitLayout
Remove `main_split`, `right_split`, `right_top`, `right_bottom`, `right_top_tabs`, `right_bottom_tabs`.
Add `editor_area` directly to the window (it will be the central area if using `DockWindow` correctly).

### PythonIDE.cpp - DockInit
```cpp
void PythonIDE::DockInit()
{
    // ... file_panel ...

    Register(var_panel.Title("Variables").SizeHint(Size(300, 400)));
    Register(help_panel.Title("Help").SizeHint(Size(300, 400)));
    Register(plots_panel.Title("Plots").SizeHint(Size(300, 400)));
    Register(files_panel.Title("Files").SizeHint(Size(300, 400)));

    Register(console_panel.Title("Console").SizeHint(Size(600, 300)));
    Register(history_panel.Title("History").SizeHint(Size(600, 300)));

    // Dock Top-Right Stack
    DockRight(var_panel);
    DockRight(help_panel, var_panel, DOCK_STACK);
    DockRight(plots_panel, help_panel, DOCK_STACK);
    DockRight(files_panel, plots_panel, DOCK_STACK);

    // Dock Bottom-Right Stack
    DockBottom(console_panel, var_panel, DOCK_TABBED); // Or similar logic to place below
    DockBottom(history_panel, console_panel, DOCK_STACK);
}
```
*Self-correction: U++ Docking API details might vary, need to verify exact methods during implementation.*

## Files Modified
- `uppsrc/ScriptIDE/PythonIDE.h`
- `uppsrc/ScriptIDE/PythonIDE.cpp`

## Success Criteria
- Right panels are now independent docking windows.
- Panels can be floated, closed, or moved.
- Layout is preserved across restarts (already implemented via `SerializeWindow`).
- No more fixed splitters on the right side.
