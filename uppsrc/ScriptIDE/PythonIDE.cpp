#include "ScriptIDE.h"

NAMESPACE_UPP

PythonIDE::PythonIDE()
	: run_manager(vm)
{
	plugin_manager.Create(*this);
	Title("Python IDE");
	Icon(CtrlImg::help()); // Placeholder icon

	InitLayout();
	InitDocking();
	
	files_pane.SetRoot(GetCurrentDirectory());
	
	plugin_manager->LoadPlugins();
	
	UpdateStatusBar();
}

PythonIDE::~PythonIDE()
{
}

void PythonIDE::InitLayout()
{
	AddFrame(menubar);
	AddFrame(toolbar);
	AddFrame(statusbar);
	
	menubar.Set([=](Bar& bar) { MainMenu(bar); });
	toolbar.Set([=](Bar& bar) { MainToolbar(bar); });

	Add(editor_area.SizePos());
	editor_area.Add(editor_tabs.TopPos(0, 24).HSizePos());
	
	editor_tabs.WhenAction = [=] { OnTabChanged(); };
	editor_tabs.WhenTabMenu = [=](Bar& bar) { OnTabMenu(bar); };
	
	console_pane.WhenInput = [=] { OnConsoleInput(); };
	
	run_manager.WhenStarted = [=] { console_pane.Write("Running script...\n"); };
	run_manager.WhenFinished = [=] { console_pane.Write("Finished.\n"); UpdateVariableExplorer(); };
	run_manager.WhenError = [=](const String& e) { console_pane.WriteError("Error: " + e + "\n"); };
	
	vm.WhenBreakpointHit = [=](const String& f, int l) { OnBreakpointHit(f, l); };
	
	debugger_pane.WhenStepOver = [=] { vm.StepOver(); };
	debugger_pane.WhenStepInto = [=] { vm.StepIn(); };
	debugger_pane.WhenStepOut = [=] { vm.StepOut(); };
	debugger_pane.WhenStop = [=] { run_manager.Stop(); };
}

void PythonIDE::InitDocking()
{
	DockInit();
}

void PythonIDE::DockInit()
{
	DockLeft(files_pane);
	DockRight(outline_pane);
	DockRight(var_explorer);
	DockRight(help_pane);
	DockRight(plots_pane);
	DockBottom(console_pane);
	DockBottom(history_pane);
	DockBottom(find_pane);
	DockBottom(debugger_pane);
	DockBottom(profiler_pane);

	Tabify(files_pane, outline_pane);
	Tabify(var_explorer, help_pane);
	Tabify(help_pane, plots_pane);
	Tabify(console_pane, history_pane);
	Tabify(history_pane, find_pane);
	Tabify(debugger_pane, profiler_pane);
	Tabify(var_explorer, debugger_pane);
}

void PythonIDE::Close()
{
	if(ConfirmSaveAll())
		TopWindow::Close();
}

void PythonIDE::Serialize(Stream& s)
{
	SerializeWindow(s);
}

void PythonIDE::MainMenu(Bar& bar)
{
	bar.Sub("File",      [=](Bar& b){ FileMenu(b); });
	bar.Sub("Edit",      [=](Bar& b){ EditMenu(b); });
	bar.Sub("Search",    [=](Bar& b){ SearchMenu(b); });
	bar.Sub("Source",    [=](Bar& b){ SourceMenu(b); });
	bar.Sub("Run",       [=](Bar& b){ RunMenu(b); });
	bar.Sub("Debug",     [=](Bar& b){ DebugMenu(b); });
	bar.Sub("Consoles",  [=](Bar& b){ ConsolesMenu(b); });
	bar.Sub("Projects",  [=](Bar& b){ ProjectsMenu(b); });
	bar.Sub("Tools",     [=](Bar& b){ ToolsMenu(b); });
	bar.Sub("Window",    [=](Bar& b){ WindowMenu(b); });
	bar.Sub("Help",      [=](Bar& b){ HelpMenu(b); });
}

void PythonIDE::FileMenu(Bar& bar)
{
	bar.Add("New file...", Image(), [=] { OnNewFile(); }).Key(K_CTRL_N);
	bar.Separator();
	bar.Add("Open...", Image(), [=] { OnOpenFile(); }).Key(K_CTRL_O);
	bar.Add("Open last closed", Image(), [=] { OnOpenLastClosed(); }).Key(K_CTRL|K_SHIFT|K_T);
	bar.Sub("Open recent", [=](Bar& b) { UpdateRecentFilesMenu(b); });
	bar.Separator();
	bar.Add("Save", Image(), [=] { OnSaveFile(); }).Key(K_CTRL_S);
	bar.Add("Save all", Image(), [=] { OnSaveAll(); }).Key(K_CTRL|K_ALT|K_S);
	bar.Add("Save as...", Image(), [=] { OnSaveFileAs(); }).Key(K_CTRL|K_SHIFT|K_S);
	bar.Add("Save copy as...", Image(), [=] { OnSaveCopyAs(); });
	bar.Add("Revert", Image(), [=] { OnRevert(); });
	bar.Separator();
	bar.Add("Print preview", Image(), [=] { Todo("Print preview"); });
	bar.Add("Print...", Image(), [=] { Todo("Print"); });
	bar.Separator();
	bar.Add("Close", Image(), [=] { OnCloseFile(); });
	bar.Add("Close all", Image(), [=] { OnCloseAll(); }).Key(K_CTRL|K_SHIFT|K_W);
	bar.Separator();
	bar.Add("File switcher...", Image(), [=] { OnFileSwitcher(); }).Key(K_CTRL_P);
	bar.Add("Symbol finder...", Image(), [=] { OnSymbolFinder(); }).Key(K_CTRL|K_ALT|K_P);
	bar.Separator();
	bar.Add("Restart", Image(), [=] { OnRestart(); }).Key(K_ALT|K_SHIFT|K_R);
	bar.Add("Restart in debug mode", Image(), [=] { Todo("Restart in debug mode"); });
	bar.Add("Quit", Image(), [=] { Close(); }).Key(K_CTRL_Q);
}

