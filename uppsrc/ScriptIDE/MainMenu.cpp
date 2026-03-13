#include "ScriptIDE.h"

namespace Upp {

void PythonIDE::MainMenu(Bar& bar)
{
	bar.Sub("File",      [this](Bar& b){ FileMenu(b); });
	bar.Sub("Edit",      [this](Bar& b){ EditMenu(b); });
	bar.Sub("Search",    [this](Bar& b){ SearchMenu(b); });
	bar.Sub("Source",    [this](Bar& b){ SourceMenu(b); });
	
	if(active_editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor))
			h->MainMenu(bar);
	}

	bar.Sub("Run",       [this](Bar& b){ RunMenu(b); });
	bar.Sub("Debug",     [this](Bar& b){ DebugMenu(b); });
	bar.Sub("Consoles",  [this](Bar& b){ ConsolesMenu(b); });
	bar.Sub("Projects",  [this](Bar& b){ ProjectsMenu(b); });
	bar.Sub("Tools",     [this](Bar& b){ ToolsMenu(b); });
	bar.Sub("Window",    [this](Bar& b){ WindowMenu(b); });
	bar.Sub("Help",      [this](Bar& b){ HelpMenu(b); });
}

void PythonIDE::FileMenu(Bar& bar)
{
	bar.Add("New file...", Icons::NewFile(), [this] { OnNewFile(); }).Key(K_CTRL_N);
	bar.Separator();
	bar.Add("Open...", Icons::OpenFile(), [this] { OnOpenFile(); }).Key(K_CTRL_O);
	bar.Add("Open last closed", [this] { OnOpenLastClosed(); }).Key(K_CTRL|K_SHIFT|K_T);
	bar.Sub("Open recent", [this](Bar& b) { UpdateRecentFilesMenu(b); });
	bar.Separator();
	bar.Add("Save", Icons::Save(), [this] { OnSaveFile(); }).Key(K_CTRL_S);
	bar.Add("Save all", Icons::SaveAll(), [this] { OnSaveAll(); }).Key(K_CTRL|K_ALT|K_S);
	bar.Add("Save as...", Icons::Save(), [this] { OnSaveFileAs(); }).Key(K_CTRL|K_SHIFT|K_S);
	bar.Add("Save copy as...", [this] { OnSaveCopyAs(); });
	bar.Add("Revert", [this] { OnRevert(); });
	bar.Separator();
	bar.Add("Print preview", [this] { Todo("Print preview"); });
	bar.Add("Print...", [this] { Todo("Print"); });
	bar.Separator();
	bar.Add("Close", [this] { OnCloseFile(); });
	bar.Add("Close all", [this] { OnCloseAll(); }).Key(K_CTRL|K_SHIFT|K_W);
	bar.Separator();
	bar.Add("File switcher...", [this] { OnFileSwitcher(); }).Key(K_CTRL_P);
	bar.Add("Symbol finder...", [this] { OnSymbolFinder(); }).Key(K_CTRL|K_ALT|K_P);
	bar.Separator();
	bar.Add("Restart", [this] { OnRestart(); }).Key(K_ALT|K_SHIFT|K_R);
	bar.Add("Restart in debug mode", [this] { Todo("Restart in debug mode"); });
	bar.Add("Quit", [this] { Close(); }).Key(K_CTRL_Q);
}

void PythonIDE::EditMenu(Bar& bar)
{
	bar.Add("Undo", Icons::Undo(), [this] { OnUndo(); }).Key(K_CTRL_Z);
	bar.Add("Redo", Icons::Redo(), [this] { OnRedo(); }).Key(K_CTRL|K_SHIFT|K_Z);
	bar.Separator();
	bar.Add("Cut", [this] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) h->Cut(); } }).Key(K_CTRL_X);
	bar.Add("Copy", [this] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) h->Copy(); } }).Key(K_CTRL_C);
	bar.Add("Paste", [this] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) h->Paste(); } }).Key(K_CTRL_V);
	bar.Add("Select All", [this] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) h->SelectAll(); } }).Key(K_CTRL_A);
	bar.Separator();
	bar.Add("Comment/uncomment", [this] { OnComment(); }).Key(K_CTRL_1);
	bar.Add("Add block comment", [this] { OnBlockComment(); }).Key(K_CTRL_4);
	bar.Add("Remove block comment", [this] { OnUncomment(); }).Key(K_CTRL_5);
	bar.Separator();
	bar.Add("Indent", [this] { Todo("Indent"); });
	bar.Add("Unindent", [this] { Todo("Unindent"); });
	bar.Separator();
	bar.Add("Toggle UPPERCASE", [this] { OnToggleCase(true); }).Key(K_ALT|K_SHIFT|K_U);
	bar.Add("Toggle lowercase", [this] { OnToggleCase(false); }).Key(K_ALT_U);
	bar.Separator();
	bar.Sub("Convert end-of-line characters", [this](Bar& b) {
		b.Add("LF (Linux/macOS)", [this] { OnConvertEOL("LF"); });
		b.Add("CRLF (Windows)", [this] { OnConvertEOL("CRLF"); });
		b.Add("CR (legacy Mac)", [this] { OnConvertEOL("CR"); });
	});
	bar.Add("Remove trailing spaces", [this] { OnRemoveTrailingSpaces(); });
	bar.Add("Convert tabs to spaces", [this] { OnTabsToSpaces(); });
}

