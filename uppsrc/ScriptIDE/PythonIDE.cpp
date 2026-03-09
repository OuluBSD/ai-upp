#include "ScriptIDE.h"

namespace Upp {

void PythonIDE::InitLayout()
{
	editor_tabs.WhenNewTab = [=] { OnNewTab(); };
	editor_tabs.WhenTabMenu = [=](Bar& bar) { OnTabMenu(bar); };

	// Add tabs to editor area
	editor_area.Add(editor_tabs.BottomPos(0, 25).HSizePos());
	editor_area.Add(code_editor.VSizePos(0, 25).HSizePos());

	// Add editor_area to main window (it will be central control)
	Add(editor_area.SizePos());

	code_editor.Highlight("python");
	code_editor.EnableBreakpointing();
}

void PythonIDE::InitDocking()
{
}

void PythonIDE::UpdateVariableExplorer()
{
	if(vm.IsRunning() && vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
		// Get variables from current frame
		const auto& stack = vm.GetCallStack();
		if(stack.GetCount() > 0) {
			const auto& locals = vm.GetLocals(stack[0].frame_index);
			var_explorer.SetVariables(locals);
		}
	}
	else {
		var_explorer.SetVariables(vm.GetGlobals());
	}
}

void PythonIDE::OnAnalyze()
{
	String code = code_editor.Get();
	String filename = current_file.path.IsEmpty() ? String("<editor>") : current_file.path;
	
	Vector<Linter::Message> msgs = linter.Analyze(code, filename);
	
	Vector<Point> err_points;
	code_editor.ClearAnnotations();
	
	for(const auto& m : msgs) {
		if(m.line > 0) {
			err_points.Add(Point(m.column, m.line - 1));
			code_editor.SetAnnotation(m.line - 1, CtrlImg::remove(), m.text);
		}
	}
	
	code_editor.Errors(pick(err_points));
}

void PythonIDE::OnNewTab()
{
	// Create new empty file
	editor_tabs.AddFile("<untitled>", CtrlImg::File());
}

void PythonIDE::OnTabMenu(Bar& bar)
{
	bar.Add("Close All", [=] { /* ... */ });
	bar.Add("Close Others", [=] { /* ... */ });
	bar.Separator();
	bar.Add("Tabs at Bottom", [=] {
		// Toggle tab position
		editor_tabs.SetAlign(editor_tabs.GetAlign() == AlignedFrame::BOTTOM ?
		                     AlignedFrame::TOP : AlignedFrame::BOTTOM);
	});
}

void PythonIDE::OnConsoleInput()
{
	String cmd = console_pane.GetInput();
	if(cmd.IsEmpty()) return;

	run_manager.RunSelection(cmd);
}

PythonIDE::PythonIDE() : run_manager(vm)
{
    Title("ScriptIDE - Python IDE");
    Sizeable().Zoomable().CenterScreen();
    SetRect(0, 0, 1400, 900);

    AddFrame(menubar);
    AddFrame(toolbar);
    AddFrame(statusbar);

    // Load settings
    LoadFromFile(settings, ConfigFile("ide_settings.bin"));
    LoadFromFile(path_manager, ConfigFile("pythonpath.bin"));

    InitLayout();
    
    path_manager.SyncToVM(vm);

    menubar.Set([=](Bar& bar) { MainMenu(bar); });

    files_pane.WhenOpen = [=](const String& path) { if(ConfirmSave()) LoadFile(path); };

    outline_pane.WhenSelectLine = [=](int line) {
        code_editor.SetCursor(code_editor.GetPos(line - 1));
        code_editor.SetFocus();
    };

    find_pane.WhenOpenMatch = [=](const String& path, int line) {
        if(path != current_file.path) {
            if(ConfirmSave()) LoadFile(path);
        }
        code_editor.SetCursor(code_editor.GetPos(line - 1));
        code_editor.SetFocus();
    };

    console_pane.WhenInput = [=] { OnConsoleInput(); };

    debugger_pane.WhenContinue = [=] { vm.Continue(); };
    debugger_pane.WhenStepOver = [=] { OnStepOver(); };
    debugger_pane.WhenStepInto = [=] { OnStepIn(); };
    debugger_pane.WhenStepOut = [=] { OnStepOut(); };
    debugger_pane.WhenStop = [=] { OnStop(); };
    debugger_pane.WhenFrameSelected = [=](int i) {
        const auto& locals = vm.GetLocals(i);
        var_explorer.SetVariables(locals);
    };

    run_manager.WhenStarted = [=] {
        code_editor.HidePtr();
        profiler_pane.Clear();
    };
    run_manager.WhenFinished = [=] {
        UpdateVariableExplorer();
        debugger_pane.Clear();
    };
    run_manager.WhenError = [=](const String& e) {
        console_pane.WriteError("Runtime error: " + e + "\n");
    };

    vm.WhenPrint = [=](const String& s) { console_pane.Write(s); };
    vm.WhenPlot = [=](const Image& img) { plots_pane.AddPlot(img); };
    vm.WhenBreakpointHit = [=](const String& file, int line) { OnBreakpointHit(file, line); };

    code_editor.WhenAction = [=] {
        current_file.dirty = true;
        KillTimeCallback(1);
        SetTimeCallback(500, [=] { OnAnalyze(); }, 1);
    };

    SetTimeCallback(-500, [=] { UpdateStatusBar(); });
    UpdateStatusBar();
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
	bar.Add("New file...", CtrlImg::new_doc(), [=] { Todo("New file"); }).Key(K_CTRL_N);
	bar.Separator();
	bar.Add("Open...", CtrlImg::open(), [=] { Todo("Open"); }).Key(K_CTRL_O);
	bar.Add("Open last closed", [=] { Todo("Open last closed"); }).Key(K_CTRL|K_SHIFT|K_T);
	bar.Sub("Open recent", [=](Bar& b) {
		b.Add("Maximum number of recent files...", [=] { Todo("Recent files count"); });
		b.Add("Clear this list", [=] { Todo("Clear recent"); });
	});
	bar.Separator();
	bar.Add("Save", CtrlImg::save(), [=] { Todo("Save"); }).Key(K_CTRL_S);
	bar.Add("Save all", [=] { Todo("Save all"); }).Key(K_CTRL|K_ALT|K_S);
	bar.Add("Save as...", CtrlImg::save_as(), [=] { Todo("Save as"); }).Key(K_CTRL|K_SHIFT|K_S);
	bar.Add("Save copy as...", [=] { Todo("Save copy as"); });
	bar.Add("Revert", [=] { Todo("Revert"); });
	bar.Separator();
	bar.Add("Print preview", [=] { Todo("Print preview"); });
	bar.Add("Print...", [=] { Todo("Print"); });
	bar.Separator();
	bar.Add("Close", [=] { Todo("Close"); });
	bar.Add("Close all", [=] { Todo("Close all"); }).Key(K_CTRL|K_SHIFT|K_W);
	bar.Separator();
	bar.Add("File switcher...", [=] { Todo("File switcher"); }).Key(K_CTRL_P);
	bar.Add("Symbol finder...", [=] { Todo("Symbol finder"); }).Key(K_CTRL|K_ALT|K_P);
	bar.Separator();
	bar.Add("Restart", [=] { Todo("Restart"); }).Key(K_ALT|K_SHIFT|K_R);
	bar.Add("Restart in debug mode", [=] { Todo("Restart in debug"); });
	bar.Add("Quit", [=] { Close(); }).Key(K_CTRL_Q);
}

void PythonIDE::EditMenu(Bar& bar)
{
	bar.Add("Undo", CtrlImg::undo(), [=] { code_editor.Undo(); }).Key(K_CTRL_Z);
	bar.Add("Redo", CtrlImg::redo(), [=] { code_editor.Redo(); }).Key(K_CTRL|K_SHIFT|K_Z);
	bar.Separator();
	bar.Add("Cut", CtrlImg::cut(), [=] { code_editor.Cut(); }).Key(K_CTRL_X);
	bar.Add("Copy", CtrlImg::copy(), [=] { code_editor.Copy(); }).Key(K_CTRL_C);
	bar.Add("Paste", CtrlImg::paste(), [=] { code_editor.Paste(); }).Key(K_CTRL_V);
	bar.Add("Select All", [=] { code_editor.SelectAll(); }).Key(K_CTRL_A);
	bar.Separator();
	bar.Add("Comment/uncomment", [=] { Todo("Comment"); }).Key(K_CTRL_1);
	bar.Add("Add block comment", [=] { Todo("Add block comment"); }).Key(K_CTRL_4);
	bar.Add("Remove block comment", [=] { Todo("Remove block comment"); }).Key(K_CTRL_5);
	bar.Separator();
	bar.Add("Indent", [=] { code_editor.TabRight(); });
	bar.Add("Unindent", [=] { code_editor.TabLeft(); });
	bar.Separator();
	bar.Add("Toggle UPPERCASE", [=] { Todo("UPPERCASE"); }).Key(K_ALT|K_SHIFT|K_U);
	bar.Add("Toggle lowercase", [=] { Todo("lowercase"); }).Key(K_ALT|K_U);
	bar.Separator();
	bar.Sub("Convert end-of-line characters", [=](Bar& b) {
		b.Add("LF (Linux/macOS)", [=] { Todo("EOL LF"); });
		b.Add("CRLF (Windows)", [=] { Todo("EOL CRLF"); });
		b.Add("CR (legacy Mac)", [=] { Todo("EOL CR"); });
	});
	bar.Add("Remove trailing spaces", [=] { Todo("Trailing spaces"); });
	bar.Add("Convert tabs to spaces", [=] { Todo("Tabs to spaces"); });
}

void PythonIDE::SearchMenu(Bar& bar)
{
	bar.Add("Find text", CtrlImg::plus(), [=] { code_editor.DoFind(); }); // Using plus as search icon
	bar.Add("Find next", [=] { code_editor.FindNext(); });
	bar.Add("Find previous", [=] { code_editor.FindPrev(); });
	bar.Add("Replace text", [=] { code_editor.Replace(); });
	bar.Separator();
	bar.Add("Last edit location", [=] { Todo("Last edit location"); }).Key(K_CTRL|K_ALT|K_SHIFT|K_LEFT);
	bar.Add("Previous cursor position", [=] { Todo("Prev cursor"); }).Key(K_ALT_LEFT);
	bar.Add("Next cursor position", [=] { Todo("Next cursor"); }).Key(K_ALT_RIGHT);
	bar.Separator();
	bar.Add("Search text in files...", [=] { find_pane.Show(); }).Key(K_ALT|K_SHIFT|K_F);
}

void PythonIDE::SourceMenu(Bar& bar)
{
	bar.Add("Show invisible characters", [=] { Todo("Invisibles"); }).Check(false);
	bar.Add("Wrap lines", [=] { Todo("Wrap lines"); }).Check(false);
	bar.Add("Show indent guides", [=] { Todo("Indent guides"); }).Check(false);
	bar.Add("Show code folding", [=] { Todo("Folding"); }).Check(true);
	bar.Add("Show class/function selector", [=] { Todo("Selector"); }).Check(false);
	bar.Add("Show docstring style warnings", [=] { Todo("Docstring warnings"); }).Check(false);
	bar.Add("Underline errors and warnings", [=] { Todo("Underline"); }).Check(true);
	bar.Separator();
	bar.Add("Show todo list", [=] { Todo("Todo list"); }).Enable(false);
	bar.Add("Show warning/error list", [=] { Todo("Warning list"); }).Enable(false);
	bar.Add("Previous warning/error", [=] { Todo("Prev warn"); }).Key(K_CTRL|K_ALT|K_SHIFT|K_COMMA);
	bar.Add("Next warning/error", [=] { Todo("Next warn"); }).Key(K_CTRL|K_ALT|K_SHIFT|K_PERIOD);
	bar.Separator();
	bar.Add("Run code analysis", [=] { OnAnalyze(); }).Key(K_F8);
	bar.Add("Format file or selection with Autopep8", [=] { Todo("Autopep8"); });
}

void PythonIDE::RunMenu(Bar& bar)
{
	bar.Add("Run", CtrlImg::right_arrow(), [=] { OnRun(); }).Key(K_F5);
	bar.Add("Re-run last file", [=] { Todo("Re-run"); }).Key(K_F6);
	bar.Add("Configuration per file", [=] { OnRunConfig(); }).Key(K_CTRL_F6);
	bar.Add("Global presets", [=] { Todo("Presets"); });
	bar.Separator();
	bar.Add("Run cell", [=] { Todo("Run cell"); }).Key(K_CTRL|K_ENTER);
	bar.Add("Run cell and advance", [=] { Todo("Run cell and advance"); }).Key(K_SHIFT|K_ENTER);
	bar.Add("Re-run last cell", [=] { Todo("Re-run last cell"); }).Key(K_ALT|K_ENTER);
	bar.Add("Run current line/selection", [=] { OnRunSelection(); }).Key(K_F9);
	bar.Add("Run to line", [=] { Todo("Run to line"); }).Key(K_SHIFT|K_F9);
	bar.Add("Run from line", [=] { Todo("Run from line"); }).Key(K_ALT|K_F9);
	bar.Separator();
	bar.Add("Run in external terminal", [=] { Todo("External terminal"); });
	bar.Separator();
	bar.Add("Profile file", [=] { Todo("Profile file"); }).Key(K_F10);
	bar.Add("Profile cell", [=] { Todo("Profile cell"); }).Key(K_ALT|K_F10);
	bar.Add("Profile current line or selection", [=] { Todo("Profile selection"); });
}

void PythonIDE::DebugMenu(Bar& bar)
{
	bar.Add("Debug file", [=] { Todo("Debug file"); }).Key(K_CTRL_F5);
	bar.Add("Debug cell", [=] { Todo("Debug cell"); });
	bar.Add("Debug the current line or selection", [=] { Todo("Debug selection"); });
	bar.Separator();
	bar.Add("Debug current line", [=] { Todo("Debug current line"); }).Key(K_CTRL_F10);
	bar.Add("Step into function or method", [=] { OnStepIn(); }).Key(K_CTRL_F11);
	bar.Add("Execute until function returns", [=] { OnStepOut(); }).Key(K_CTRL|K_SHIFT|K_F11);
	bar.Add("Execute until next breakpoint", [=] { vm.Continue(); }).Key(K_CTRL_F12);
	bar.Add("Stop debugging", [=] { OnStop(); }).Key(K_CTRL|K_SHIFT|K_F12);
	bar.Separator();
	bar.Add("Toggle breakpoint", [=] { OnToggleBreakpoint(); }).Key(K_F12);
	bar.Add("Set/edit conditional breakpoint", [=] { Todo("Conditional BP"); }).Key(K_SHIFT_F12);
	bar.Add("Clear breakpoints in all files", [=] { Todo("Clear all BP"); });
	bar.Add("List breakpoints", [=] { Todo("List BP"); });
}

void PythonIDE::ConsolesMenu(Bar& bar)
{
	bar.Add("New console (default settings)", [=] { Todo("New console"); }).Key(K_CTRL_T);
	bar.Sub("New console in environment", [=](Bar& b) {
		b.Add("Conda: spyder-runtime 0", [=] { Todo("Conda console"); });
	});
	bar.Sub("New special console", [=](Bar& b) {
		b.Add("New Pylab console (data plotting)", [=] { Todo("Pylab console"); });
		b.Add("New SymPy console (symbolic math)", [=] { Todo("SymPy console"); });
		b.Add("New Cython console (Python with C extensions)", [=] { Todo("Cython console"); });
	});
	bar.Sub("New console in remote server", [=](Bar& b) {
		b.Add("Manage remote connections", [=] { Todo("Manage remote"); });
	});
	bar.Add("Connect to existing kernel...", [=] { Todo("Connect kernel"); });
	bar.Separator();
	bar.Add("Interrupt kernel", [=] { Todo("Interrupt kernel"); });
	bar.Add("Restart kernel", [=] { Todo("Restart kernel"); }).Key(K_CTRL_PERIOD);
	bar.Add("Remove all variables", [=] { Todo("Remove variables"); }).Key(K_CTRL|K_ALT|K_R);
}

void PythonIDE::ProjectsMenu(Bar& bar)
{
	bar.Add("New Project...", [=] { Todo("New Project"); });
	bar.Add("Open Project...", [=] { Todo("Open Project"); });
	bar.Add("Close Project", [=] { Todo("Close Project"); });
	bar.Add("Delete Project", [=] { Todo("Delete Project"); });
	bar.Separator();
	bar.Sub("Recent Projects", [=](Bar& b) {
		b.Add("Clear this list", [=] { Todo("Clear projects"); });
		b.Add("Maximum number of recent projects", [=] { Todo("Projects count"); });
	});
}

void PythonIDE::ToolsMenu(Bar& bar)
{
	bar.Add("PYTHONPATH manager", [=] { OnPathManager(); });
	bar.Add("User environment variables", [=] { Todo("Env vars"); });
	bar.Add("Manage remote connections", [=] { Todo("Remote conn"); });
	bar.Separator();
	bar.Add("Preferences", [=] { OnSettings(); });
	bar.Add("Reset all preferences to defaults", [=] { Todo("Reset prefs"); });
}

void PythonIDE::WindowMenu(Bar& bar)
{
	bar.Sub("Panes", [=](Bar& b) {
		b.Add("Editor", [=] { Todo("Toggle Editor"); }).Key(K_CTRL|K_SHIFT|K_E).Check(true);
		b.Add("IPython Console", [=] { console_pane.Show(!console_pane.IsVisible()); }).Key(K_CTRL|K_SHIFT|K_I).Check(console_pane.IsVisible());
		b.Add("Variable Explorer", [=] { var_explorer.Show(!var_explorer.IsVisible()); }).Key(K_CTRL|K_SHIFT|K_V).Check(var_explorer.IsVisible());
		b.Add("Debugger", [=] { debugger_pane.Show(!debugger_pane.IsVisible()); }).Key(K_CTRL|K_SHIFT|K_D).Check(debugger_pane.IsVisible());
		b.Add("Help", [=] { help_pane.Show(!help_pane.IsVisible()); }).Key(K_CTRL|K_SHIFT|K_H).Check(help_pane.IsVisible());
		b.Add("Plots", [=] { plots_pane.Show(!plots_pane.IsVisible()); }).Key(K_CTRL|K_SHIFT|K_G).Check(plots_pane.IsVisible());
		b.Separator();
		b.Add("Files", [=] { files_pane.Show(!files_pane.IsVisible()); }).Key(K_CTRL|K_SHIFT|K_X).Check(files_pane.IsVisible());
		b.Add("Outline", [=] { outline_pane.Show(!outline_pane.IsVisible()); }).Key(K_CTRL|K_SHIFT|K_O).Check(outline_pane.IsVisible());
		b.Add("Project", [=] { Todo("Project pane"); }).Key(K_CTRL|K_SHIFT|K_P).Check(false);
		b.Add("Find", [=] { find_pane.Show(!find_pane.IsVisible()); }).Key(K_CTRL|K_SHIFT|K_F).Check(find_pane.IsVisible());
		b.Separator();
		b.Add("History", [=] { history_pane.Show(!history_pane.IsVisible()); }).Key(K_CTRL|K_SHIFT|K_L).Check(history_pane.IsVisible());
		b.Add("Profiler", [=] { profiler_pane.Show(!profiler_pane.IsVisible()); }).Key(K_CTRL|K_SHIFT|K_R).Check(profiler_pane.IsVisible());
		b.Add("Code Analysis", [=] { Todo("Analysis pane"); }).Key(K_CTRL|K_SHIFT|K_C).Check(false);
		b.Separator();
		b.Add("Online help", [=] { Todo("Online help"); }).Check(false);
		b.Add("Internal console", [=] { Todo("Internal console"); }).Check(false);
	});
	bar.Add("Unlock panes and toolbars", [=] { Todo("Unlock layout"); }).Key(K_CTRL|K_SHIFT|K_F5);
	bar.Add("Maximize current pane", [=] { Todo("Maximize pane"); }).Key(K_CTRL|K_ALT|K_SHIFT|K_M);
	bar.Add("Close current pane", [=] { Todo("Close pane"); }).Key(K_CTRL|K_SHIFT|K_F4);
	bar.Separator();
	bar.Sub("Toolbars", [=](Bar& b) {
		b.Add("File toolbar", [=] { Todo("Toggle File TB"); }).Check(true);
		b.Add("Run toolbar", [=] { Todo("Toggle Run TB"); }).Check(true);
		b.Add("Debug toolbar", [=] { Todo("Toggle Debug TB"); }).Check(true);
		b.Add("Profile toolbar", [=] { Todo("Toggle Profile TB"); }).Check(true);
		b.Add("Main toolbar", [=] { Todo("Toggle Main TB"); }).Check(true);
		b.Add("Current working directory", [=] { Todo("Toggle CWD TB"); }).Check(true);
	});
	bar.Add("Hide toolbars", [=] { Todo("Hide TB"); });
	bar.Separator();
	bar.Sub("Window layouts", [=](Bar& b) {
		b.Add("Default layout", [=] { Todo("Layout Default"); });
		b.Add("Rstudio layout", [=] { Todo("Layout Rstudio"); });
		b.Add("Matlab layout", [=] { Todo("Layout Matlab"); });
		b.Add("Horizontal split", [=] { Todo("Layout Horz"); });
		b.Add("Vertical split", [=] { Todo("Layout Vert"); });
		b.Separator();
		b.Add("Save current layout", [=] { Todo("Save layout"); });
		b.Add("Layout preferences", [=] { Todo("Layout prefs"); });
		b.Add("Reset to Spyder default", [=] { Todo("Reset layout"); });
	});
	bar.Add("Use next layout", [=] { Todo("Next layout"); }).Key(K_ALT|K_SHIFT|K_PAGEUP);
	bar.Add("Use previous layout", [=] { Todo("Prev layout"); }).Key(K_ALT|K_SHIFT|K_PAGEDOWN);
	bar.Separator();
	bar.Add("Fullscreen mode", [=] { Todo("Fullscreen"); }).Key(K_F11);
}

void PythonIDE::HelpMenu(Bar& bar)
{
	bar.Add("Interactive tour", [=] { Todo("Tour"); });
	bar.Add("Built-in tutorial", [=] { Todo("Tutorial"); });
	bar.Add("Shortcuts summary", [=] { Todo("Shortcuts"); });
	bar.Separator();
	bar.Add("Spyder documentation", [=] { Todo("Doc"); }).Key(K_F1);
	bar.Add("Tutorial videos", [=] { Todo("Videos"); });
	bar.Sub("IPython documentation", [=](Bar& b) {
		b.Add("Intro to IPython", [=] { Todo("IPython Intro"); });
		b.Add("Console help", [=] { Todo("Console help"); });
		b.Add("Quick reference", [=] { Todo("Quick ref"); });
	});
	bar.Add("Troubleshooting guide", [=] { Todo("Trouble"); });
	bar.Add("Spyder Google group", [=] { Todo("Google group"); });
	bar.Add("Dependency status", [=] { Todo("Deps"); });
	bar.Add("Report issue...", [=] { Todo("Report"); });
	bar.Separator();
	bar.Add("Check for updates", [=] { Todo("Updates"); });
	bar.Add("Help Spyder...", [=] { Todo("Help Spyder"); });
	bar.Add("About ScriptIDE", [=] { PromptOK("ScriptIDE\n\nA Spyder-like Python IDE using ByteVM and U++."); });
}

void PythonIDE::Todo(const String& msg)
{
	PromptOK("Feature not yet implemented: " + msg);
}

void PythonIDE::OnNewFile()
{
	if(!ConfirmSave()) return;

	code_editor.Clear();
	current_file.path = "";
	current_file.dirty = false;
	editor_tabs.Clear();
	editor_tabs.AddFile("<untitled>", CtrlImg::File());
}

void PythonIDE::OnOpenFile()
{
	if(!ConfirmSave()) return;

	FileSel fs;
	fs.Type("Python files", "*.py");
	if(fs.ExecuteOpen("Open Python File")) {
		LoadFile(fs.Get());
	}
}

void PythonIDE::LoadFile(const String& path)
{
	String content = Upp::LoadFile(path);
	code_editor.Set(content);
	current_file.path = path;
	current_file.dirty = false;
	
	outline_pane.UpdateOutline(content);
	OnAnalyze();

	editor_tabs.Clear();
	editor_tabs.AddFile(path.ToWString(), CtrlImg::File());
}

void PythonIDE::OnSaveFile()
{
	if(current_file.path.IsEmpty()) {
		OnSaveFileAs();
	}
	else {
		SaveFile(current_file.path);
	}
}

void PythonIDE::OnSaveFileAs()
{
	FileSel fs;
	fs.Type("Python files", "*.py");
	if(fs.ExecuteSaveAs("Save Python File As")) {
		SaveFile(fs.Get());
	}
}

void PythonIDE::SaveFile(const String& path)
{
	if(Upp::SaveFile(path, code_editor.Get())) {
		current_file.path = path;
		current_file.dirty = false;
		// Update tab title
		editor_tabs.Set(0, path.ToWString(), GetFileName(path).ToWString());
	}
	else {
		Exclamation("Failed to save file: " + path);
	}
}

bool PythonIDE::ConfirmSave()
{
	if(!current_file.dirty) return true;

	int res = Prompt("Save", CtrlImg::question(), "Save changes to " + (current_file.path.IsEmpty() ? String("untitled") : current_file.path) + "?",
	                 "Save", "Don't Save", "Cancel");

	if(res == 1) {
		OnSaveFile();
		return !current_file.dirty;
	}
	if(res == 0) return true;
	return false;
}

void PythonIDE::OnRun()
{
	String code = code_editor.Get();
	if(code.IsEmpty()) return;

	console_pane.Clear();
	console_pane.Write("--- Running script ---\n");

	String filename = current_file.path.IsEmpty() ? String("<editor>") : current_file.path;
	run_manager.Run(code, filename);
}

void PythonIDE::OnRunSelection()
{
	String code;
	if(code_editor.IsSelection())
		code = code_editor.GetSelection();
	else
		code = code_editor.GetWLine(code_editor.GetCursorLine()).ToString();

	if(code.IsEmpty()) return;

	console_pane.Write("--- Running selection ---\n");
	run_manager.RunSelection(code);
}
void PythonIDE::OnRunConfig() {}
void PythonIDE::OnDebug() {}

void PythonIDE::OnStop()
{
	run_manager.Stop();
	code_editor.HidePtr();
	console_pane.Write("--- Execution stopped ---\n");
}

void PythonIDE::OnBreakpointHit(const String& file, int line)
{
	console_pane.Write("Breakpoint hit at " + file + ":" + AsString(line) + "\n");
	code_editor.SetCursor(code_editor.GetPos(line - 1));
	code_editor.SetPtr(line - 1, CtrlImg::right_arrow(), 0);
	debugger_pane.SetStack(vm.GetCallStack());
	UpdateVariableExplorer();
	code_editor.SetFocus();
}

void PythonIDE::OnStepOver()
{
	if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
		vm.StepOver();
		while(vm.Step() && vm.GetDebugState() != PyVM::DEBUG_PAUSED);
		if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
			OnBreakpointHit(vm.GetCurrentFile(), vm.GetCurrentLine());
		}
	}
}