void PythonIDE::EditMenu(Bar& bar)
{
	bar.Add("Undo", Image(), [=] { OnUndo(); }).Key(K_CTRL_Z);
	bar.Add("Redo", Image(), [=] { OnRedo(); }).Key(K_CTRL|K_SHIFT|K_Z);
	bar.Separator();
	bar.Add("Cut", Image(), [=] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~active_editor)) h->Cut(); } }).Key(K_CTRL_X);
	bar.Add("Copy", Image(), [=] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~active_editor)) h->Copy(); } }).Key(K_CTRL_C);
	bar.Add("Paste", Image(), [=] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~active_editor)) h->Paste(); } }).Key(K_CTRL_V);
	bar.Add("Select All", Image(), [=] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~active_editor)) h->SelectAll(); } }).Key(K_CTRL_A);
	bar.Separator();
	bar.Add("Comment/uncomment", Image(), [=] { OnComment(); }).Key(K_CTRL_1);
	bar.Add("Add block comment", Image(), [=] { OnBlockComment(); }).Key(K_CTRL_4);
	bar.Add("Remove block comment", Image(), [=] { OnUncomment(); }).Key(K_CTRL_5);
	bar.Separator();
	bar.Add("Indent", Image(), [=] { Todo("Indent"); });
	bar.Add("Unindent", Image(), [=] { Todo("Unindent"); });
	bar.Separator();
	bar.Add("Toggle UPPERCASE", Image(), [=] { OnToggleCase(true); }).Key(K_ALT|K_SHIFT|K_U);
	bar.Add("Toggle lowercase", Image(), [=] { OnToggleCase(false); }).Key(K_ALT_U);
	bar.Separator();
	bar.Sub("Convert end-of-line characters", [=](Bar& b) {
		b.Add("LF (Linux/macOS)", Image(), [=] { OnConvertEOL("LF"); });
		b.Add("CRLF (Windows)", Image(), [=] { OnConvertEOL("CRLF"); });
		b.Add("CR (legacy Mac)", Image(), [=] { OnConvertEOL("CR"); });
	});
	bar.Add("Remove trailing spaces", Image(), [=] { OnRemoveTrailingSpaces(); });
	bar.Add("Convert tabs to spaces", Image(), [=] { OnTabsToSpaces(); });
}

void PythonIDE::SearchMenu(Bar& bar)
{
	bar.Add("Find text", Image(), [=] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~active_editor)) h->Find(); } });
	bar.Add("Replace text", Image(), [=] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~active_editor)) h->Replace(); } });
	bar.Separator();
	bar.Add("Last edit location", Image(), [=] { Todo("Last edit location"); }).Key(K_CTRL|K_ALT|K_SHIFT|K_LEFT);
	bar.Add("Previous cursor position", Image(), [=] { OnPrevCursor(); }).Key(K_ALT_LEFT);
	bar.Add("Next cursor position", Image(), [=] { OnNextCursor(); }).Key(K_ALT_RIGHT);
	bar.Separator();
	bar.Add("Search text in files...", Image(), [=] { OnTogglePane(find_pane); }).Key(K_ALT|K_SHIFT|K_F);
}

void PythonIDE::SourceMenu(Bar& bar)
{
	bar.Add("Show invisible characters", Image(), [=] { Todo("Toggle invisible characters"); }).Check(false);
	bar.Add("Wrap lines", Image(), [=] { Todo("Toggle wrap lines"); }).Check(false);
	bar.Add("Show indent guides", Image(), [=] { Todo("Toggle indent guides"); }).Check(true);
	bar.Add("Show code folding", Image(), [=] { Todo("Toggle code folding"); }).Check(true);
	bar.Add("Show class/function selector", Image(), [=] { Todo("Toggle symbol selector"); }).Check(true);
	bar.Add("Show docstring style warnings", Image(), [=] { Todo("Toggle docstring warnings"); }).Check(false);
	bar.Add("Underline errors and warnings", Image(), [=] { Todo("Toggle lint underlines"); }).Check(true);
	bar.Separator();
	bar.Add("Show todo list", Image(), [=] { Todo("Show todo list"); }).Enable(false);
	bar.Add("Show warning/error list", Image(), [=] { Todo("Show warning/error list"); }).Enable(false);
	bar.Add("Previous warning/error", Image(), [=] { Todo("Prev diagnostic"); }).Key(K_CTRL|K_ALT|K_SHIFT|K_COMMA);
	bar.Add("Next warning/error", Image(), [=] { Todo("Next diagnostic"); }).Key(K_CTRL|K_ALT|K_SHIFT|K_PERIOD);
	bar.Separator();
	bar.Add("Run code analysis", Image(), [=] { OnAnalyze(); }).Key(K_F8);
	bar.Add("Format file or selection with Autopep8", Image(), [=] { Todo("Autopep8"); });
}

