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

    // Center area - Editor
    ParentCtrl editor_area;
    CustomFileTabs editor_tabs;

    // Dockable panes
    FilesPane files_pane;
    VariableExplorer var_explorer;
    WithDockable<RichTextCtrl> help_pane;
    PlotsPane plots_pane;
    PythonConsole console_pane;
    WithDockable<ParentCtrl> history_pane;
    WithDockable<ParentCtrl> find_pane;

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

    struct FileInfo {
        String path;
        bool dirty = false;
    } current_file;
};

#endif