void PythonIDE::SearchMenu(Bar& bar)
{
	bar.Add("Find text", Icons::Search(), [this] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) h->Find(); } });
	bar.Add("Find next", [this] { Todo("Find next"); });
	bar.Add("Find previous", [this] { Todo("Find previous"); });
	bar.Add("Replace text", [this] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) h->Replace(); } });
	bar.Separator();
	bar.Add("Last edit location", [this] { Todo("Last edit location"); }).Key(K_CTRL|K_ALT|K_SHIFT|K_LEFT);
	bar.Add("Previous cursor position", [this] { OnPrevCursor(); }).Key(K_ALT_LEFT);
	bar.Add("Next cursor position", [this] { OnNextCursor(); }).Key(K_ALT_RIGHT);
	bar.Separator();
	bar.Add("Search text in files...", Icons::Search(), [this] { OnTogglePane(*find_pane); }).Key(K_ALT|K_SHIFT|K_F);
}

void PythonIDE::SourceMenu(Bar& bar)
{
	bar.Add("Show invisible characters", [this] { Todo("Toggle invisible characters"); }).Check(false);
	bar.Add("Wrap lines", [this] { Todo("Toggle wrap lines"); }).Check(false);
	bar.Add("Show indent guides", [this] { Todo("Toggle indent guides"); }).Check(true);
	bar.Add("Show code folding", [this] { Todo("Toggle code folding"); }).Check(true);
	bar.Add("Show class/function selector", [this] { Todo("Toggle symbol selector"); }).Check(true);
	bar.Add("Show docstring style warnings", [this] { Todo("Toggle docstring warnings"); }).Check(false);
	bar.Add("Underline errors and warnings", [this] { Todo("Toggle lint underlines"); }).Check(true);
	bar.Separator();
	bar.Add("Show todo list", [this] { Todo("Show todo list"); }).Enable(false);
	bar.Add("Show warning/error list", [this] { Todo("Show warning/error list"); }).Enable(false);
	bar.Add("Previous warning/error", [this] { Todo("Prev diagnostic"); }).Key(K_CTRL|K_ALT|K_SHIFT|K_COMMA);
	bar.Add("Next warning/error", [this] { Todo("Next diagnostic"); }).Key(K_CTRL|K_ALT|K_SHIFT|K_PERIOD);
	bar.Separator();
	bar.Add("Run code analysis", Icons::Profiler(), [this] { OnAnalyze(); }).Key(K_F8);
	bar.Add("Format file or selection with Autopep8", [this] { Todo("Autopep8"); });
}

