# ScriptIDE Implementation Summary

## Overview
ScriptIDE is a Spyder-like Python IDE built on Ultimate++ using the internal ByteVM Python interpreter.

## Architecture

### Core Components

1. **PythonIDE** (Main Window)
   - TopWindow with MenuBar, ToolBar, StatusBar
   - Splitter-based layout (3-column)
   - Manages all sub-components

2. **ByteVM Integration**
   - Python interpreter with debugging support
   - Breakpoints, stepping, call stack
   - Variable inspection

3. **Editor Area**
   - CustomFileTabs with New/Menu buttons
   - CodeEditor with Python syntax highlighting
   - File path label above tabs

4. **Variable Explorer**
   - ArrayCtrl showing stack variables
   - Type-based color coding (hash→hue)
   - Name/Type/Size/Value columns

5. **Console**
   - IPython-style console
   - Command history
   - Output display

6. **File Tree**
   - TreeCtrl for .py files
   - Class/function navigation
   - Special comment markers

## Layout Structure

```
┌─────────────────────────────────────────────────────────────┐
│ MenuBar                                                      │
├─────────────────────────────────────────────────────────────┤
│ ToolBar                                                      │
├──┬────────────────────────────────────┬─────────────────────┤
│D │ File Path Label                   │ [Help][VarExp]...  │
│o │ ┌────────────────────────────┐    ├─────────────────────┤
│c │ │ CodeEditor                 │    │ TabCtrl            │
│k │ │  (Python syntax)           │    │  - Help            │
│e │ └────────────────────────────┘    │  - Variable Exp    │
│d │ [+] [file1][file2][x] [≡]    │    │  - Plots           │
│  │      └─ tabs (bottom optional)     │  - Files           │
│F │          ↑                         ├─────────────────────┤
│i │        Splitter                    │ TabCtrl            │
│l │          ↓                         │  - IPython Console │
│e │                                    │  - History         │
│s │                                    │  └─ tabs at bottom │
│  │                                    │        ↑           │
│  │                                    │     Splitter       │
├──┴────────────────────────────────────┴─────────────────────┤
│ StatusBar: Line Col Format LineEnd Mode Mem%               │
└─────────────────────────────────────────────────────────────┘

Layout: DockLeft(FileTree) | Splitter(Editor | Splitter(TopTabs | BottomTabs))
```

## Implementation Tracks

### Track 1: Foundation (Priority 1)
- Package setup
- Basic UI framework
- **ByteVM debugging enhancements** (critical)

### Track 2: GUI Components (Priority 2)
- Main window layout
- Editor area with custom tabs
- Right panels (top/bottom)
- Left sidebar
- Status/Menu/ToolBar

### Track 3: Python Integration (Priority 3)
- Console implementation
- Execution engine
- Debugging integration

### Track 4: IDE Features (Priority 4)
- Variable explorer
- File management
- Help system
- Settings dialog

## Critical Path

1. **ByteVM Debugging** must be completed first
   - All IDE features depend on debugger
   - Breakpoints, stepping, variable access

2. **GUI Layout** can proceed in parallel
   - Basic structure independent of ByteVM

3. **Integration** happens after both complete
   - Wire debugger to Variable Explorer
   - Connect console to ByteVM
   - Link editor breakpoints to ByteVM

## Key Design Decisions

### 1. DockWindow + Splitter Hybrid Layout
We use **DockWindow** from the Docking package for the left file tree:
- Left panel is DockableCtrl (can be closed, reopened, repositioned)
- Main area uses Splitters for editor and right panels
- SerializeWindow() persists docking + splitter layout
- ConfigFile("docking-layout.bin") stores layout state

### 2. Custom FileTabs
Extended FileTabs to add:
- "New Tab" button (left)
- Menu button (right)
- Bottom tab placement option
- File path label integration

### 3. Type-Based Coloring
Variable Explorer uses hash-to-hue mapping:
- Hash type name → 0-360 hue
- Low saturation (15%) for readability
- Consistent colors for same type

