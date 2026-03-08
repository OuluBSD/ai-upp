# Task: Implement MenuBar

## Goal
Create Spyder-like menu structure: File, Edit, Search, Source, Run, Debug, Consoles, Projects, Tools, View, Help.

## Implementation in PythonIDE

```cpp
// In PythonIDE.h
class PythonIDE : public TopWindow {
    void FileMenu(Bar& bar);
    void EditMenu(Bar& bar);
    void SearchMenu(Bar& bar);
    void SourceMenu(Bar& bar);
    void RunMenu(Bar& bar);
    void DebugMenu(Bar& bar);
    void ConsolesMenu(Bar& bar);
    void ProjectsMenu(Bar& bar);
    void ToolsMenu(Bar& bar);
    void ViewMenu(Bar& bar);
    void HelpMenu(Bar& bar);

    void MainMenu(Bar& bar);
};

// In PythonIDE.cpp
void PythonIDE::MainMenu(Bar& bar)
{
    bar.Add("File", THISBACK(FileMenu));
    bar.Add("Edit", THISBACK(EditMenu));
    bar.Add("Search", THISBACK(SearchMenu));
    bar.Add("Source", THISBACK(SourceMenu));
    bar.Add("Run", THISBACK(RunMenu));
    bar.Add("Debug", THISBACK(DebugMenu));
    bar.Add("Consoles", THISBACK(ConsolesMenu));
    bar.Add("Projects", THISBACK(ProjectsMenu));
    bar.Add("Tools", THISBACK(ToolsMenu));
    bar.Add("View", THISBACK(ViewMenu));
    bar.Add("Help", THISBACK(HelpMenu));
}

void PythonIDE::FileMenu(Bar& bar)
{
    bar.Add("New File", ScriptIDEImg::IconNewFile(), [=] { OnNewFile(); })
        .Key(K_CTRL_N);
    bar.Add("Open...", ScriptIDEImg::IconOpenFile(), [=] { OnOpenFile(); })
        .Key(K_CTRL_O);
    bar.Separator();
    bar.Add("Save", ScriptIDEImg::IconSave(), [=] { OnSaveFile(); })
        .Key(K_CTRL_S);
    bar.Add("Save As...", ScriptIDEImg::IconSaveAs(), [=] { OnSaveFileAs(); })
        .Key(K_CTRL_SHIFT_S);
    bar.Separator();
    bar.Add("Exit", [=] { Close(); })
        .Key(K_ALT_F4);
}

void PythonIDE::RunMenu(Bar& bar)
{
    bar.Add("Run", ScriptIDEImg::IconRun(), [=] { OnRun(); })
        .Key(K_F5);
    bar.Add("Run Selection", [=] { OnRunSelection(); })
        .Key(K_F9);
    bar.Separator();
    bar.Add("Configuration per file...", [=] { OnRunConfig(); });
}

void PythonIDE::DebugMenu(Bar& bar)
{
    bar.Add("Debug", ScriptIDEImg::IconDebug(), [=] { OnDebug(); })
        .Key(K_CTRL_F5);
    bar.Separator();
    bar.Add("Step Over", ScriptIDEImg::IconStepOver(), [=] { OnStepOver(); })
        .Key(K_F10);
    bar.Add("Step Into", ScriptIDEImg::IconStepIn(), [=] { OnStepIn(); })
        .Key(K_F11);
    bar.Add("Step Out", ScriptIDEImg::IconStepOut(), [=] { OnStepOut(); })
        .Key(K_SHIFT_F11);
    bar.Separator();
    bar.Add("Toggle Breakpoint", ScriptIDEImg::IconBreakpoint(), [=] { OnToggleBreakpoint(); })
        .Key(K_F9);
}

void PythonIDE::ViewMenu(Bar& bar)
{
    bar.Add("Variable Explorer", [=] { right_top_tabs.Set(0); });
    bar.Add("Help", [=] { right_top_tabs.Set(1); });
    bar.Add("Files", [=] { right_top_tabs.Set(2); });
    bar.Separator();
    bar.Add("IPython Console", [=] { right_bottom_tabs.Set(0); });
    bar.Add("History", [=] { right_bottom_tabs.Set(1); });
}

// Constructor
PythonIDE::PythonIDE()
{
    // ... existing code ...
    menubar.Set(THISBACK(MainMenu));
}
```

## Menu Actions (Stubs)

```cpp
void PythonIDE::OnNewFile() { /* TODO */ }
void PythonIDE::OnOpenFile() { /* TODO */ }
void PythonIDE::OnSaveFile() { /* TODO */ }
void PythonIDE::OnSaveFileAs() { /* TODO */ }
void PythonIDE::OnRun() { /* TODO: Track 3 */ }
void PythonIDE::OnRunSelection() { /* TODO */ }
void PythonIDE::OnDebug() { /* TODO: Track 3 */ }
void PythonIDE::OnStepOver() { if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) vm.StepOver(); }
void PythonIDE::OnStepIn() { if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) vm.StepIn(); }
void PythonIDE::OnStepOut() { if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) vm.StepOut(); }
void PythonIDE::OnToggleBreakpoint() { /* TODO */ }
```

## Files Modified
- `uppsrc/ScriptIDE/PythonIDE.h`
- `uppsrc/ScriptIDE/PythonIDE.cpp`

## Success Criteria
- All menus visible
- Keyboard shortcuts work
- Menu items trigger correct actions
- View menu switches tabs correctly