void PythonIDE::RunMenu(Bar& bar)
{
	bar.Add("Run", Image(), [=] { OnRun(); }).Key(K_F5);
	bar.Add("Re-run last file", Image(), [=] { OnRunLast(); }).Key(K_F6);
	bar.Add("Configuration per file", Image(), [=] { OnRunConfig(); }).Key(K_CTRL_F6);
	bar.Add("Global presets", Image(), [=] { Todo("Global presets"); });
	bar.Separator();
	bar.Add("Run cell", Image(), [=] { OnRunCell(); }).Key(K_CTRL_RETURN);
	bar.Add("Run cell and advance", Image(), [=] { OnRunCellAndAdvance(); }).Key(K_SHIFT_RETURN);
	bar.Add("Re-run last cell", Image(), [=] { Todo("Re-run last cell"); }).Key(K_ALT_RETURN);
	bar.Add("Run current line/selection", Image(), [=] { OnRunSelection(); }).Key(K_F9);
	bar.Add("Run to line", Image(), [=] { OnRunToLine(); }).Key(K_SHIFT_F9);
	bar.Add("Run from line", Image(), [=] { OnRunFromLine(); }).Key(K_ALT_F9);
	bar.Separator();
	bar.Add("Run in external terminal", Image(), [=] { Todo("Run in external terminal"); });
	bar.Separator();
	bar.Add("Profile file", Image(), [=] { Todo("Profile file"); }).Key(K_F10);
	bar.Add("Profile cell", Image(), [=] { Todo("Profile cell"); }).Key(K_ALT_F10);
	bar.Add("Profile current line or selection", Image(), [=] { Todo("Profile selection"); });
}

void PythonIDE::DebugMenu(Bar& bar)
{
	bar.Add("Debug file", Image(), [=] { OnDebug(); }).Key(K_CTRL|K_F5);
	bar.Add("Debug cell", Image(), [=] { OnDebugCell(); });
	bar.Add("Debug the current line or selection", Image(), [=] { OnDebugSelection(); });
	bar.Separator();
	bar.Add("Step into function or method", Image(), [=] { OnStepIn(); }).Key(K_CTRL|K_F11);
	bar.Add("Execute until function returns", Image(), [=] { OnStepOut(); }).Key(K_CTRL|K_SHIFT|K_F11);
	bar.Add("Execute until next breakpoint", Image(), [=] { OnStepOver(); }).Key(K_CTRL|K_F12);
	bar.Add("Stop debugging", Image(), [=] { OnStop(); }).Key(K_CTRL|K_SHIFT|K_F12);
	bar.Separator();
	bar.Add("Toggle breakpoint", Image(), [=] { OnToggleBreakpoint(); }).Key(K_F12);
	bar.Add("Set/edit conditional breakpoint", Image(), [=] { Todo("Conditional breakpoint"); }).Key(K_SHIFT|K_F12);
	bar.Add("Clear breakpoints in all files", Image(), [=] { OnClearBreakpoints(); });
	bar.Add("List breakpoints", Image(), [=] { OnListBreakpoints(); });
}

void PythonIDE::ConsolesMenu(Bar& bar)
{
	bar.Add("New console (default settings)", Image(), [=] { OnConsoleInput(); }).Key(K_CTRL_T);
	bar.Sub("New console in environment", [=](Bar& b) {
		b.Add("Conda: spyder-runtime 0", Image(), [=] { Todo("New Conda console"); });
	});
	bar.Sub("New special console", [=](Bar& b) {
		b.Add("New Pylab console (data plotting)", Image(), [=] { Todo("New Pylab console"); });
		b.Add("New SymPy console (symbolic math)", Image(), [=] { Todo("New SymPy console"); });
		b.Add("New Cython console (Python with C extensions)", Image(), [=] { Todo("New Cython console"); });
	});
	bar.Sub("New console in remote server", [=](Bar& b) {
		b.Add("Manage remote connections", Image(), [=] { Todo("Manage remote connections"); });
	});
	bar.Add("Connect to existing kernel...", Image(), [=] { Todo("Connect to kernel"); });
	bar.Separator();
	bar.Add("Interrupt kernel", Image(), [=] { Todo("Interrupt kernel"); });
	bar.Add("Restart kernel", Image(), [=] { Todo("Restart kernel"); }).Key(K_CTRL_PERIOD);
	bar.Add("Remove all variables", Image(), [=] { Todo("Remove all variables"); }).Key(K_CTRL|K_ALT|K_R);
}

