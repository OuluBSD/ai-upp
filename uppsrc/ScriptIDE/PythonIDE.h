#ifndef _ScriptIDE_PythonIDE_h_
#define _ScriptIDE_PythonIDE_h_

class PluginManager;

class PythonIDE : public DockWindow {
public:
	typedef PythonIDE CLASSNAME;

	PythonIDE();
	~PythonIDE();

	struct FileInfo {
		String path;
		bool   dirty = false;
		String content;
		Ptr<Ctrl> editor;
		bool   is_plugin = false;
	};

	Array<FileInfo> open_files;
	int             active_file = -1;
	Ptr<Ctrl>       active_editor;

	MenuBar         menubar;
	ToolBar         toolbar;
	StatusBar       statusbar;
	
	ParentCtrl      editor_area;
	CustomFileTabs  editor_tabs;

	PyVM            vm;
	RunManager      run_manager;
	PathManager     path_manager;
	
	IDESettings     settings;
	
	PythonConsole   console_pane;
	FilesPane       files_pane;
	VariableExplorer var_explorer;
	PlotsPane       plots_pane;
	DebuggerPane    debugger_pane;
	ProfilerPane    profiler_pane;
	FindInFilesPane find_pane;
	OutlinePane     outline_pane;
	HelpPane        help_pane;
	HistoryPane     history_pane;

	DockableCtrl    context_pane_left;
	DockableCtrl    context_pane_right;

	One<PluginManager> plugin_manager;
	ArrayMap<String, One<DockableCtrl>> plugin_panes;

	void InitLayout();
	void InitDocking();
	virtual void DockInit() override;
	
	void OnNewFile();
	void OnOpenFile();
	void LoadFile(const String& path);
	void OnSaveFile();
	void OnSaveFileAs();
	void OnSaveAll();
	void SaveFile(int idx);
	bool ConfirmSave(int idx);
	bool ConfirmSaveAll();
	
	void OnCloseFile();
	void OnCloseAll();
	void OnTabChanged();
	void OnTabMenu(Bar& bar);
	void SyncTabsWithFiles();
	
	void OnUndo();
	void OnRedo();
	void OnComment();
	void OnBlockComment();
	void OnUncomment();
	void OnToggleCase(bool upper);
	void OnConvertEOL(const String& mode);
	void OnRemoveTrailingSpaces();
	void OnTabsToSpaces();
	
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
	void OnStepOver();
	void OnStepIn();
	void OnStepOut();
	void OnToggleBreakpoint();
	void OnClearBreakpoints();
	void OnListBreakpoints();
	
	void OnConsoleInput();
	void OnPathManager();
	
	void MainMenu(Bar& bar);
	void MainToolbar(Bar& bar);
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

	void SyncPluginPanes();
	void OnTogglePane(DockableCtrl& pane);
	void OnMaximizePane();
	void OnClosePane();
	
	void OnLayoutDefault();
	void OnLayoutRstudio();
	void OnLayoutMatlab();
	void OnFullscreen();
	
	void UpdateStatusBar();
	void UpdateVariableExplorer();
	void ApplySettings();
	void OnAnalyze();
	void OnRestart();
	void OnOpenFileSwitcher();
	void OnSymbolFinder();
	void OnOpenLastClosed();
	void OnSaveCopyAs();
	void OnRevert();
	void OnFileSwitcher();
	void UpdateRecentFilesMenu(Bar& bar);
	void AddRecentFile(const String& path);
	
	void OnBreakpointHit(const String& file, int line);
	void ShowHelp(const String& topic);
	void OnSettings();
	
	void OnPrevCursor();
	void OnNextCursor();
	void AddCursorHistory();

	void Log(const String& s) { console_pane.Write(s); }
	void Error(const String& s) { console_pane.WriteError(s); }

	virtual void Close() override;
	virtual void Serialize(Stream& s) override;
	
	static size_t MemoryUsedKb();
	static size_t MemoryTotalKb();
};

#endif
