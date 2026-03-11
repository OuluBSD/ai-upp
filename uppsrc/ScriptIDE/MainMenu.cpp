#include "ScriptIDE.h"

namespace Upp {

void PythonIDE::MainMenu(Bar& bar)
{
	bar.Sub("File",      [=](Bar& b){ FileMenu(b); });
	bar.Sub("Edit",      [=](Bar& b){ EditMenu(b); });
	bar.Sub("Search",    [=](Bar& b){ SearchMenu(b); });
	bar.Sub("Source",    [=](Bar& b){ SourceMenu(b); });
	
	if(active_editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor))
			h->MainMenu(bar);
	}

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
	bar.Add("New file...", ScriptIDEImg::IconNewFile(), [=] { OnNewFile(); }).Key(K_CTRL_N);
	bar.Separator();
	bar.Add("Open...", ScriptIDEImg::IconOpenFile(), [=] { OnOpenFile(); }).Key(K_CTRL_O);
	bar.Add("Open last closed", [=] { OnOpenLastClosed(); }).Key(K_CTRL|K_SHIFT|K_T);
	bar.Sub("Open recent", [=](Bar& b) { UpdateRecentFilesMenu(b); });
	bar.Separator();
	bar.Add("Save", ScriptIDEImg::IconSave(), [=] { OnSaveFile(); }).Key(K_CTRL_S);
	bar.Add("Save all", ScriptIDEImg::IconSave(), [=] { OnSaveAll(); }).Key(K_CTRL|K_ALT|K_S);
	bar.Add("Save as...", ScriptIDEImg::IconSaveAs(), [=] { OnSaveFileAs(); }).Key(K_CTRL|K_SHIFT|K_S);
	bar.Add("Save copy as...", [=] { OnSaveCopyAs(); });
	bar.Add("Revert", [=] { OnRevert(); });
	bar.Separator();
	bar.Add("Print preview", [=] { Todo("Print preview"); });
	bar.Add("Print...", [=] { Todo("Print"); });
	bar.Separator();
	bar.Add("Close", [=] { OnCloseFile(); });
	bar.Add("Close all", [=] { OnCloseAll(); }).Key(K_CTRL|K_SHIFT|K_W);
	bar.Separator();
	bar.Add("File switcher...", [=] { OnFileSwitcher(); }).Key(K_CTRL_P);
	bar.Add("Symbol finder...", [=] { OnSymbolFinder(); }).Key(K_CTRL|K_ALT|K_P);
	bar.Separator();
	bar.Add("Restart", [=] { OnRestart(); }).Key(K_ALT|K_SHIFT|K_R);
	bar.Add("Restart in debug mode", [=] { Todo("Restart in debug mode"); });
	bar.Add("Quit", [=] { Close(); }).Key(K_CTRL_Q);
}

void PythonIDE::EditMenu(Bar& bar)
{
	bar.Add("Undo", [=] { OnUndo(); }).Key(K_CTRL_Z);
	bar.Add("Redo", [=] { OnRedo(); }).Key(K_CTRL|K_SHIFT|K_Z);
	bar.Separator();
	bar.Add("Cut", [=] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) h->Cut(); } }).Key(K_CTRL_X);
	bar.Add("Copy", [=] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) h->Copy(); } }).Key(K_CTRL_C);
	bar.Add("Paste", [=] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) h->Paste(); } }).Key(K_CTRL_V);
	bar.Add("Select All", [=] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) h->SelectAll(); } }).Key(K_CTRL_A);
	bar.Separator();
	bar.Add("Comment/uncomment", [=] { OnComment(); }).Key(K_CTRL_1);
	bar.Add("Add block comment", [=] { OnBlockComment(); }).Key(K_CTRL_4);
	bar.Add("Remove block comment", [=] { OnUncomment(); }).Key(K_CTRL_5);
	bar.Separator();
	bar.Add("Indent", [=] { Todo("Indent"); });
	bar.Add("Unindent", [=] { Todo("Unindent"); });
	bar.Separator();
	bar.Add("Toggle UPPERCASE", [=] { OnToggleCase(true); }).Key(K_ALT|K_SHIFT|K_U);
	bar.Add("Toggle lowercase", [=] { OnToggleCase(false); }).Key(K_ALT_U);
	bar.Separator();
	bar.Sub("Convert end-of-line characters", [=](Bar& b) {
		b.Add("LF (Linux/macOS)", [=] { OnConvertEOL("LF"); });
		b.Add("CRLF (Windows)", [=] { OnConvertEOL("CRLF"); });
		b.Add("CR (legacy Mac)", [=] { OnConvertEOL("CR"); });
	});
	bar.Add("Remove trailing spaces", [=] { OnRemoveTrailingSpaces(); });
	bar.Add("Convert tabs to spaces", [=] { OnTabsToSpaces(); });
}