void PythonIDE::OnStepIn()
{
	if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
		vm.StepIn();
		while(vm.Step() && vm.GetDebugState() != PyVM::DEBUG_PAUSED);
		if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
			OnBreakpointHit(vm.GetCurrentFile(), vm.GetCurrentLine());
		}
	}
}

void PythonIDE::OnStepOut()
{
	if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
		vm.StepOut();
		while(vm.Step() && vm.GetDebugState() != PyVM::DEBUG_PAUSED);
		if(vm.GetDebugState() == PyVM::DEBUG_PAUSED) {
			OnBreakpointHit(vm.GetCurrentFile(), vm.GetCurrentLine());
		}
	}
}


void PythonIDE::OnToggleBreakpoint()
{
	int line = code_editor.GetCursorLine();
	String filename = current_file.path.IsEmpty() ? String("<editor>") : current_file.path;

	if(!code_editor.GetBreakpoint(line).IsEmpty()) {
		code_editor.SetBreakpoint(line, String());
		vm.RemoveBreakpoint(filename, line + 1);
	}
	else {
		code_editor.SetBreakpoint(line, "1"); // "1" means active breakpoint
		vm.AddBreakpoint(filename, line + 1);
	}
}

void PythonIDE::UpdateStatusBar()
{
	// Update from current editor
	if(CodeEditor* ed = GetCurrentEditor()) {
		Point pos = ed->GetColumnLine(ed->GetCursor());
		status_info.line = pos.y + 1;
		status_info.column = pos.x + 1;
		status_info.edit_mode = ed->IsReadOnly() ? "RO" : "RW";
	}

	// Calculate memory usage %
	size_t mem_used = MemoryUsedKb();
	size_t mem_total = MemoryTotalKb();
	if(mem_total > 0)
		status_info.memory_percent = (int)((mem_used * 100) / mem_total);

	// Build status text
	String status = Format(
	    "Line: %d  Col: %d     %s     %s     %s     Mem: %d%%",
	    status_info.line,
	    status_info.column,
	    status_info.format,
	    status_info.line_ending,
	    status_info.edit_mode,
	    status_info.memory_percent
	);

	statusbar.Set(status);
}

