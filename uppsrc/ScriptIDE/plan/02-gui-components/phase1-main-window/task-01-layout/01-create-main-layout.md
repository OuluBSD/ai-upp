# Task: Create Main Window DockWindow Layout

## Goal
Implement Spyder-like layout: DockLeft(FileTree) | Splitters(Editor | RightPanels).

## Implementation in PythonIDE.h

```cpp
class PythonIDE : public DockWindow {
public:
    typedef PythonIDE CLASSNAME;

    PythonIDE();
    virtual void DockInit() override;
    virtual void Close() override;

private:
    // Layout components
    MenuBar menubar;
    ToolBar toolbar;
    StatusBar statusbar;

    // Left dockable panel
    ToolBar file_toolbar;
    TreeCtrl file_tree;
    WithFileTreeLayout<DockableCtrl> file_panel;  // Contains toolbar + tree

    // Main center-right area (Splitter)
    Splitter main_split;          // Horizontal: editor | right_split

    // Center area - Editor
    ParentCtrl editor_area;

    // Right area (top/bottom split)
    Splitter right_split;         // Vertical: top | bottom

    ParentCtrl right_top;
    ParentCtrl right_bottom;

    void InitLayout();
};
```

## Implementation in PythonIDE.cpp

```cpp
void PythonIDE::InitLayout()
{
    // Main horizontal split: editor | right panels
    main_split.Horz();
    main_split << editor_area << right_split;
    main_split.SetPos(7000);  // 70% editor, 30% right

    // Right vertical split: top | bottom
    right_split.Vert();
    right_split << right_top << right_bottom;
    right_split.SetPos(5000);  // 50/50 split

    // Add main splitter to window
    Add(main_split.SizePos());
}

PythonIDE::PythonIDE()
{
    Title("ScriptIDE - Python IDE");
    Sizeable().Zoomable().CenterScreen();
    SetRect(0, 0, 1400, 900);

    AddFrame(menubar);
    AddFrame(toolbar);
    AddFrame(statusbar);

    InitLayout();

    // Setup file tree panel (layout done in DockInit)
    file_panel.Add(file_toolbar.TopPos(0, 24).HSizePos());
    file_panel.Add(file_tree.VSizePos(24, 0).HSizePos());
    file_panel.Title("Files");
    file_panel.Icon(CtrlImg::Dir());
}

void PythonIDE::DockInit()
{
    // Register and dock the file tree to the left
    Register(file_panel.SizeHint(Size(250, 400)));
    DockLeft(file_panel);

    // Try to load saved layout
    FileIn in(ConfigFile("docking-layout.bin"));
    if(in.IsOpen() && !in.IsError())
        SerializeWindow(in);
}

void PythonIDE::Close()
{
    // Save layout before closing
    FileOut out(ConfigFile("docking-layout.bin"));
    if(out.IsOpen())
        SerializeWindow(out);

    DockWindow::Close();
}
```

## Files Modified
- `uppsrc/ScriptIDE/PythonIDE.h`
- `uppsrc/ScriptIDE/PythonIDE.cpp`

## Testing
Build and run:
```bash
script/build.py -mc 1 -j 12 ScriptIDE
./bin/ScriptIDE
```

Verify:
- Three-panel layout visible
- Splitters are draggable
- Layout persists across restarts

## Success Criteria
- Main window shows 3-column layout
- Splitters work correctly
- Proportions are reasonable (20/70/30)