void PythonIDE::RunMenu(Bar& bar)
{
	bool doc_running = false;
	bool doc_paused = false;
	bool doc_can_pause = false;
	if(active_file >= 0 && active_file < open_files.GetCount()) {
		if(IDocumentHost* h = open_files[active_file].editor) {
			doc_running = h->IsRunning();
			doc_paused = h->IsPaused();
			doc_can_pause = h->CanPause();
		}
	}
	bool run_enabled = !(doc_running || vm.IsRunning());
	bool pause_enabled = doc_can_pause && doc_running;

	bar.Add("Run", Icons::Run(), [this] { OnRun(); }).Key(K_CTRL|K_F5).Enable(run_enabled);
	bar.Add("Re-run last file", [this] { OnRunLast(); }).Key(K_F6);
	bar.Add("Configuration per file", [this] { OnRunConfig(); }).Key(K_CTRL_F6);
	bar.Add("Global presets", [this] { Todo("Global presets"); });
	bar.Separator();
	bar.Add("Run cell", [this] { OnRunCell(); }).Key(K_CTRL_RETURN).Enable(run_enabled);
	bar.Add("Run cell and advance", [this] { OnRunCellAndAdvance(); }).Key(K_SHIFT_RETURN).Enable(run_enabled);
	bar.Add("Re-run last cell", [this] { Todo("Re-run last cell"); }).Key(K_ALT_RETURN).Enable(run_enabled);
	bar.Add("Run current line/selection", [this] { OnRunSelection(); }).Key(K_F9).Enable(run_enabled);
	bar.Add("Run to line", [this] { OnRunToLine(); }).Key(K_SHIFT_F9).Enable(run_enabled);
	bar.Add("Run from line", [this] { OnRunFromLine(); }).Key(K_ALT_F9).Enable(run_enabled);
	bar.Separator();
	bar.Add("Run in external terminal", [this] { Todo("Run in external terminal"); });
	bar.Separator();
	bar.Add("Profile file", Icons::Profiler(), [this] { OnProfile(); }).Key(K_F10).Enable(active_file >= 0 && run_enabled);
	bar.Add("Profile cell", Icons::Profiler(), [this] { Todo("Profile cell"); }).Key(K_ALT_F10).Enable(run_enabled);
	bar.Add("Profile current line or selection", Icons::Profiler(), [this] { Todo("Profile selection"); }).Enable(run_enabled);
	bar.Separator();
	bar.Add(doc_paused ? "Resume" : "Pause", [this] { OnPause(); }).Enable(pause_enabled);
	bar.Add("Stop", Icons::Stop(), [this] { OnStop(); }).Key(K_CTRL|K_SHIFT|K_F12).Enable(doc_running || vm.IsRunning());
}

void PythonIDE::DebugMenu(Bar& bar)
{
	bool doc_running = false;
	bool doc_paused = false;
	bool doc_can_pause = false;
	if(active_file >= 0 && active_file < open_files.GetCount()) {
		if(IDocumentHost* h = open_files[active_file].editor) {
			doc_running = h->IsRunning();
			doc_paused = h->IsPaused();
			doc_can_pause = h->CanPause();
		}
	}
	bool stop_enabled = doc_running || vm.IsRunning();
	bool run_enabled = !stop_enabled;
	bool pause_enabled = doc_can_pause && doc_running;

	bar.Add("Debug file", Icons::Debug(), [this] { OnDebug(); }).Key(K_F5).Enable(run_enabled);
	bar.Add("Debug cell", [this] { OnDebugCell(); }).Enable(run_enabled);
	bar.Add("Debug the current line or selection", [this] { OnDebugSelection(); }).Enable(run_enabled);
	bar.Separator();
	bar.Add("Debug current line", Icons::StepOver(), [this] { vm.StepOver(); }).Key(K_CTRL_F10).Enable(vm.IsRunning());
	bar.Add("Step into function or method", Icons::StepIn(), [this] { vm.StepIn(); }).Key(K_CTRL_F11).Enable(vm.IsRunning());
	bar.Add("Execute until function returns", Icons::StepOut(), [this] { vm.StepOut(); }).Key(K_CTRL|K_SHIFT|K_F11).Enable(vm.IsRunning());
	bar.Add("Execute until next breakpoint", Icons::Run(), [this] { vm.Run(); }).Key(K_CTRL_F12).Enable(vm.IsRunning());
	bar.Add(doc_paused ? "Resume debugging" : "Pause debugging", [this] { OnPause(); }).Enable(pause_enabled);
	bar.Add("Stop debugging", Icons::Stop(), [this] { OnStop(); }).Key(K_CTRL|K_SHIFT|K_F12).Enable(stop_enabled);
	bar.Separator();
	bar.Add("Toggle breakpoint", Icons::Breakpoint(), [this] { OnToggleBreakpoint(); }).Key(K_F12);
	bar.Add("Set/edit conditional breakpoint", [this] { Todo("Conditional breakpoint"); }).Key(K_SHIFT|K_F12);
	bar.Add("Clear breakpoints in all files", [this] { OnClearBreakpoints(); });
	bar.Add("List breakpoints", [this] { OnListBreakpoints(); });
}

