#ifndef _ScriptIDE_PythonIDE_h_
#define _ScriptIDE_PythonIDE_h_

class PythonIDE : public DockWindow {
public:
    typedef PythonIDE CLASSNAME;

    PythonIDE();
    virtual void DockInit() override;
    virtual void Close() override;

    void OnNewFile();
    void OnOpenFile();
    void OnSaveFile();
    void OnSaveFileAs();
    void OnSaveAll();
    void OnCloseFile();
    void OnCloseAll();
    void OnFileSwitcher();
    void OnSymbolFinder();
    void OnRestart();

    void OnUndo();
    void OnRedo();
    void OnComment();
    void OnBlockComment();
    void OnUncomment();
    void OnToggleCase(bool upper);
    void OnConvertEOL(const String& mode);
    void OnRemoveTrailingSpaces();
    void OnTabsToSpaces();

    void LoadFile(const String& path);
    void SaveFile(int idx);
    bool ConfirmSave(int idx);
    bool ConfirmSaveAll();
    void ShowHelp(const String& topic);
    void OnPathManager();

    void OnRun();
    void OnRunLast();
    void OnRunSelection();
    void OnRunCell();
    void OnRunCellAndAdvance();
    void OnRunToLine();
    void OnRunFromLine();
    void OnRunConfig();
    
    void OnDebug();
    void OnDebugCell();
    void OnDebugSelection();
    void OnDebugToLine();
    void OnStop();
    void OnToggleBreakpoint();
    void OnClearBreakpoints();
    void OnListBreakpoints();
    void OnConsoleInput();

    void ApplySettings();
    void OnSettings();
    void OnBreakpointHit(const String& file, int line);
    void OnStepOver();
    void OnStepIn();
    void OnStepOut();

    void OnTabChanged();
    void OnTabMenu(Bar& bar);
    void SyncTabsWithFiles();

    void OnTogglePane(DockableCtrl& pane);
    void OnLayoutDefault();
    void OnLayoutRstudio();
    void OnLayoutMatlab();
    void OnFullscreen();
    void OnMaximizePane();
    void OnClosePane();

    void InitLayout();
    void InitDocking();

    void FileMenu(Bar& bar);
    void EditMenu(Bar& bar);
    void SearchMenu(Bar& bar);
    void SourceMenu(Bar& bar);
    void RunMenu(Bar& bar);
    void DebugMenu(Bar& bar);
    void ConsolesMenu(Bar& bar);
    void ProjectsMenu(Bar& bar);
    void ToolsMenu(Bar& bar);
    void WindowMenu(Bar& bar);
    void HelpMenu(Bar& bar);

    void MainMenu(Bar& bar);
    void MainToolbar(Bar& bar);

    void Todo(const String& msg);

    void UpdateStatusBar();
    void UpdateVariableExplorer();
    void OnAnalyze();

    static size_t MemoryUsedKb();
    static size_t MemoryTotalKb();
    CodeEditor* GetCurrentEditor() { return &code_editor; }

    MenuBar menubar;
    ToolBar toolbar;
    StatusBar statusbar;

    ParentCtrl editor_area;
    CustomFileTabs editor_tabs;

    FilesPane files_pane;
    OutlinePane outline_pane;
    VariableExplorer var_explorer;
    DebuggerPane debugger_pane;
    ProfilerPane profiler_pane;
    WithDockable<RichTextCtrl> help_pane;
    PlotsPane plots_pane;
    PythonConsole console_pane;
    WithDockable<ParentCtrl> history_pane;
    FindInFilesPane find_pane;

    PyVM vm;
    RunManager run_manager;
    PathManager path_manager;
    Linter linter;

    struct StatusInfo {
        int line = 1;
        int column = 1;
        String format = "UTF-8";
        String line_ending = "LF";
        String edit_mode = "RW";
        int memory_percent = 0;
    } status_info;

    IDESettings settings;

    PythonEditor code_editor;

    void OnPrevCursor();
    void OnNextCursor();
    void AddCursorHistory();

    struct FileInfo {
        String path;
        String content;
        bool dirty = false;
    };
    Array<FileInfo> open_files;
    int active_file = -1;

    Vector<int64> cursor_history;
    int cursor_history_idx = -1;
};

#endif
