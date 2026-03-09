#ifndef _ScriptIDE_PythonIDE_h_
#define _ScriptIDE_PythonIDE_h_

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
    FileTree file_tree;
    WithFileTreeLayout<DockableCtrl> file_panel;  // Contains toolbar + tree

    // Center area - Editor
    ParentCtrl editor_area;
    CustomFileTabs editor_tabs;

    // Dockable panels
    DockableCtrl var_dock;
    DockableCtrl help_dock;
    DockableCtrl plots_dock;
    DockableCtrl files_dock;
    DockableCtrl console_dock;
    DockableCtrl history_dock;

    void InitLayout();
    void InitDocking();


    void OnNewTab();
    void OnTabMenu(Bar& bar);

    void OnConsoleInput();

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

    void UpdateStatusBar();
    void UpdateVariableExplorer();

    void LoadFile(const String& path);
    void SaveFile(const String& path);
    bool ConfirmSave();
    void ShowHelp(const String& topic);

    static size_t MemoryUsedKb();
    static size_t MemoryTotalKb();
    CodeEditor* GetCurrentEditor() { return &code_editor; }

    void OnNewFile();
    void OnOpenFile();
    void OnSaveFile();
    void OnSaveFileAs();
    void OnRun();
    void OnRunSelection();
    void OnRunConfig();
    void OnSettings();
    void ApplySettings();
    void OnDebug();
    void OnBreakpointHit(const String& file, int line);
    void OnStepOver();
    void OnStepIn();
    void OnStepOut();
    void OnToggleBreakpoint();

    PyVM vm;

    struct StatusInfo {
        int line = 1;
        int column = 1;
        String format = "UTF-8";
        String line_ending = "LF";
        String edit_mode = "RW";
        int memory_percent = 0;
    } status_info;

    PythonIDESettings settings;

    CodeEditor code_editor;
    VariableExplorer var_explorer;

    RichTextCtrl help_viewer;
    PlotsPane plots_viewer;
    ParentCtrl files_viewer;
    ParentCtrl history_viewer;
    PythonConsole python_console;

    struct FileInfo {
        String path;
        bool dirty = false;
    } current_file;
};

#endif