void PythonIDE::ConsolesMenu(Bar& bar)
{
	bar.Add("New console (default settings)", [this] { OnConsoleInput(); }).Key(K_CTRL_T);
	bar.Sub("New console in environment", [this](Bar& b) {
		b.Add("Conda: spyder-runtime 0", [this] { Todo("New Conda console"); });
	});
	bar.Sub("New special console", [this](Bar& b) {
		b.Add("New Pylab console (data plotting)", [this] { Todo("New Pylab console"); });
		b.Add("New SymPy console (symbolic math)", [this] { Todo("New SymPy console"); });
		b.Add("New Cython console (Python with C extensions)", [this] { Todo("New Cython console"); });
	});
	bar.Sub("New console in remote server", [this](Bar& b) {
		b.Add("Manage remote connections", [this] { Todo("Manage remote connections"); });
	});
	bar.Add("Connect to existing kernel...", [this] { Todo("Connect to kernel"); });
	bar.Separator();
	bar.Add("Interrupt kernel", Icons::Stop(), [this] { console_pane->WhenInterrupt(); });
	bar.Add("Restart kernel", [this] { console_pane->WhenRestart(); }).Key(K_CTRL_PERIOD);
	bar.Add("Remove all variables", [this] { console_pane->WhenRemoveVariables(); }).Key(K_CTRL|K_ALT|K_R);
}

void PythonIDE::ProjectsMenu(Bar& bar)
{
	bar.Add("New Project...", [this] { Todo("New Project"); });
	bar.Add("Open Project...", [this] { Todo("Open Project"); });
	bar.Add("Close Project", [this] { Todo("Close Project"); });
	bar.Add("Delete Project", [this] { Todo("Delete Project"); });
	bar.Separator();
	bar.Sub("Recent Projects", [this](Bar& b) {
		b.Add("Clear this list", [this] { Todo("Clear recent projects"); });
		b.Add("Maximum number of recent projects", [this] { Todo("Max recent projects"); });
	});
}

void PythonIDE::ToolsMenu(Bar& bar)
{
	bar.Add("PYTHONPATH manager", Icons::Plus(), [this] { OnPathManager(); });
	bar.Add("User environment variables", [this] { Todo("Env vars"); });
	bar.Add("Manage remote connections", [this] { Todo("Remote connections"); });
	bar.Separator();
	bar.Add("Preferences", Icons::Settings(), [this] { OnSettings(); });
	bar.Add("Reset all preferences to defaults", [this] { Todo("Reset preferences"); });
}

