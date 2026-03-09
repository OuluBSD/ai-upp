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

	InitDocking();
}

void PythonIDE::InitDocking()
{
	var_dock.Title("Variable Explorer").Add(var_explorer.SizePos());
	help_dock.Title("Help").Add(help_viewer.SizePos());
	plots_dock.Title("Plots").Add(plots_viewer.SizePos());
	files_dock.Title("Files Explorer").Add(files_viewer.SizePos());
	console_dock.Title("IPython Console").Add(python_console.SizePos());
	history_dock.Title("History").Add(history_viewer.SizePos());
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
	String cmd = python_console.GetInput();
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
			python_console.Write(res.Repr() + "\n");
		
		UpdateVariableExplorer();
	}
	catch (Exc& e) {
		python_console.WriteError(e + "\n");
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

    // Setup file tree panel (layout done in DockInit)
    file_panel.Add(file_toolbar.TopPos(0, 24).HSizePos());
    file_panel.Add(file_tree.VSizePos(24, 0).HSizePos());
    file_panel.Title("Files");
    file_panel.Icon(CtrlImg::Dir());

    menubar.Set([=](Bar& bar) { MainMenu(bar); });

    file_tree.WhenOpen = [=](const String& path) { if(ConfirmSave()) LoadFile(path); };

    python_console.WhenInput = [=] { OnConsoleInput(); };

    vm.WhenPrint = [=](const String& s) { python_console.Write(s); };
    vm.WhenPlot = [=](const Image& img) { plots_viewer.AddPlot(img); };
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

	python_console.Clear();
	python_console.Write("--- Running script ---\n");

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

		python_console.Write("--- Script finished ---\n");
		UpdateVariableExplorer();
	}
	catch (Exc& e) {
		python_console.WriteError("Runtime error: " + e + "\n");
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

	python_console.Write("--- Running selection ---\n");

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
		
		UpdateVariableExplorer();
	}
	catch (Exc& e) {
		python_console.WriteError("Runtime error: " + e + "\n");
	}
}
void PythonIDE::OnRunConfig() {}
void PythonIDE::OnDebug() {}

void PythonIDE::OnBreakpointHit(const String& file, int line)
{
	python_console.Write("Breakpoint hit at " + file + ":" + AsString(line) + "\n");
	code_editor.SetCursor(code_editor.GetPos(line - 1));
	code_editor.SetPtr(line - 1, CtrlImg::right_arrow(), 0);
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
	// Register and dock the file tree to the left
	Register(file_panel.SizeHint(Size(250, 400)));
	DockLeft(file_panel);

	file_tree.SetRoot(GetCurrentDirectory());

	// Register other panels
	Register(var_dock.SizeHint(Size(300, 400)));
	Register(help_dock.SizeHint(Size(300, 400)));
	Register(plots_dock.SizeHint(Size(300, 400)));
	Register(files_dock.SizeHint(Size(300, 400)));
	Register(console_dock.SizeHint(Size(600, 300)));
	Register(history_dock.SizeHint(Size(600, 300)));

	// Dock Top-Right Stack (tabbed)
	DockRight(var_dock);
	Tabify(var_dock, help_dock);
	Tabify(var_dock, plots_dock);
	Tabify(var_dock, files_dock);

	// Dock Bottom Stack (tabbed)
	DockBottom(console_dock);
	Tabify(console_dock, history_dock);

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
	help_viewer.SetQTF(qtf);
	help_dock.Show(); // Activates docking window if it exists
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
