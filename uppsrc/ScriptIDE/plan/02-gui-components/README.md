# Track 2: GUI Components

Build the complete Spyder-like IDE interface.

## Phases

### Phase 1: Main Window Layout
Create main window with Splitter-based layout:
- Left sidebar (collapsible)
- Center area (editor tabs)
- Right area split (top/bottom panels)

### Phase 2: Editor Area
- Custom FileTabs with "New Tab" button
- Custom FileTabs with menu button (three lines)
- Optional bottom tab placement
- File path label above tabs
- CodeEditor with Python syntax highlighting

### Phase 3: Right Panels
- Top tabs: Help, Variable Explorer, Plots, Files
- Bottom tabs: IPython Console, History
- Tab placement at bottom

### Phase 4: Left Sidebar
- ToolBar with icons
- TreeCtrl showing .py files, classes, functions
- Special comment markers (# -- TEXT)

### Phase 5: Status/Menu/ToolBar
- MenuBar: File, Edit, Search, Source, Run, Debug, Consoles, Projects, Tools, View, Help
- ToolBar: New, Open, Save, Run, Debug, Step controls, WorkArea droplist
- StatusBar: Line/Col, Format, Line Ending, Edit Mode, Memory %

## UI Layout Structure

```
┌─────────────────────────────────────────────────────────────┐
│ MenuBar: File Edit Search Source Run Debug ... Help        │
├─────────────────────────────────────────────────────────────┤
│ ToolBar: [New][Open][Save] [Run][Debug][Step▷] [WorkDir▾] │
├──┬────────────────────────────────────┬─────────────────────┤
│  │ [/path/to/current/file.py]        │ [Help][VarExp]...  │
│S │ ┌────────────────────────────┐    ├─────────────────────┤
│i │ │  editor content            │    │  Right-Top Panel   │
│d │ │  (CodeEditor)              │    │                     │
│e │ │                            │    │                     │
│b │ └────────────────────────────┘    │                     │
│a │ [file.py][main.py][x]       │    ├─────────────────────┤
│r │  └─ tabs at bottom (optional)     │ Right-Bottom Panel │
│  │                                    │ [Console][History] │
│  │                                    │  └─ tabs at bottom │
├──┴────────────────────────────────────┴─────────────────────┤
│ Line: 10  Col: 5  UTF-8  LF  RW  Mem: 45%                  │
└─────────────────────────────────────────────────────────────┘
```

## Success Criteria
- [ ] All panels visible and resizable
- [ ] Editor tabs work with custom buttons
- [ ] Python syntax highlighting active
- [ ] StatusBar shows realtime info
- [ ] All menus/toolbar buttons present
