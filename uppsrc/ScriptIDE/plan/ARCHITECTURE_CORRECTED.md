# ScriptIDE Architecture - DockWindow Edition

## Corrected Architecture Based on User Feedback

### Base Class
**PythonIDE inherits from `DockWindow`** (not TopWindow)

Reference: `reference/DockingExample1` and `reference/DockingExample2`

### Layout Structure

```
PythonIDE (DockWindow)
│
├── MenuBar
├── ToolBar
├── StatusBar
│
├── DockLeft: file_panel (DockableCtrl)
│   ├── file_toolbar (ToolBar)
│   └── file_tree (TreeCtrl)
│
└── main_split (Splitter - Horizontal)
    │
    ├── editor_area (ParentCtrl)
    │   ├── file_path_label
    │   ├── editor_tabs (CustomFileTabs)
    │   └── code_editor (CodeEditor)
    │
    └── right_split (Splitter - Vertical)
        │
        ├── right_top (TabCtrl)
        │   ├── Help
        │   ├── Variable Explorer
        │   ├── Plots
        │   └── Files
        │
        └── right_bottom (TabCtrl)
            ├── IPython Console
            └── History
```

### Key Components

#### 1. DockWindow Features
- **file_panel**: DockableCtrl on left side
  - Can be closed/reopened via Windows menu
  - Position persists via SerializeWindow()
  - User can undock to floating window

#### 2. Splitter Main Area
- **main_split**: Editor | Right panels (horizontal)
- **right_split**: Top tabs | Bottom tabs (vertical)
- Splitter positions serialized separately

#### 3. Layout Persistence
```cpp
void PythonIDE::DockInit()
{
    // Register dockable panels
    Register(file_panel.SizeHint(Size(250, 400)));
    DockLeft(file_panel);

    // Load saved layout
    FileIn in(ConfigFile("docking-layout.bin"));
    if(in.IsOpen())
        SerializeWindow(in);  // Restores dock + splitter state
}

void PythonIDE::Close()
{
    // Save layout
    FileOut out(ConfigFile("docking-layout.bin"));
    if(out.IsOpen())
        SerializeWindow(out);
    DockWindow::Close();
}
```

### Right Panels - Current Design

**Not Dockable (for now)**:
- Right top/bottom panels stay in fixed Splitters
- Simpler implementation initially
- Could be upgraded to DockableCtrl later if needed

### File Tree Panel Layout

```
┌─────────────────┐
│ [Toolbar]       │ ← file_toolbar (New, Refresh, etc.)
├─────────────────┤
│ 📁 project/     │
│   📄 main.py    │ ← file_tree (TreeCtrl)
│   📄 utils.py   │
│   📁 tests/     │
└─────────────────┘
```

### Dependencies (Corrected)

ScriptIDE.upp:
```
uses
    CtrlLib,
    CodeEditor,
    TabBar,
    ByteVM,
    RichEdit,
    Docking;    ← ADDED
```

### DockWindow vs TopWindow Differences

| Feature | TopWindow | DockWindow |
|---------|-----------|------------|
| Base class | TopWindow | DockWindow (extends TopWindow) |
| Docking support | ❌ No | ✅ Yes |
| DockInit() | ❌ | ✅ Required |
| Register() | ❌ | ✅ For dockable panels |
| SerializeWindow() | ❌ | ✅ Saves dock state |
| DockManager() | ❌ | ✅ Built-in UI |

### Future Extensibility

User mentioned:
> "we might be able to add DockWindow to splitter though"

This means we could make right panels dockable later:
```cpp
// Future: Make Variable Explorer dockable
Register(Dockable(var_explorer, "Variable Explorer"));
DockRight(var_explorer);
```

But for initial implementation: **Splitters for main area, DockLeft for file tree only**.

### Corrected Build Dependencies

```
Docking package depends on:
├── CtrlLib
└── (no other packages)

ScriptIDE now depends on:
├── Docking  (for DockWindow)
├── CtrlLib  (UI)
├── CodeEditor
├── TabBar
├── ByteVM
└── RichEdit
```

No circular dependencies - good to go!

### Testing DockWindow

After implementing:
```bash
script/build.py -mc 1 -j 12 ScriptIDE
./bin/ScriptIDE

# Test docking:
1. Drag file tree panel to floating window
2. Close it
3. Open via Windows menu → Files
4. Close app, reopen → layout should restore
```

### Key Takeaways

✅ Use `DockWindow` as base class
✅ Left file tree in `DockableCtrl`
✅ Main area uses `Splitter` (not docking)
✅ Right panels in fixed `Splitter` (not dockable yet)
✅ SerializeWindow() to ConfigFile for persistence
✅ Add `Docking` to package dependencies

This matches the reference examples perfectly!