void PythonIDE::SearchMenu(Bar& bar)
{
	bar.Add("Find text", [=] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) h->Find(); } });
	bar.Add("Find next", [=] { Todo("Find next"); });
	bar.Add("Find previous", [=] { Todo("Find previous"); });
	bar.Add("Replace text", [=] { if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) h->Replace(); } });
	bar.Separator();
	bar.Add("Last edit location", [=] { Todo("Last edit location"); }).Key(K_CTRL|K_ALT|K_SHIFT|K_LEFT);
	bar.Add("Previous cursor position", [=] { OnPrevCursor(); }).Key(K_ALT_LEFT);
	bar.Add("Next cursor position", [=] { OnNextCursor(); }).Key(K_ALT_RIGHT);
	bar.Separator();
	bar.Add("Search text in files...", [=] { OnTogglePane(*find_pane); }).Key(K_ALT|K_SHIFT|K_F);
}

void PythonIDE::SourceMenu(Bar& bar)
{
	bar.Add("Show invisible characters", [=] { Todo("Toggle invisible characters"); }).Check(false);
	bar.Add("Wrap lines", [=] { Todo("Toggle wrap lines"); }).Check(false);
	bar.Add("Show indent guides", [=] { Todo("Toggle indent guides"); }).Check(true);
	bar.Add("Show code folding", [=] { Todo("Toggle code folding"); }).Check(true);
	bar.Add("Show class/function selector", [=] { Todo("Toggle symbol selector"); }).Check(true);
	bar.Add("Show docstring style warnings", [=] { Todo("Toggle docstring warnings"); }).Check(false);
	bar.Add("Underline errors and warnings", [=] { Todo("Toggle lint underlines"); }).Check(true);
	bar.Separator();
	bar.Add("Show todo list", [=] { Todo("Show todo list"); }).Enable(false);
	bar.Add("Show warning/error list", [=] { Todo("Show warning/error list"); }).Enable(false);
	bar.Add("Previous warning/error", [=] { Todo("Prev diagnostic"); }).Key(K_CTRL|K_ALT|K_SHIFT|K_COMMA);
	bar.Add("Next warning/error", [=] { Todo("Next diagnostic"); }).Key(K_CTRL|K_ALT|K_SHIFT|K_PERIOD);
	bar.Separator();
	bar.Add("Run code analysis", [=] { OnAnalyze(); }).Key(K_F8);
	bar.Add("Format file or selection with Autopep8", [=] { Todo("Autopep8"); });
}