size_t PythonIDE::MemoryUsedKb()
{
#ifdef PLATFORM_WIN32
	PROCESS_MEMORY_COUNTERS pmc;
	if(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
		return pmc.WorkingSetSize / 1024;
#elif defined(PLATFORM_POSIX)
	// Read /proc/self/status
	String status = Upp::LoadFile("/proc/self/status");
	Vector<String> lines = Split(status, '\n');
	for(const String& line : lines) {
		if(line.StartsWith("VmRSS:")) {
			int kb = ScanInt(line.Mid(6));
			return kb;
		}
	}
#endif
	return 0;
}

size_t PythonIDE::MemoryTotalKb()
{
#ifdef PLATFORM_WIN32
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	if(GlobalMemoryStatusEx(&statex))
		return statex.ullTotalPhys / 1024;
#elif defined(PLATFORM_POSIX)
	// Read /proc/meminfo
	String meminfo = Upp::LoadFile("/proc/meminfo");
	Vector<String> lines = Split(meminfo, '\n');
	for(const String& line : lines) {
		if(line.StartsWith("MemTotal:")) {
			return ScanInt(line.Mid(9));
		}
	}
#endif
	return 0;
}

void PythonIDE::DockInit()
{
	// Register all panes
	Register(files_pane.SizeHint(Size(250, 400)));
	Register(outline_pane.SizeHint(Size(250, 400)));
	Register(var_explorer.SizeHint(Size(350, 400)));
	Register(debugger_pane.SizeHint(Size(350, 400)));
	Register(profiler_pane.SizeHint(Size(350, 400)));
	Register(help_pane.Title("Help").SizeHint(Size(350, 400)));
	Register(plots_pane.SizeHint(Size(350, 400)));
	Register(console_pane.SizeHint(Size(600, 350)));
	Register(history_pane.Title("History").SizeHint(Size(600, 350)));
	Register(find_pane.SizeHint(Size(600, 350)));

	// Initial Default Docking (only if no saved layout)
	if(!FileExists(ConfigFile("docking-layout.bin"))) {
		DockLeft(files_pane);
		Tabify(files_pane, outline_pane);

		DockRight(var_explorer);
		Tabify(var_explorer, debugger_pane);
		Tabify(var_explorer, profiler_pane);
		Tabify(var_explorer, help_pane);
		Tabify(var_explorer, plots_pane);

		DockBottom(console_pane); // Simple DockBottom
		Tabify(console_pane, history_pane);
		Tabify(console_pane, find_pane);
	}

	files_pane.SetRoot(GetCurrentDirectory());
	find_pane.SetRoot(GetCurrentDirectory());

	// Try to load saved layout
	FileIn in(ConfigFile("docking-layout.bin"));
	if(in.IsOpen() && !in.IsError())
		SerializeWindow(in);
		
	// Populate Toolbar
	toolbar.Set([=](Bar& bar) { MainToolbar(bar); });
}

void PythonIDE::MainToolbar(Bar& bar)
{
	bar.Add(CtrlImg::new_doc(), [=] { OnNewFile(); }).Help("New File");
	bar.Add(CtrlImg::open(), [=] { OnOpenFile(); }).Help("Open File");
	bar.Add(CtrlImg::save(), [=] { OnSaveFile(); }).Help("Save File");
	bar.Separator();
	bar.Add(CtrlImg::right_arrow(), [=] { OnRun(); }).Help("Run Script (F5)");
	bar.Add(CtrlImg::plus(), [=] { OnRunSelection(); }).Help("Run Selection (F9)");
	bar.Separator();
	bar.Add(CtrlImg::remove(), [=] { OnStop(); }).Help("Stop Execution");
}

void PythonIDE::Close()
{
	// Save settings
	StoreToFile(settings, ConfigFile("ide_settings.bin"));

	// Save layout before closing
	FileOut out(ConfigFile("docking-layout.bin"));
	if(out.IsOpen())
		SerializeWindow(out);

	TopWindow::Close();
}

void PythonIDE::ShowHelp(const String& topic)
{
	String qtf;
	qtf << "[_^https://docs.python.org/3/search.html?q=" << topic << "^ Search Python Docs for: " << topic << "]";
	help_pane->SetQTF(qtf);
	help_pane.Show(); 
}

void PythonIDE::OnPathManager()
{
	PathManagerDlg dlg;
	dlg.Set(path_manager);
	if(dlg.Execute() == IDOK) {
		dlg.Get(path_manager);
		path_manager.SyncToVM(vm);
		StoreToFile(path_manager, ConfigFile("pythonpath.bin"));
	}
}

void PythonIDE::ApplySettings()
{
	// Map IDESettings to CodeEditor
	int face = Font::FindFaceNameIndex(settings.appearance.monospace_font_face);
	if(face < 0) face = Font::COURIER;
	code_editor.SetFont(Font(face, settings.appearance.monospace_font_size));
	code_editor.LineNumbers(settings.editor.show_line_numbers);
	code_editor.ShowSpaces(settings.editor.show_spaces);
}

}
