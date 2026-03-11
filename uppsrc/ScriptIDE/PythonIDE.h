#ifndef _ScriptIDE_PythonIDE_h_
#define _ScriptIDE_PythonIDE_h_

class PluginManager;

class PythonIDE : public DockWindow, public IDEContext {
public:
	typedef PythonIDE CLASSNAME;

	PythonIDE();
	~PythonIDE();

	struct FileInfo {
		String path;
		bool   dirty = false;
		String content;
		IDocumentHost* editor = nullptr;
		bool   is_plugin = false;
	};

	Array<FileInfo> open_files;
	int             active_file = -1;
	Ctrl*           active_editor = nullptr;
	DockableCtrl*   active_pane = nullptr;
	String          last_run_path;

	struct CursorPos : Moveable<CursorPos> {
		String path;
		int    pos;
	};
	Vector<CursorPos> cursor_history;
	int               cursor_history_idx = -1;

	IDESettings     settings;
	String          default_layout;

	MenuBar         menubar;
	ToolBar         toolbar;
	StatusBar       statusbar;

	ParentCtrl      editor_area;
	One<CustomFileTabs> editor_tabs;

	PyVM            vm;
	RunManager      run_manager;
	Linter          linter;
	PathManager     path_manager;
	One<PluginManager> plugin_manager;

	One<PythonConsole>   console_pane;
	One<FilesPane>       files_pane;
	One<VariableExplorer> var_explorer;
	One<PlotsPane>       plots_pane;
	One<DebuggerPane>    debugger_pane;
	One<ProfilerPane>    profiler_pane;
	One<FindInFilesPane> find_pane;
	One<OutlinePane>     outline_pane;
	One<HelpPane>        help_pane;
	One<HistoryPane>     history_pane;

	ArrayMap<String, DockableCtrl> plugin_panes;
	
	DockableCtrl context_pane_left;
	DockableCtrl context_pane_right;

	void Log(const String& s);
	void Error(const String& s);

	void InitLayout();
	virtual void DockInit() override;

	void MainMenu(Bar& bar);
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

	void MainToolbar(Bar& bar);

	void OnNewFile();
	void OnOpenFile();
	void LoadFile(const String& path);
	bool SaveFile(int idx);
	void OnSaveFile();
	void OnSaveFileAs();
	void OnSaveAll();
	void OnSaveCopyAs();
	void OnRevert();
	void OnCloseFile();
	void OnCloseAll();
	void OnOpenLastClosed();
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
	void OnAnalyze();
	void OnTabChanged();
	void OnTabMenu(Bar& bar);
	void OnTogglePane(DockableCtrl& pane);
	void OnMaximizePane();
	void OnClosePane();
	void OnFullscreen();
	void OnLayoutDefault();
	void OnLayoutRstudio();
	void OnLayoutMatlab();
	void OnSettings();
	void OnPathManager();
	void OnBreakpointHit(const String& file, int line);
	void OnConsoleInput();

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

	void UpdateStatusBar();
	void UpdateVariableExplorer();
	void UpdateRecentFilesMenu(Bar& bar);
	void AddRecentFile(const String& path);
	void SyncTabsWithFiles();
	void SyncPluginPanes();
	void ApplySettings();
	void ShowHelp(const String& topic);
	void AddCursorHistory();
	void OnPrevCursor();
	void OnNextCursor();

	bool ConfirmSave(int idx);
	bool ConfirmSaveAll();

	virtual void Close() override;
	virtual void Serialize(Stream& s) override;
	
	static size_t MemoryUsedKb();
	static size_t MemoryTotalKb();
};

#endif