void PythonIDE::RunMenu(Bar& bar)
{
	bar.Add("Run", ScriptIDEImg::IconRun(), [=] { OnRun(); }).Key(K_F5);
	bar.Add("Re-run last file", [=] { OnRunLast(); }).Key(K_F6);
	bar.Add("Configuration per file", [=] { OnRunConfig(); }).Key(K_CTRL_F6);
	bar.Add("Global presets", [=] { Todo("Global presets"); });
	bar.Separator();
	bar.Add("Run cell", [=] { OnRunCell(); }).Key(K_CTRL_RETURN);
	bar.Add("Run cell and advance", [=] { OnRunCellAndAdvance(); }).Key(K_SHIFT_RETURN);
	bar.Add("Re-run last cell", [=] { Todo("Re-run last cell"); }).Key(K_ALT_RETURN);
	bar.Add("Run current line/selection", [=] { OnRunSelection(); }).Key(K_F9);
	bar.Add("Run to line", [=] { OnRunToLine(); }).Key(K_SHIFT_F9);
	bar.Add("Run from line", [=] { OnRunFromLine(); }).Key(K_ALT_F9);
	bar.Separator();
	bar.Add("Run in external terminal", [=] { Todo("Run in external terminal"); });
	bar.Separator();
	bar.Add("Profile file", [=] { Todo("Profile file"); }).Key(K_F10);
	bar.Add("Profile cell", [=] { Todo("Profile cell"); }).Key(K_ALT_F10);
	bar.Add("Profile current line or selection", [=] { Todo("Profile selection"); });
}

void PythonIDE::DebugMenu(Bar& bar)
{
	bar.Add("Debug file", ScriptIDEImg::IconDebug(), [=] { OnDebug(); }).Key(K_CTRL|K_F5);
	bar.Add("Debug cell", [=] { OnDebugCell(); });
	bar.Add("Debug the current line or selection", [=] { OnDebugSelection(); });
	bar.Separator();
	bar.Add("Debug current line", [=] { vm.StepOver(); }).Key(K_CTRL_F10).Enable(vm.IsRunning());
	bar.Add("Step into function or method", [=] { vm.StepIn(); }).Key(K_CTRL_F11).Enable(vm.IsRunning());
	bar.Add("Execute until function returns", [=] { vm.StepOut(); }).Key(K_CTRL|K_SHIFT|K_F11).Enable(vm.IsRunning());
	bar.Add("Execute until next breakpoint", [=] { vm.Run(); }).Key(K_CTRL_F12).Enable(vm.IsRunning());
	bar.Add("Stop debugging", ScriptIDEImg::IconStop(), [=] { OnStop(); }).Key(K_CTRL|K_SHIFT|K_F12).Enable(vm.IsRunning());
	bar.Separator();
	bar.Add("Toggle breakpoint", ScriptIDEImg::IconBreakpoint(), [=] { OnToggleBreakpoint(); }).Key(K_F12);
	bar.Add("Set/edit conditional breakpoint", [=] { Todo("Conditional breakpoint"); }).Key(K_SHIFT|K_F12);
	bar.Add("Clear breakpoints in all files", [=] { OnClearBreakpoints(); });
	bar.Add("List breakpoints", [=] { OnListBreakpoints(); });
}

void PythonIDE::ConsolesMenu(Bar& bar)
{
	bar.Add("New console (default settings)", [=] { OnConsoleInput(); }).Key(K_CTRL_T);
	bar.Sub("New console in environment", [=](Bar& b) {
		b.Add("Conda: spyder-runtime 0", [=] { Todo("New Conda console"); });
	});
	bar.Sub("New special console", [=](Bar& b) {
		b.Add("New Pylab console (data plotting)", [=] { Todo("New Pylab console"); });
		b.Add("New SymPy console (symbolic math)", [=] { Todo("New SymPy console"); });
		b.Add("New Cython console (Python with C extensions)", [=] { Todo("New Cython console"); });
	});
	bar.Sub("New console in remote server", [=](Bar& b) {
		b.Add("Manage remote connections", [=] { Todo("Manage remote connections"); });
	});
	bar.Add("Connect to existing kernel...", [=] { Todo("Connect to kernel"); });
	bar.Separator();
	bar.Add("Interrupt kernel", ScriptIDEImg::IconStop(), [=] { console_pane->WhenInterrupt(); });
	bar.Add("Restart kernel", [=] { console_pane->WhenRestart(); }).Key(K_CTRL_PERIOD);
	bar.Add("Remove all variables", [=] { console_pane->WhenRemoveVariables(); }).Key(K_CTRL|K_ALT|K_R);
}