### 4. ByteVM-Only
**No external Python runtime**. Only uppsrc/ByteVM interpreter used:
- Simpler deployment
- Full control over debugging
- Integrated solution

## Dependencies

```
ScriptIDE
├── CtrlLib (UI controls)
├── CodeEditor (syntax highlighting)
├── TabBar/FileTabs (tab management)
├── ByteVM (Python interpreter)
│   └── Core
├── RichEdit (help system)
└── Docking (DockWindow, DockableCtrl)
    └── CtrlLib
```

## Build Command

```bash
script/build.py -mc 1 -j 12 ScriptIDE
```

## Testing Strategy

1. **Unit Tests** for ByteVM debugging
2. **Integration Tests** for editor-debugger
3. **Manual Tests** for UI/UX
4. **End-to-End** scenarios (edit→run→debug)

## Settings Persistence

All settings serialized via `PythonIDESettings`:
- Editor font
- Console font
- Layout (splitter positions)
- Recent files
- Breakpoints
- Work directory

## Future Enhancements (Post-MVP)

- Matplotlib/plotting support
- Package management
- Code completion (via static analysis)
- Profiler integration
- Git integration
- Jupyter notebook support
- Remote debugging

## Timeline Estimate

- **Track 1 (Foundation)**: 2-3 days
- **Track 2 (GUI)**: 2-3 days
- **Track 3 (Python)**: 2-3 days
- **Track 4 (Features)**: 2-3 days
- **Track 8 (Plugin System)**: 2 days (COMPLETED)
  - Modular architecture for custom file types and panes
  - Reference implementation (GameStatePlugin)

**Total: ~10-14 days** for full implementation

## Plugin System (Track 08)

### Architecture
The Plugin System allows extending ScriptIDE without modifying the core. It uses a registry-based approach:
- **IPlugin**: Base interface for all plugins.
- **IPluginContext**: Provides plugins with access to the IDE and VM.
- **IPluginRegistry**: Allows plugins to register handlers for file types, dock panes, and VM bindings.

### Capabilities
1. **Custom Document Hosts**: Plugins can handle new file extensions (e.g., `.gamestate`) by providing a custom `Ctrl` that implements `IDocumentHost`.
2. **Dockable Panes**: Plugins can register new panes that integrate with the U++ `Docking` system and appear in the `Window` menu.
3. **VM Bindings**: Plugins can inject C++ functions into the ByteVM globals (e.g., `get_game_score()`).
4. **Custom Execution**: Plugins can intercept the "Run" command for specific files.

### Implementation Details
- **PluginManager**: Manages the lifecycle (`Init`, `Shutdown`) and discovery of plugins.
- **Discovery**: Uses a static registration macro `REGISTER_PLUGIN(T)`.
- **UI Integration**: `PythonIDE` was refactored to host arbitrary `Ctrl`s using `FileInfo`.
- **Preferences**: A dedicated "Plugins" page in the Settings dialog allows enabling/disabling plugins at runtime.

## Risk Factors

1. **ByteVM Debugging Complexity**
   - May need deeper changes than planned
   - Source location tracking might be challenging
   - **Mitigation**: Start with ByteVM first, validate early

2. **Custom Tab Controls**
   - FileTabs extension might conflict with parent behavior
   - **Mitigation**: Study TabBar implementation thoroughly

3. **Console Integration**
   - IPython-like features might be complex
   - **Mitigation**: Start simple, add features incrementally

## Success Metrics

- [ ] Can edit Python files with syntax highlighting
- [ ] Can run Python scripts via ByteVM
- [ ] Can set breakpoints and pause execution
- [ ] Can step through code (over/in/out)
- [ ] Can inspect variables in Variable Explorer
- [ ] Console shows output and accepts input
- [ ] File tree navigates project structure
- [ ] Help system displays Python documentation
- [ ] Settings persist across sessions
- [ ] Memory-efficient (< 100MB for basic usage)