void PythonIDE::ProjectsMenu(Bar& bar)
{
	bar.Add("New Project...", Image(), [=] { Todo("New Project"); });
	bar.Add("Open Project...", Image(), [=] { Todo("Open Project"); });
	bar.Add("Close Project", Image(), [=] { Todo("Close Project"); });
	bar.Add("Delete Project", Image(), [=] { Todo("Delete Project"); });
	bar.Separator();
	bar.Sub("Recent Projects", [=](Bar& b) {
		b.Add("Clear this list", Image(), [=] { Todo("Clear recent projects"); });
		b.Add("Maximum number of recent projects", Image(), [=] { Todo("Max recent projects"); });
	});
}

void PythonIDE::ToolsMenu(Bar& bar)
{
	bar.Add("PYTHONPATH manager", Image(), [=] { OnPathManager(); });
	bar.Add("User environment variables", Image(), [=] { Todo("Env vars"); });
	bar.Add("Manage remote connections", Image(), [=] { Todo("Remote connections"); });
	bar.Separator();
	bar.Add("Preferences", Image(), [=] { OnSettings(); });
	bar.Add("Reset all preferences to defaults", Image(), [=] { Todo("Reset preferences"); });
}

void PythonIDE::WindowMenu(Bar& bar)
{
	bar.Sub("Panes", [=](Bar& b) {
		b.Add("Editor", Image(), [=] { Todo("Show Editor"); }).Key(K_CTRL|K_SHIFT|K_E).Check(true);
		b.Add("IPython Console", Image(), [=] { OnTogglePane(console_pane); }).Key(K_CTRL|K_SHIFT|K_I).Check(console_pane.IsDocked());
		b.Add("Variable Explorer", Image(), [=] { OnTogglePane(var_explorer); }).Key(K_CTRL|K_SHIFT|K_V).Check(var_explorer.IsDocked());
		b.Add("Debugger", Image(), [=] { OnTogglePane(debugger_pane); }).Key(K_CTRL|K_SHIFT|K_D).Check(debugger_pane.IsDocked());
		b.Add("Help", Image(), [=] { OnTogglePane(help_pane); }).Key(K_CTRL|K_SHIFT|K_H).Check(help_pane.IsDocked());
		b.Add("Plots", Image(), [=] { OnTogglePane(plots_pane); }).Key(K_CTRL|K_SHIFT|K_G).Check(plots_pane.IsDocked());
		b.Separator();
		b.Add("Files", Image(), [=] { OnTogglePane(files_pane); }).Key(K_CTRL|K_SHIFT|K_X).Check(files_pane.IsDocked());
		b.Add("Outline", Image(), [=] { OnTogglePane(outline_pane); }).Key(K_CTRL|K_SHIFT|K_O).Check(outline_pane.IsDocked());
		b.Add("Project", Image(), [=] { Todo("Toggle Project Pane"); }).Key(K_CTRL|K_SHIFT|K_P).Check(false);
		b.Add("Find", Image(), [=] { OnTogglePane(find_pane); }).Key(K_CTRL|K_SHIFT|K_F).Check(find_pane.IsDocked());
		b.Separator();
		b.Add("History", Image(), [=] { OnTogglePane(history_pane); }).Key(K_CTRL|K_SHIFT|K_L).Check(history_pane.IsDocked());
		b.Add("Profiler", Image(), [=] { OnTogglePane(profiler_pane); }).Key(K_CTRL|K_SHIFT|K_R).Check(profiler_pane.IsDocked());
		b.Add("Code Analysis", Image(), [=] { Todo("Toggle Code Analysis Pane"); }).Key(K_CTRL|K_SHIFT|K_C).Check(false);
		b.Separator();
		b.Add("Online help", Image(), [=] { Todo("Toggle Online Help"); }).Check(false);
		b.Add("Internal console", Image(), [=] { Todo("Toggle Internal Console"); }).Check(false);
		
		if(plugin_panes.GetCount() > 0) {
			b.Separator();
			for(int i = 0; i < plugin_panes.GetCount(); i++) {
				String title = plugin_panes.GetKey(i);
				DockableCtrl& pane = *plugin_panes[i];
				b.Add(title, [=, &pane] { OnTogglePane(pane); }).Check(pane.IsDocked());
			}
		}
	});
	bar.Add("Unlock panes and toolbars", Image(), [=] { Todo("Unlock layout"); }).Key(K_CTRL|K_SHIFT|K_F5);
	bar.Add("Maximize current pane", Image(), [=] { OnMaximizePane(); }).Key(K_CTRL|K_ALT|K_SHIFT|K_M);
	bar.Add("Close current pane", Image(), [=] { OnClosePane(); }).Key(K_CTRL|K_SHIFT|K_F4);
	bar.Separator();
	bar.Sub("Toolbars", [=](Bar& b) {
		b.Add("File toolbar", Image(), [=] { Todo("Toggle File toolbar"); }).Check(true);
		b.Add("Run toolbar", Image(), [=] { Todo("Toggle Run toolbar"); }).Check(true);
		b.Add("Debug toolbar", Image(), [=] { Todo("Toggle Debug toolbar"); }).Check(true);
		b.Add("Profile toolbar", Image(), [=] { Todo("Toggle Profile toolbar"); }).Check(true);
		b.Add("Main toolbar", Image(), [=] { Todo("Toggle Main toolbar"); }).Check(true);
		b.Add("Current working directory", Image(), [=] { Todo("Toggle CWD toolbar"); }).Check(true);
	});
	bar.Add("Hide toolbars", Image(), [=] { Todo("Hide toolbars"); });
	bar.Separator();
	bar.Sub("Window layouts", [=](Bar& b) {
		b.Add("Default layout", Image(), [=] { OnLayoutDefault(); });
		b.Add("Rstudio layout", Image(), [=] { OnLayoutRstudio(); });
		b.Add("Matlab layout", Image(), [=] { OnLayoutMatlab(); });
		b.Add("Horizontal split", Image(), [=] { Todo("Layout Horizontal"); });
		b.Add("Vertical split", Image(), [=] { Todo("Layout Vertical"); });
		b.Separator();
		b.Add("Save current layout", Image(), [=] { Todo("Save layout"); });
		b.Add("Layout preferences", Image(), [=] { Todo("Layout preferences"); });
		b.Add("Reset to Spyder default", Image(), [=] { OnLayoutDefault(); });
	});
	bar.Add("Use next layout", Image(), [=] { Todo("Next layout"); }).Key(K_ALT|K_SHIFT|K_PAGEDOWN);
	bar.Add("Use previous layout", Image(), [=] { Todo("Prev layout"); }).Key(K_ALT|K_SHIFT|K_PAGEUP);
	bar.Separator();
	bar.Add("Fullscreen mode", Image(), [=] { OnFullscreen(); }).Key(K_F11);
}