void PythonIDE::ProjectsMenu(Bar& bar)
{
	bar.Add("New Project...", [=] { Todo("New Project"); });
	bar.Add("Open Project...", [=] { Todo("Open Project"); });
	bar.Add("Close Project", [=] { Todo("Close Project"); });
	bar.Add("Delete Project", [=] { Todo("Delete Project"); });
	bar.Separator();
	bar.Sub("Recent Projects", [=](Bar& b) {
		b.Add("Clear this list", [=] { Todo("Clear recent projects"); });
		b.Add("Maximum number of recent projects", [=] { Todo("Max recent projects"); });
	});
}

void PythonIDE::ToolsMenu(Bar& bar)
{
	bar.Add("PYTHONPATH manager", [=] { OnPathManager(); });
	bar.Add("User environment variables", [=] { Todo("Env vars"); });
	bar.Add("Manage remote connections", [=] { Todo("Remote connections"); });
	bar.Separator();
	bar.Add("Preferences", [=] { OnSettings(); });
	bar.Add("Reset all preferences to defaults", [=] { Todo("Reset preferences"); });
}

void PythonIDE::WindowMenu(Bar& bar)
{
	bar.Sub("Panes", [=](Bar& b) {
		b.Add("Editor", [=] { Todo("Show Editor"); }).Key(K_CTRL|K_SHIFT|K_E).Check(true);
		b.Add("IPython Console", [=] { OnTogglePane(*console_pane); }).Key(K_CTRL|K_SHIFT|K_I).Check(console_pane->IsDocked());
		b.Add("Variable Explorer", [=] { OnTogglePane(*var_explorer); }).Key(K_CTRL|K_SHIFT|K_V).Check(var_explorer->IsDocked());
		b.Add("Debugger", [=] { OnTogglePane(*debugger_pane); }).Key(K_CTRL|K_SHIFT|K_D).Check(debugger_pane->IsDocked());
		b.Add("Help", [=] { OnTogglePane(*help_pane); }).Key(K_CTRL|K_SHIFT|K_H).Check(help_pane->IsDocked());
		b.Add("Plots", [=] { OnTogglePane(*plots_pane); }).Key(K_CTRL|K_SHIFT|K_G).Check(plots_pane->IsDocked());
		b.Separator();
		b.Add("Files", [=] { OnTogglePane(*files_pane); }).Key(K_CTRL|K_SHIFT|K_X).Check(files_pane->IsDocked());
		b.Add("Outline", [=] { OnTogglePane(*outline_pane); }).Key(K_CTRL|K_SHIFT|K_O).Check(outline_pane->IsDocked());
		b.Add("Project", [=] { Todo("Toggle Project Pane"); }).Key(K_CTRL|K_SHIFT|K_P).Check(false);
		b.Add("Find", [=] { OnTogglePane(*find_pane); }).Key(K_CTRL|K_SHIFT|K_F).Check(find_pane->IsDocked());
		b.Separator();
		b.Add("History", [=] { OnTogglePane(*history_pane); }).Key(K_CTRL|K_SHIFT|K_L).Check(history_pane->IsDocked());
		b.Add("Profiler", [=] { OnTogglePane(*profiler_pane); }).Key(K_CTRL|K_SHIFT|K_R).Check(profiler_pane->IsDocked());
		b.Add("Code Analysis", [=] { Todo("Toggle Code Analysis Pane"); }).Key(K_CTRL|K_SHIFT|K_C).Check(false);
		b.Separator();
		b.Add("Online help", [=] { Todo("Toggle Online Help"); }).Check(false);
		b.Add("Internal console", [=] { Todo("Toggle Internal Console"); }).Check(false);
		
		if(plugin_panes.GetCount() > 0) {
			b.Separator();
			for(int i = 0; i < plugin_panes.GetCount(); i++) {
				String title = plugin_panes.GetKey(i);
				DockableCtrl& pane = plugin_panes[i];
				b.Add(title, [=, &pane] { OnTogglePane(pane); }).Check(pane.IsDocked());
			}
		}
	});
	bar.Add("Unlock panes and toolbars", [=] { Todo("Unlock layout"); }).Key(K_CTRL|K_SHIFT|K_F5);
	bar.Add("Maximize current pane", [=] { OnMaximizePane(); }).Key(K_CTRL|K_ALT|K_SHIFT|K_M);
	bar.Add("Close current pane", [=] { OnClosePane(); }).Key(K_CTRL|K_SHIFT|K_F4);
	bar.Separator();
	bar.Sub("Toolbars", [=](Bar& b) {
		b.Add("File toolbar", [=] { Todo("Toggle File toolbar"); }).Check(true);
		b.Add("Run toolbar", [=] { Todo("Toggle Run toolbar"); }).Check(true);
		b.Add("Debug toolbar", [=] { Todo("Toggle Debug toolbar"); }).Check(true);
		b.Add("Profile toolbar", [=] { Todo("Toggle Profile toolbar"); }).Check(true);
		b.Add("Main toolbar", [=] { Todo("Toggle Main toolbar"); }).Check(true);
		b.Add("Current working directory", [=] { Todo("Toggle CWD toolbar"); }).Check(true);
	});
	bar.Add("Hide toolbars", [=] { Todo("Hide toolbars"); });
	bar.Separator();
	bar.Sub("Window layouts", [=](Bar& b) {
		b.Add("Default layout", [=] { OnLayoutDefault(); });
		b.Add("Rstudio layout", [=] { OnLayoutRstudio(); });
		b.Add("Matlab layout", [=] { OnLayoutMatlab(); });
		b.Add("Horizontal split", [=] { Todo("Layout Horizontal"); });
		b.Add("Vertical split", [=] { Todo("Layout Vertical"); });
		b.Separator();
		b.Add("Save current layout", [=] { Todo("Save layout"); });
		b.Add("Layout preferences", [=] { Todo("Layout preferences"); });
		b.Add("Reset to Spyder default", [=] { OnLayoutDefault(); });
	});
	bar.Add("Use next layout", [=] { Todo("Next layout"); }).Key(K_ALT|K_SHIFT|K_PAGEDOWN);
	bar.Add("Use previous layout", [=] { Todo("Prev layout"); }).Key(K_ALT|K_SHIFT|K_PAGEUP);
	bar.Separator();
	bar.Add("Fullscreen mode", [=] { OnFullscreen(); }).Key(K_F11);
}