void PythonIDE::WindowMenu(Bar& bar)
{
	bar.Sub("Panes", [this](Bar& b) {
		b.Add("Editor", [this] { Todo("Show Editor"); }).Key(K_CTRL|K_SHIFT|K_E).Check(true);
		b.Add("IPython Console", [this] { OnTogglePane(*console_pane); }).Key(K_CTRL|K_SHIFT|K_I).Check(console_pane->IsDocked());
		b.Add("Variable Explorer", Icons::VariableExplorer(), [this] { OnTogglePane(*var_explorer); }).Key(K_CTRL|K_SHIFT|K_V).Check(var_explorer->IsDocked());
		b.Add("Debugger", Icons::Debug(), [this] { OnTogglePane(*debugger_pane); }).Key(K_CTRL|K_SHIFT|K_D).Check(debugger_pane->IsDocked());
		b.Add("Help", Icons::Help(), [this] { OnTogglePane(*help_pane); }).Key(K_CTRL|K_SHIFT|K_H).Check(help_pane->IsDocked());
		b.Add("Plots", Icons::Plots(), [this] { OnTogglePane(*plots_pane); }).Key(K_CTRL|K_SHIFT|K_G).Check(plots_pane->IsDocked());
		b.Separator();
		b.Add("Files", Icons::Files(), [this] { OnTogglePane(*files_pane); }).Key(K_CTRL|K_SHIFT|K_X).Check(files_pane->IsDocked());
		b.Add("Outline", Icons::Outline(), [this] { OnTogglePane(*outline_pane); }).Key(K_CTRL|K_SHIFT|K_O).Check(outline_pane->IsDocked());
		b.Add("Project", [this] { Todo("Toggle Project Pane"); }).Key(K_CTRL|K_SHIFT|K_P).Check(false);
		b.Add("Find", Icons::Search(), [this] { OnTogglePane(*find_pane); }).Key(K_CTRL|K_SHIFT|K_F).Check(find_pane->IsDocked());
		b.Separator();
		b.Add("History", Icons::History(), [this] { OnTogglePane(*history_pane); }).Key(K_CTRL|K_SHIFT|K_L).Check(history_pane->IsDocked());
		b.Add("Profiler", Icons::Profiler(), [this] { OnTogglePane(*profiler_pane); }).Key(K_CTRL|K_SHIFT|K_R).Check(profiler_pane->IsDocked());
		b.Add("Code Analysis", [this] { Todo("Toggle Code Analysis Pane"); }).Key(K_CTRL|K_SHIFT|K_C).Check(false);
		b.Separator();
		b.Add("Online help", [this] { Todo("Toggle Online Help"); }).Check(false);
		b.Add("Internal console", [this] { Todo("Toggle Internal Console"); }).Check(false);
		
		if(plugin_panes.GetCount() > 0) {
			b.Separator();
			for(int i = 0; i < plugin_panes.GetCount(); i++) {
				String title = plugin_panes.GetKey(i);
				DockableCtrl& pane = plugin_panes[i];
				b.Add(title, [this, &pane] { OnTogglePane(pane); }).Check(pane.IsDocked());
			}
		}
	});
	bar.Add("Unlock panes and toolbars", [this] { Todo("Unlock layout"); }).Key(K_CTRL|K_SHIFT|K_F5);
	bar.Add("Maximize current pane", Icons::Maximize(), [this] { OnMaximizePane(); }).Key(K_CTRL|K_ALT|K_SHIFT|K_M);
	bar.Add("Close current pane", [this] { OnClosePane(); }).Key(K_CTRL|K_SHIFT|K_F4);
	bar.Separator();
	bar.Sub("Toolbars", [this](Bar& b) {
		b.Add("File toolbar", [this] { Todo("Toggle File toolbar"); }).Check(true);
		b.Add("Run toolbar", [this] { Todo("Toggle Run toolbar"); }).Check(true);
		b.Add("Debug toolbar", [this] { Todo("Toggle Debug toolbar"); }).Check(true);
		b.Add("Profile toolbar", [this] { Todo("Toggle Profile toolbar"); }).Check(true);
		b.Add("Main toolbar", [this] { Todo("Toggle Main toolbar"); }).Check(true);
		b.Add("Current working directory", [this] { Todo("Toggle CWD toolbar"); }).Check(true);
	});
	bar.Add("Hide toolbars", [this] { Todo("Hide toolbars"); });
	bar.Separator();
	bar.Sub("Window layouts", [this](Bar& b) {
		b.Add("Default layout", [this] { OnLayoutDefault(); });
		b.Add("Rstudio layout", [this] { OnLayoutRstudio(); });
		b.Add("Matlab layout", [this] { OnLayoutMatlab(); });
		b.Add("Horizontal split", [this] { Todo("Layout Horizontal"); });
		b.Add("Vertical split", [this] { Todo("Layout Vertical"); });
		b.Separator();
		b.Add("Save current layout", [this] { Todo("Save layout"); });
		b.Add("Layout preferences", [this] { Todo("Layout preferences"); });
		b.Add("Reset to Spyder default", [this] { OnLayoutDefault(); });
	});
	bar.Add("Use next layout", [this] { Todo("Next layout"); }).Key(K_ALT|K_SHIFT|K_PAGEDOWN);
	bar.Add("Use previous layout", [this] { Todo("Prev layout"); }).Key(K_ALT|K_SHIFT|K_PAGEUP);
	bar.Separator();
	bar.Add("Fullscreen mode", [this] { OnFullscreen(); }).Key(K_F11);
}

void PythonIDE::HelpMenu(Bar& bar)
{
	bar.Add("Interactive tour", [this] { Todo("Tour"); });
	bar.Add("Built-in tutorial", [this] { Todo("Tutorial"); });
	bar.Add("Shortcuts summary", [this] { Todo("Shortcuts"); });
	bar.Separator();
	bar.Add("Spyder documentation", Icons::Help(), [this] { ShowHelp("index"); }).Key(K_F1);
	bar.Add("Tutorial videos", [this] { Todo("Videos"); });
	bar.Sub("IPython documentation", [this](Bar& b) {
		b.Add("Intro to IPython", [this] { Todo("IPython Intro"); });
		b.Add("Console help", [this] { Todo("Console Help"); });
		b.Add("Quick reference", [this] { Todo("Quick Ref"); });
	});
	bar.Add("Troubleshooting guide", [this] { Todo("Troubleshooting"); });
	bar.Add("Spyder Google group", [this] { Todo("Google Group"); });
	bar.Add("Dependency status", [this] { Todo("Dependencies"); });
	bar.Add("Report issue...", [this] { Todo("Report issue"); });
	bar.Separator();
	bar.Add("Check for updates", [this] { Todo("Updates"); });
	bar.Add("Help Spyder...", Icons::Help(), [this] { Todo("Help Spyder"); });
	bar.Add("About Spyder", Icons::Info(), [this] { Todo("About"); });
}

}