void PythonIDE::HelpMenu(Bar& bar)
{
	bar.Add("Interactive tour", Image(), [=] { Todo("Tour"); });
	bar.Add("Built-in tutorial", Image(), [=] { Todo("Tutorial"); });
	bar.Add("Shortcuts summary", Image(), [=] { Todo("Shortcuts"); });
	bar.Separator();
	bar.Add("Spyder documentation", Image(), [=] { ShowHelp("index"); }).Key(K_F1);
	bar.Add("Tutorial videos", Image(), [=] { Todo("Videos"); });
	bar.Sub("IPython documentation", [=](Bar& b) {
		b.Add("Intro to IPython", Image(), [=] { Todo("IPython Intro"); });
		b.Add("Console help", Image(), [=] { Todo("Console Help"); });
		b.Add("Quick reference", Image(), [=] { Todo("Quick Ref"); });
	});
	bar.Add("Troubleshooting guide", Image(), [=] { Todo("Troubleshooting"); });
	bar.Add("Spyder Google group", Image(), [=] { Todo("Google Group"); });
	bar.Add("Dependency status", Image(), [=] { Todo("Dependencies"); });
	bar.Add("Report issue...", Image(), [=] { Todo("Report issue"); });
	bar.Separator();
	bar.Add("Check for updates", Image(), [=] { Todo("Updates"); });
	bar.Add("Help Spyder...", Image(), [=] { Todo("Help Spyder"); });
	bar.Add("About Spyder", Image(), [=] { Todo("About"); });
}

void PythonIDE::MainToolbar(Bar& bar)
{
	bar.Add(CtrlImg::new_doc(), [=] { OnNewFile(); }).Help("New File");
	bar.Add(CtrlImg::open(), [=] { OnOpenFile(); }).Help("Open File");
	bar.Add(CtrlImg::save(), [=] { OnSaveFile(); }).Help("Save File");
	bar.Add(CtrlImg::save_as(), [=] { OnSaveAll(); }).Help("Save All Files");
	bar.Separator();
	bar.Add(CtrlImg::plus(), [=] { Todo("Create new cell"); }).Help("Create new cell at the current line");
	bar.Separator();
	bar.Add(CtrlImg::right_arrow(), [=] { OnRun(); }).Help("Run file");
	bar.Add(CtrlImg::right_arrow(), [=] { OnRunCell(); }).Help("Run cell");
	bar.Add(CtrlImg::right_arrow(), [=] { OnRunCellAndAdvance(); }).Help("Run cell and advance");
	bar.Add(CtrlImg::right_arrow(), [=] { OnRunSelection(); }).Help("Run current line or selection");
	bar.Separator();
	bar.Add(CtrlImg::help(), [=] { OnDebug(); }).Help("Debug file");
	bar.Add(CtrlImg::help(), [=] { OnDebugCell(); }).Help("Debug cell");
	bar.Add(CtrlImg::help(), [=] { OnDebugSelection(); }).Help("Debug the current file or selection");
	bar.Separator();
	bar.Add(CtrlImg::save(), [=] { Todo("Profile file"); }).Help("Profile file");
	bar.Add(CtrlImg::save(), [=] { Todo("Profile cell"); }).Help("Profile cell");
	bar.Add(CtrlImg::save(), [=] { Todo("Profile selection"); }).Help("Profile current line or selection");
	bar.Separator();
	bar.Add(CtrlImg::plus(), [=] { OnMaximizePane(); }).Help("Maximize current pane");
	bar.Add(CtrlImg::plus(), [=] { OnSettings(); }).Help("Preferences");
	bar.Add(CtrlImg::plus(), [=] { OnPathManager(); }).Help("PYTHONPATH manager");
	bar.Separator();
	
	// Dropdowns and path helpers would go here
	bar.Add("Recent Projects", [=] { Todo("Recent projects dropdown"); });
	bar.Add("Working Directory", [=] { Todo("Working directory dropdown"); });
	bar.Add(CtrlImg::open(), [=] { Todo("Browse working directory"); }).Help("Browse working directory");
	bar.Add(CtrlImg::undo(), [=] { Todo("Change to parent directory"); }).Help("Change to parent directory");
}

