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

	code_editor.HidePtr();

	try {
		Tokenizer tk;
		tk.SkipComments();
		tk.SkipPythonComments();
		if(!tk.Process(cmd, "<stdin>")) return;
		tk.NewlineToEndStatement();
		tk.CombineTokens();

		PyCompiler compiler(tk.GetTokens(), "<stdin>");
		Vector<PyIR> ir;
		compiler.Compile(ir);

		vm.SetIR(ir);
		PyValue res = vm.Run();
		if(!res.IsNone())
			console_pane.Write(res.Repr() + "\n");
		
		debugger_pane.Clear();
		UpdateVariableExplorer();
	}
	catch (Exc& e) {
		console_pane.WriteError(e + "\n");
	}
}

PythonIDE::PythonIDE()
{
    Title("ScriptIDE - Python IDE");
    Sizeable().Zoomable().CenterScreen();
    SetRect(0, 0, 1400, 900);

    AddFrame(menubar);
    AddFrame(toolbar);
    AddFrame(statusbar);

    // Load settings
    LoadFromFile(settings, ConfigFile("settings.bin"));

    InitLayout();

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

    vm.WhenPrint = [=](const String& s) { console_pane.Write(s); };
    vm.WhenPlot = [=](const Image& img) { plots_pane.AddPlot(img); };
    vm.WhenBreakpointHit = [=](const String& file, int line) { OnBreakpointHit(file, line); };

    code_editor.WhenAction = [=] { current_file.dirty = true; };

    SetTimeCallback(-500, [=] { UpdateStatusBar(); });
    UpdateStatusBar();
}

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
	bar.Add("New File", CtrlImg::new_doc(), [=] { OnNewFile(); })
	    .Key(K_CTRL_N);
	bar.Add("Open...", CtrlImg::open(), [=] { OnOpenFile(); })
	    .Key(K_CTRL_O);
	bar.Separator();
	bar.Add("Save", CtrlImg::save(), [=] { OnSaveFile(); })
	    .Key(K_CTRL_S);
	bar.Add("Save As...", CtrlImg::save_as(), [=] { OnSaveFileAs(); })
	    .Key(K_CTRL | K_SHIFT | K_S);
	bar.Separator();
	bar.Add("Exit", [=] { Close(); })
	    .Key(K_ALT_F4);
}

void PythonIDE::EditMenu(Bar& bar) {}
void PythonIDE::SearchMenu(Bar& bar) {}
void PythonIDE::SourceMenu(Bar& bar) {}

void PythonIDE::RunMenu(Bar& bar)
{
	bar.Add("Run", CtrlImg::right_arrow(), [=] { OnRun(); })
	    .Key(K_F5);
	bar.Add("Run Selection", [=] { OnRunSelection(); })
	    .Key(K_F9);
	bar.Separator();
	bar.Add("Configuration per file...", [=] { OnRunConfig(); });
}

void PythonIDE::DebugMenu(Bar& bar)
{
	bar.Add("Debug", [=] { OnDebug(); })
	    .Key(K_CTRL_F5);
	bar.Separator();
	bar.Add("Step Over", [=] { OnStepOver(); })
	    .Key(K_F10);
	bar.Add("Step Into", [=] { OnStepIn(); })
	    .Key(K_F11);
	bar.Add("Step Out", [=] { OnStepOut(); })
	    .Key(K_SHIFT_F11);
	bar.Separator();
	bar.Add("Toggle Breakpoint", [=] { OnToggleBreakpoint(); })
	    .Key(K_F9);
}

void PythonIDE::ConsolesMenu(Bar& bar) {}
void PythonIDE::ProjectsMenu(Bar& bar) {}
void PythonIDE::ToolsMenu(Bar& bar)
{
	bar.Add("Settings", [=] { OnSettings(); });
}

void PythonIDE::ViewMenu(Bar& bar)
{
}

void PythonIDE::HelpMenu(Bar& bar) {}

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

	code_editor.HidePtr();

	console_pane.Clear();
	console_pane.Write("--- Running script ---\n");
	profiler_pane.Clear();

	try {
		Tokenizer tk;
		tk.SkipComments();
		tk.SkipPythonComments();
		String filename = current_file.path.IsEmpty() ? String("<editor>") : current_file.path;
		if(!tk.Process(code, filename)) return;
		tk.NewlineToEndStatement();
		tk.CombineTokens();

		PyCompiler compiler(tk.GetTokens(), filename);
		Vector<PyIR> ir;
		compiler.Compile(ir);

		vm.SetIR(ir);
		vm.Run();

		console_pane.Write("--- Script finished ---\n");
		debugger_pane.Clear();
		UpdateVariableExplorer();
	}
	catch (Exc& e) {
		console_pane.WriteError("Runtime error: " + e + "\n");
	}
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

	try {
		Tokenizer tk;
		tk.SkipComments();
		tk.SkipPythonComments();
		if(!tk.Process(code, "<selection>")) return;
		tk.NewlineToEndStatement();
		tk.CombineTokens();

		PyCompiler compiler(tk.GetTokens(), "<selection>");
		Vector<PyIR> ir;
		compiler.Compile(ir);

		vm.SetIR(ir);
		vm.Run();
		
		debugger_pane.Clear();
		UpdateVariableExplorer();
	}
	catch (Exc& e) {
		console_pane.WriteError("Runtime error: " + e + "\n");
	}
}
void PythonIDE::OnRunConfig() {}
void PythonIDE::OnDebug() {}

void PythonIDE::OnStop()
{
	vm.Reset();
	debugger_pane.Clear();
	UpdateVariableExplorer();
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
	// Register and dock panes
	Register(files_pane.SizeHint(Size(250, 400)));
	Register(outline_pane.SizeHint(Size(250, 400)));
	DockLeft(files_pane);
	Tabify(files_pane, outline_pane);

	files_pane.SetRoot(GetCurrentDirectory());
	find_pane.SetRoot(GetCurrentDirectory());

	Register(var_explorer.SizeHint(Size(300, 400)));
	Register(debugger_pane.SizeHint(Size(300, 400)));
	Register(profiler_pane.SizeHint(Size(300, 400)));
	Register(help_pane.Title("Help").SizeHint(Size(300, 400)));
	Register(plots_pane.SizeHint(Size(300, 400)));
	Register(console_pane.SizeHint(Size(600, 300)));
	Register(history_pane.Title("History").SizeHint(Size(600, 300)));
	Register(find_pane.SizeHint(Size(600, 300)));

	// Dock Top-Right Stack (tabbed)
	DockRight(var_explorer);
	Tabify(var_explorer, debugger_pane);
	Tabify(var_explorer, profiler_pane);
	Tabify(var_explorer, help_pane);
	Tabify(var_explorer, plots_pane);

	// Dock Bottom Stack (tabbed)
	DockBottom(console_pane);
	Tabify(console_pane, history_pane);
	Tabify(console_pane, find_pane);

	// Try to load saved layout
	FileIn in(ConfigFile("docking-layout.bin"));
	if(in.IsOpen() && !in.IsError())
		SerializeWindow(in);
}

void PythonIDE::Close()
{
	// Save settings
	StoreToFile(settings, ConfigFile("settings.bin"));

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

void PythonIDE::ApplySettings()
{
	code_editor.SetFont(settings.editor_font);
	code_editor.LineNumbers(settings.show_line_numbers);
	code_editor.ShowSpaces(settings.show_spaces);
}

void PythonIDE::OnSettings()
{
	SettingsDlg dlg;
	dlg.Set(settings);
	if(dlg.Execute() == IDOK) {
		dlg.Get(settings);
		ApplySettings();
	}
}

}