void PythonIDE::HelpMenu(Bar& bar)
{
	bar.Add("Interactive tour", [=] { Todo("Tour"); });
	bar.Add("Built-in tutorial", [=] { Todo("Tutorial"); });
	bar.Add("Shortcuts summary", [=] { Todo("Shortcuts"); });
	bar.Separator();
	bar.Add("Spyder documentation", [=] { ShowHelp("index"); }).Key(K_F1);
	bar.Add("Tutorial videos", [=] { Todo("Videos"); });
	bar.Sub("IPython documentation", [=](Bar& b) {
		b.Add("Intro to IPython", [=] { Todo("IPython Intro"); });
		b.Add("Console help", [=] { Todo("Console Help"); });
		b.Add("Quick reference", [=] { Todo("Quick Ref"); });
	});
	bar.Add("Troubleshooting guide", [=] { Todo("Troubleshooting"); });
	bar.Add("Spyder Google group", [=] { Todo("Google Group"); });
	bar.Add("Dependency status", [=] { Todo("Dependencies"); });
	bar.Add("Report issue...", [=] { Todo("Report issue"); });
	bar.Separator();
	bar.Add("Check for updates", [=] { Todo("Updates"); });
	bar.Add("Help Spyder...", [=] { Todo("Help Spyder"); });
	bar.Add("About Spyder", [=] { Todo("About"); });
}

}