void PythonIDE::ShowHelp(const String& topic)
{
	String qtf;
	qtf << "[_^https://docs.python.org/3/search.html?q=" << topic << "^ Search Python Docs for: " << topic << "]";
	help_pane.SetQTF(qtf);
}

void PythonIDE::OnPathManager()
{
	PathManagerDlg dlg;
	dlg.Set(path_manager);
	if(dlg.Run() == IDOK) {
		dlg.Get(path_manager);
		path_manager.SyncToVM(vm);
		StoreToFile(path_manager, ConfigFile("pythonpath.bin"));
	}
}

void PythonIDE::OnRun()
{
	if(active_file < 0) return;
	
	String path = open_files[active_file].path;
	String ext = GetFileExt(path);
	
	ICustomExecuteProvider* provider = plugin_manager->FindCustomExecuteProvider(path);
	if(provider) {
		provider->Execute(path);
		return;
	}
	
	String content;
	if(PythonEditor* ed = dynamic_cast<PythonEditor*>(~open_files[active_file].editor))
		content = ed->Get();
	
	plugin_manager->SyncBindings(vm);
	run_manager.Run(content, path);
}

void PythonIDE::OnRunLast() { Todo(); }
void PythonIDE::OnRunSelection()
{
	if(active_file < 0) return;
	if(PythonEditor* ed = dynamic_cast<PythonEditor*>(~open_files[active_file].editor))
		run_manager.RunSelection(ed->GetSelection());
}

void PythonIDE::OnRunCell() { Todo("Run cell"); }
void PythonIDE::OnRunCellAndAdvance() { Todo("Run cell and advance"); }
void PythonIDE::OnRunToLine() { Todo("Run to line"); }
void PythonIDE::OnRunFromLine() { Todo("Run from line"); }
void PythonIDE::OnRunConfig() { Todo(); }

void PythonIDE::OnDebug() { Todo(); }
void PythonIDE::OnDebugCell() { Todo("Debug cell"); }
void PythonIDE::OnDebugSelection() { Todo("Debug selection"); }
void PythonIDE::OnDebugToLine() { Todo("Debug to line"); }
void PythonIDE::OnStop() { run_manager.Stop(); }
void PythonIDE::OnStepOver() { vm.StepOver(); }
void PythonIDE::OnStepIn() { vm.StepIn(); }
void PythonIDE::OnStepOut() { vm.StepOut(); }

void PythonIDE::OnToggleBreakpoint()
{
	if(active_file < 0) return;
	if(PythonEditor* ed = dynamic_cast<PythonEditor*>(~open_files[active_file].editor)) {
		int line = ed->GetLine(ed->GetCursor()) + 1;
		String file = open_files[active_file].path;
		if(file.IsEmpty()) return;
		
		if(vm.HasBreakpoint(file, line))
			vm.RemoveBreakpoint(file, line);
		else
			vm.AddBreakpoint(file, line);
			
		ed->Refresh();
	}
}
void PythonIDE::OnClearBreakpoints() { Todo(); }
void PythonIDE::OnListBreakpoints() { Todo(); }
void PythonIDE::OnConsoleInput()
{
	String input = console_pane.GetInput();
	if(input.IsEmpty()) return;
	
	try {
		// Minimalistic REPL execution for now
		Tokenizer tk;
		if(!tk.Process(input, "<console>")) return;
		tk.NewlineToEndStatement();
		tk.CombineTokens();

		PyCompiler compiler(tk.GetTokens(), "<console>");
		Vector<PyIR> ir;
		compiler.Compile(ir);

		plugin_manager->SyncBindings(vm);
		vm.SetIR(ir);
		vm.Run();
		
		PyValue res = vm.GetLastResult();
		if(!res.IsNone())
			console_pane.Write(res.Repr() + "\n");
			
		UpdateVariableExplorer();
	}
	catch (Exc& e) {
		console_pane.WriteError(String("Error: ") + e + "\n");
	}
}

class SourceDocumentHost : public PythonEditor, public IDocumentHost {
public:
	SourceDocumentHost() { 
		Highlight("python");
		EnableBreakpointing();
		WordWrap(true);
	}
	
	virtual Ctrl&  GetCtrl() override { return *this; }
	virtual bool   Load(const String& path_) override { 
		path = path_;
		Set(::Upp::LoadFile(path));
		ClearDirty();
		return true;
	}
	virtual bool   Save() override { 
		if(path.IsEmpty()) return false;
		if(::Upp::SaveFile(path, Get())) {
			ClearDirty();
			return true;
		}
		return false;
	}
	virtual bool   SaveAs(const String& path_) override {
		path = path_;
		return Save();
	}
	virtual String GetPath() const override { return path; }
	virtual bool   IsModified() const override { return CodeEditor::IsDirty(); }
	virtual void   SetFocus() override { CodeEditor::SetFocus(); }
	virtual void   Undo() override { CodeEditor::Undo(); }
	virtual void   Redo() override { CodeEditor::Redo(); }
	virtual void   Cut() override { CodeEditor::Cut(); }
	virtual void   Copy() override { CodeEditor::Copy(); }
	virtual void   Paste() override { CodeEditor::Paste(); }
	virtual void   SelectAll() override { CodeEditor::SelectAll(); }
	virtual void   Find() override { PythonEditor::DoFind(false, false); }
	virtual void   Replace() override { PythonEditor::DoReplace(); }
	
	String path;
};

void PythonIDE::OnNewFile()
{
	SourceDocumentHost* editor = new SourceDocumentHost();
	editor_area.Add(editor->VSizePos(24, 0).HSizePos());
	
	FileInfo& fi = open_files.Add();
	fi.path = "";
	fi.dirty = false;
	fi.content = "";
	fi.editor = editor;
	
	active_file = open_files.GetCount() - 1;
	editor_tabs.Add("<untitled>", CtrlImg::File());
	editor_tabs.SetCursor(active_file);
	
	OnTabChanged();
}

void PythonIDE::OnOpenFile()
{
	FileSel fs;
	fs.Type("Python files", "*.py");
	if(fs.ExecuteOpen("Open Python File")) {
		LoadFile(fs.Get());
	}
}

void PythonIDE::LoadFile(const String& path)
{
	for(int i = 0; i < open_files.GetCount(); i++) {
		if(open_files[i].path == path) {
			editor_tabs.SetCursor(i);
			return;
		}
	}

	String ext = GetFileExt(path);
	IDocumentHost* host = nullptr;
	bool is_plugin = false;
	
	IFileTypeHandler* handler = plugin_manager->FindFileTypeHandler(ext);
	if(handler) {
		host = handler->CreateDocumentHost();
		if(host) is_plugin = true;
	}
	
	if(!host) {
		host = new SourceDocumentHost();
	}
	
	if(!host->Load(path)) {
		delete host;
		return;
	}
	
	Ctrl* editor = &host->GetCtrl();
	editor_area.Add(editor->VSizePos(24, 0).HSizePos());
	
	FileInfo& fi = open_files.Add();
	fi.path = path;
	fi.dirty = false;
	fi.editor = editor;
	fi.is_plugin = is_plugin;
	
	active_file = open_files.GetCount() - 1;
	editor_tabs.Add(GetFileName(path), CtrlImg::File());
	editor_tabs.SetCursor(active_file);
	
	OnTabChanged();
}

void PythonIDE::OnSaveFile()
{
	if(active_file < 0) return;
	FileInfo& fi = open_files[active_file];
	if(fi.path.IsEmpty()) {
		OnSaveFileAs();
	}
	else {
		SaveFile(active_file);
	}
}

void PythonIDE::OnSaveFileAs()
{
	if(active_file < 0) return;
	FileSel fs;
	fs.Type("Python files", "*.py");
	if(fs.ExecuteSaveAs("Save Python File As")) {
		open_files[active_file].path = fs.Get();
		SaveFile(active_file);
	}
}

void PythonIDE::SaveFile(int idx)
{
	if(idx < 0 || idx >= open_files.GetCount()) return;
	FileInfo& fi = open_files[idx];
	if(fi.path.IsEmpty()) return;
	
	if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~fi.editor)) {
		if(h->SaveAs(fi.path)) {
			fi.dirty = false;
			editor_tabs.Set(idx, idx, GetFileName(fi.path));
		}
		else {
			Exclamation("Failed to save file: " + fi.path);
		}
	}
}

bool PythonIDE::ConfirmSave(int idx)
{
	if(idx < 0 || idx >= open_files.GetCount()) return true;
	FileInfo& fi = open_files[idx];
	
	bool dirty = fi.dirty;
	if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~fi.editor))
		dirty = h->IsModified();
	
	if(!dirty) return true;
	
	int res = PromptYesNoCancel("Save changes to " + (fi.path.IsEmpty() ? "untitled" : fi.path) + "?");
	
	if(res == 1) { // Yes
		if(fi.path.IsEmpty()) {
			int old_active = active_file;
			active_file = idx;
			OnSaveFileAs();
			active_file = old_active;
		}
		else {
			SaveFile(idx);
		}
		
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~fi.editor))
			dirty = h->IsModified();
		return !dirty;
	}
	if(res == 0) return true; // No
	return false; // Cancel
}

bool PythonIDE::ConfirmSaveAll()
{
	for(int i = 0; i < open_files.GetCount(); i++) {
		if(!ConfirmSave(i)) return false;
	}
	return true;
}

void PythonIDE::OnSaveAll()
{
	int old_active = active_file;
	for(int i = 0; i < open_files.GetCount(); i++) {
		active_file = i;
		OnSaveFile();
	}
	active_file = old_active;
}

void PythonIDE::OnSaveCopyAs() { Todo(); }
void PythonIDE::OnRevert() { Todo(); }
void PythonIDE::AddRecentFile(const String& path) { Todo(); }
void PythonIDE::UpdateRecentFilesMenu(Bar& bar) { Todo(); }
void PythonIDE::OnFileSwitcher() { Todo(); }
void PythonIDE::OnOpenFileSwitcher() { Todo(); }
void PythonIDE::OnSymbolFinder() { Todo(); }
void PythonIDE::OnRestart() { Todo(); }

void PythonIDE::OnUndo() { 
	if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~active_editor)) h->Undo(); }
}
void PythonIDE::OnRedo() { 
	if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(~active_editor)) h->Redo(); }
}
void PythonIDE::OnComment() { 
	if(PythonEditor* ed = dynamic_cast<PythonEditor*>(~active_editor))
		ed->ToggleComments();
}
void PythonIDE::OnBlockComment() { 
	if(PythonEditor* ed = dynamic_cast<PythonEditor*>(~active_editor))
		ed->ToggleBlockComments();
}
void PythonIDE::OnUncomment() { 
	if(PythonEditor* ed = dynamic_cast<PythonEditor*>(~active_editor))
		ed->ToggleComments(); 
}
void PythonIDE::OnToggleCase(bool upper) { Todo(String(upper ? "Uppercase" : "Lowercase")); }
void PythonIDE::OnConvertEOL(const String& mode) { Todo("Convert EOL to " + mode); }
void PythonIDE::OnRemoveTrailingSpaces() { Todo("Remove trailing spaces"); }
void PythonIDE::OnTabsToSpaces() { Todo("Tabs to spaces"); }

void PythonIDE::OnCloseFile()
{
	if(active_file < 0) return;
	if(!ConfirmSave(active_file)) return;
	
	int to_close = active_file;
	open_files.Remove(to_close);
	editor_tabs.Close(to_close);
	
	if(open_files.GetCount() > 0) {
		active_file = editor_tabs.GetCursor();
		OnTabChanged();
	}
	else {
		OnTabChanged();
	}
}

void PythonIDE::OnCloseAll()
{
	if(!ConfirmSaveAll()) return;
	open_files.Clear();
	editor_tabs.Clear();
	OnTabChanged();
}

void PythonIDE::OnOpenLastClosed() { Todo("Open last closed"); }

void PythonIDE::OnBreakpointHit(const String& file, int line)
{
	LoadFile(file);
	if(active_editor) {
		if(PythonEditor* ed = dynamic_cast<PythonEditor*>(~active_editor)) {
			ed->SetCursor(ed->GetPos(line - 1));
			ed->SetPtr(line - 1, CtrlImg::right_arrow(), 0);
		}
	}
	
	debugger_pane.SetStack(vm.GetCallStack());
	UpdateVariableExplorer();
	
	console_pane.Write(Format("Breakpoint hit at %s:%d\n", file, line));
	Show();
}

void PythonIDE::OnTabChanged()
{
	if(active_editor) active_editor->Hide();
	int idx = editor_tabs.GetCursor();
	if(idx >= 0 && idx < open_files.GetCount()) {
		active_file = idx;
		active_editor = open_files[active_file].editor;
		
		if(active_editor) {
			active_editor->Show();
			active_editor->SetFocus();
		}
	}
	else {
		active_file = -1;
		active_editor = nullptr;
	}
}
void PythonIDE::OnTabMenu(Bar& bar) { Todo("Tab menu"); }
void PythonIDE::SyncTabsWithFiles() { Todo("Sync tabs"); }

void PythonIDE::OnTogglePane(DockableCtrl& pane)
{
	if(pane.IsDocked())
		Float(pane);
	else
		DockBottom(pane);
}

void PythonIDE::OnLayoutDefault() { Todo("Default layout"); }
void PythonIDE::OnLayoutRstudio() { Todo("Rstudio layout"); }
void PythonIDE::OnLayoutMatlab() { Todo("Matlab layout"); }
void PythonIDE::OnFullscreen() { Todo("Fullscreen"); }
void PythonIDE::OnMaximizePane() { Todo("Maximize pane"); }
void PythonIDE::OnClosePane() { Todo("Close pane"); }

void PythonIDE::UpdateStatusBar()
{
	String text;
	if(active_file >= 0 && active_editor) {
		if(PythonEditor* ed = dynamic_cast<PythonEditor*>(~active_editor)) {
			Point p = ed->GetColumnLine(ed->GetCursor64());
			text << "Line: " << p.y + 1 << "  Col: " << p.x + 1;
		}
	}
	
	size_t used = MemoryUsedKb();
	if(used > 0)
		text << "    Memory: " << used << " KB";
		
	statusbar.Set(text);
	SetTimeCallback(-200, [=] { UpdateStatusBar(); });
}
void PythonIDE::UpdateVariableExplorer()
{
	var_explorer.SetVariables(vm.GetGlobals());
}

void PythonIDE::ApplySettings()
{
	Todo("Apply settings");
}

void PythonIDE::OnAnalyze() { Todo(); }

void PythonIDE::SyncPluginPanes()
{
	// Plugin panes are handled via RegisterDockPane in PluginManager
}

void PythonIDE::OnPrevCursor() { Todo("Prev cursor"); }
void PythonIDE::OnNextCursor() { Todo("Next cursor"); }
void PythonIDE::AddCursorHistory() { Todo("Add cursor history"); }

size_t PythonIDE::MemoryUsedKb() { return 0; }
size_t PythonIDE::MemoryTotalKb() { return 0; }

}
