#include "ScriptIDE.h"

NAMESPACE_UPP

#define IMAGECLASS ScriptIDEImg
#define IMAGEFILE <ScriptIDE/ScriptIDE.iml>
#include <Draw/iml_source.h>

PythonIDE::PythonIDE()
	: run_manager(vm)
{
	main_window = this;
	plugin_manager.Create(*this);
	Title("Python IDE");
	Icon(Icons::Python());

	Sizeable().Zoomable();
	SetRect(0, 0, 1024, 768);

	InitLayout();

	LoadFromFile(settings, ConfigFile("ide_settings.bin"));
	
	files_pane->SetRoot(GetCurrentDirectory());
	
	plugin_manager->LoadPlugins();
	
	ApplySettings();
	SetTimeCallback(-200, [this] { UpdateStatusBar(); }); // DONT CHANGE THIS
}

PythonIDE::~PythonIDE()
{
}

void PythonIDE::ApplySettings()
{
	// Editor settings
	for(int i = 0; i < open_files.GetCount(); i++) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[i].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
				ed->LineNumbers(settings.editor.show_line_numbers);
				ed->ShowTabs(settings.editor.show_spaces);
				ed->ShowSpaces(settings.editor.show_spaces);
				ed->TabSize(settings.editor.tab_width);
				
				Font f = Courier(settings.appearance.monospace_font_size);
				f.FaceName(settings.appearance.monospace_font_face);
				ed->SetFont(f);
			}
		}
	}
	
	// Files pane settings
	files_pane->ShowHidden(settings.files.show_hidden_files);
	
	// Status bar settings
	// (handled in UpdateStatusBar via settings poll)
}

void PythonIDE::InitLayout()
{
	AddFrame(menubar);
	AddFrame(toolbar);
	AddFrame(statusbar);
	
	toolbar.MaxIconSize(Size(24, 24)); // Set icons to 24x24
	
	menubar.Set([this](Bar& bar) { MainMenu(bar); });
	toolbar.Set([this](Bar& bar) { MainToolbar(bar); });

	Add(editor_area.SizePos());
	
	editor_tabs.Create();
	editor_area.Add(editor_tabs->TopPos(0, 24).HSizePos());
	
	editor_tabs->WhenAction = [=] { OnTabChanged(); };
	editor_tabs->WhenTabMenu = [=](Bar& bar) { OnTabMenu(bar); };
	
	console_pane.Create();
	console_pane->WhenInput = [=] { OnConsoleInput(); };
	console_pane->WhenInterrupt = [=] { run_manager.Stop(); };
	console_pane->WhenRestart = [=] { 
		if(PromptYesNo("Restart kernel? This will clear all variables.")) {
			vm.Reset();
			console_pane->Clear();
			console_pane->Write("Kernel restarted.\n");
			UpdateVariableExplorer();
		}
	};
	console_pane->WhenRemoveVariables = [=] { 
		if(PromptYesNo("Remove all variables from current session?")) {
			vm.GetGlobals().Clear();
			var_explorer->Clear();
		}
	};
	
	files_pane.Create();
	files_pane->WhenOpen = [=](const String& path) { LoadFile(path); };
	files_pane->WhenPathManager = [=] { OnPathManager(); };
	files_pane->WhenBrowse = [=] {
		FileSel fs;
		if(fs.ExecuteSelectDir("Select Working Directory"))
			files_pane->SetRoot(fs.Get());
	};
	files_pane->WhenParent = [=] {
		String p = GetFileDirectory(files_pane->GetRoot());
		if(p.EndsWith(String(DIR_SEP, 1))) p.TrimLast();
		p = GetFileDirectory(p);
		if(!p.IsEmpty()) files_pane->SetRoot(p);
	};
	
	var_explorer.Create();
	var_explorer->WhenRemoveAll = [=] {
		if(PromptYesNo("Remove all variables from current session?")) {
			vm.GetGlobals().Clear();
			var_explorer->Clear();
		}
	};
	var_explorer->WhenRefresh = [=] { UpdateVariableExplorer(); };
	
	plots_pane.Create();
	debugger_pane.Create();
	debugger_pane->WhenStepOver = [=] { vm.StepOver(); };
	debugger_pane->WhenStepInto = [=] { vm.StepIn(); };
	debugger_pane->WhenStepOut = [=] { vm.StepOut(); };
	debugger_pane->WhenContinue = [=] { vm.Run(); };
	debugger_pane->WhenStop = [=] { run_manager.Stop(); };

	profiler_pane.Create();
	find_pane.Create();
	find_pane->WhenOpenMatch = [=](const String& path, int line) {
		LoadFile(path);
		if(active_file >= 0 && open_files[active_file].editor) {
			if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
				if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
					ed->SetCursor(ed->GetPos(line - 1));
				}
			}
		}
	};

	outline_pane.Create();
	outline_pane->WhenSelectLine = [=](int line) {
		if(active_file >= 0 && open_files[active_file].editor) {
			if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
				if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
					ed->SetCursor(ed->GetPos(line - 1));
				}
			}
		}
	};

	help_pane.Create();
	history_pane.Create();
	
	// Track active pane
	files_pane->WhenAction = [this] { active_pane = &*files_pane; };
	outline_pane->WhenAction = [this] { active_pane = &*outline_pane; };
	var_explorer->WhenAction = [this] { active_pane = &*var_explorer; };
	help_pane->WhenAction = [this] { active_pane = &*help_pane; };
	plots_pane->WhenAction = [this] { active_pane = &*plots_pane; };
	console_pane->WhenAction = [this] { active_pane = &*console_pane; };
	history_pane->WhenAction = [this] { active_pane = &*history_pane; };
	find_pane->WhenAction = [this] { active_pane = &*find_pane; };
	debugger_pane->WhenAction = [this] { active_pane = &*debugger_pane; };
	profiler_pane->WhenAction = [this] { active_pane = &*profiler_pane; };
}

void PythonIDE::RegisterPanes()
{
	Register(*files_pane);
	Register(*outline_pane);
	Register(*var_explorer);
	Register(*help_pane);
	Register(*plots_pane);
	Register(*console_pane);
	Register(*history_pane);
	Register(*find_pane);
	Register(*debugger_pane);
	Register(*profiler_pane);
	Register(context_pane_left);
	Register(context_pane_right);
}

void PythonIDE::DockInit()
{
	// Register all panes so SerializeWindow can find them by index
	RegisterPanes();

	// Try to restore previous session
	String session_file = ConfigFile("ide_session.json");
	bool loaded = false;
	if(FileExists(session_file)) {
		String json = ::Upp::LoadFile(session_file);
		if(!json.IsEmpty()) {
			Value v = ParseJSON(json);
			if(!v.IsError()) {
				JsonIO jio(v);
				JsonizeWindow(jio);
				loaded = true;
			}
		}
	}

	if(!loaded) {
		// Set size hints BEFORE docking
		files_pane->SizeHint(Size(250, 400));
		outline_pane->SizeHint(Size(250, 400));

		var_explorer->SizeHint(Size(300, 300));
		help_pane->SizeHint(Size(300, 300));
		plots_pane->SizeHint(Size(300, 300));
		debugger_pane->SizeHint(Size(300, 300));
		profiler_pane->SizeHint(Size(300, 300));

		console_pane->SizeHint(Size(400, 250));
		history_pane->SizeHint(Size(400, 250));
		find_pane->SizeHint(Size(400, 250));

		// Perform default docking layout
		DockLeft(*files_pane);
		Tabify(*files_pane, *outline_pane);

		DockRight(*var_explorer);
		Tabify(*var_explorer, *help_pane);
		Tabify(*var_explorer, *plots_pane);

		DockRight(*debugger_pane);
		Tabify(*debugger_pane, *profiler_pane);

		DockBottom(*console_pane);
		DockBottom(*history_pane);
		Tabify(*console_pane, *find_pane);

		DockLeft(context_pane_left);
		DockRight(context_pane_right);

		context_pane_left.Title("Context (L)").Hide();
		context_pane_right.Title("Context (R)").Hide();

		// Set frame order: Left/Right take precedence (full height)
		SetFrameOrder(DOCK_LEFT, DOCK_RIGHT, DOCK_BOTTOM, DOCK_TOP);

		SetFrameSize(DOCK_LEFT, 250);
		SetFrameLayoutHalf(DOCK_RIGHT);
		SetFrameSize(DOCK_BOTTOM, 250);
	}

	// Capture the default layout string
	StringStream s;
	SerializeLayout(s);
	default_layout = s;
}

void PythonIDE::Close()
{
	if(ConfirmSaveAll()) {
		Value v;
		JsonIO jio(v);
		JsonizeWindow(jio);
		::Upp::SaveFile(ConfigFile("ide_session.json"), AsJSON(v, true));
		StoreToFile(settings, ConfigFile("ide_settings.bin"));
		TopWindow::Close();
	}
}

void PythonIDE::ShowHelp(const String& topic)
{
	String qtf;
	qtf << "[_^https://docs.python.org/3/search.html?q=" << topic << "^ Search Python Docs for: " << topic << "]";
	help_pane->SetQTF(qtf);
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
	if(path.IsEmpty()) {
		OnSaveFileAs();
		path = open_files[active_file].path;
		if(path.IsEmpty()) return;
	}
	
	last_run_path = path;
	
	ICustomExecuteProvider* provider = plugin_manager->FindCustomExecuteProvider(path);
	if(provider) {
		provider->Execute(path);
		return;
	}
	
	String content;
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl()))
				content = ed->Get();
		}
	}
	
	plugin_manager->SyncBindings(vm);
	run_manager.Run(content, path);
}

void PythonIDE::OnRunLast()
{
	if(last_run_path.IsEmpty()) return;
	
	// Load the file if it's not open
	bool found = false;
	for(int i = 0; i < open_files.GetCount(); i++) {
		if(open_files[i].path == last_run_path) {
			active_file = i;
			editor_tabs->SetCursor(i);
			found = true;
			break;
		}
	}
	
	if(!found) {
		LoadFile(last_run_path);
	}
	
	OnRun();
}

void PythonIDE::OnRunSelection()
{
	if(active_file < 0) return;
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl()))
				run_manager.RunSelection(ed->GetSelection());
		}
	}
}

void PythonIDE::OnRunCell()
{
	if(active_file < 0) return;
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl()))
				run_manager.RunSelection(ed->GetCurrentCell());
		}
	}
}

void PythonIDE::OnRunCellAndAdvance()
{
	if(active_file < 0) return;
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
				run_manager.RunSelection(ed->GetCurrentCell());
				// Advance cursor to next cell or end
				int line = ed->GetLine(ed->GetCursor());
				for(int i = line + 1; i < ed->GetLineCount(); i++) {
					if(Upp::TrimLeft(ed->GetUtf8Line(i)).StartsWith("# %%")) {
						ed->SetCursor(ed->GetPos(i));
						return;
					}
				}
				ed->SetCursor(ed->GetLength());
			}
		}
	}
}

void PythonIDE::OnRunToLine()
{
	if(active_file < 0) return;
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
				int line = ed->GetLine(ed->GetCursor());
				String code;
				for(int i = 0; i <= line; i++)
					code << ed->GetUtf8Line(i) << "\n";
				run_manager.RunSelection(code);
			}
		}
	}
}

void PythonIDE::OnRunFromLine()
{
	if(active_file < 0) return;
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
				int line = ed->GetLine(ed->GetCursor());
				String code;
				for(int i = line; i < ed->GetLineCount(); i++)
					code << ed->GetUtf8Line(i) << "\n";
				run_manager.RunSelection(code);
			}
		}
	}
}

void PythonIDE::OnRunConfig() { Todo("Run Configuration"); }

void PythonIDE::OnDebug()
{
	// Similar to Run but maybe with some debug flags set in VM
	OnRun();
}

void PythonIDE::OnDebugCell() { OnRunCell(); }
void PythonIDE::OnDebugSelection() { OnRunSelection(); }
void PythonIDE::OnDebugToLine() { OnRunToLine(); }
void PythonIDE::OnStop() { run_manager.Stop(); }
void PythonIDE::OnStepOver() { vm.StepOver(); }
void PythonIDE::OnStepIn() { vm.StepIn(); }
void PythonIDE::OnStepOut() { vm.StepOut(); }

void PythonIDE::OnToggleBreakpoint()
{
	if(active_file < 0) return;
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
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
	}
}

void PythonIDE::OnClearBreakpoints()
{
	if(PromptYesNo("Clear all breakpoints?")) {
		vm.ClearBreakpoints();
		for(int i = 0; i < open_files.GetCount(); i++) {
			if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[i].editor)) {
				h->GetCtrl().Refresh();
			}
		}
	}
}

void PythonIDE::OnListBreakpoints()
{
	Todo("List breakpoints");
}

void PythonIDE::OnConsoleInput()
{
	String input = console_pane->GetInput();
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
			console_pane->Write(res.Repr() + "\n");
			
		UpdateVariableExplorer();
	}
	catch (Exc& e) {
		console_pane->WriteError(String("Error: ") + e + "\n");
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
	editor_tabs->Add("<untitled>", Icons::File());
	editor_tabs->SetCursor(active_file);
	
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
			editor_tabs->SetCursor(i);
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
	fi.editor = host;
	fi.is_plugin = is_plugin;
	
	active_file = open_files.GetCount() - 1;
	editor_tabs->Add(GetFileName(path), Icons::File());
	editor_tabs->SetCursor(active_file);
	
	OnTabChanged();
	AddRecentFile(path);
}

bool PythonIDE::SaveFile(int idx)
{
	if(idx < 0 || idx >= open_files.GetCount()) return false;
	FileInfo& fi = open_files[idx];
	if(fi.path.IsEmpty()) return false;
	
	if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(fi.editor)) {
		if(h->SaveAs(fi.path)) {
			fi.dirty = false;
			SyncTabsWithFiles();
			AddRecentFile(fi.path);
			return true;
		}
		else {
			Exclamation("Failed to save file: " + fi.path);
		}
	}
	return false;
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

void PythonIDE::OnSaveAll()
{
	int old_active = active_file;
	for(int i = 0; i < open_files.GetCount(); i++) {
		active_file = i;
		OnSaveFile();
	}
	active_file = old_active;
}

void PythonIDE::OnSaveCopyAs()
{
	if(active_file < 0) return;
	FileSel fs;
	fs.Type("Python files", "*.py");
	if(fs.ExecuteSaveAs("Save Copy As")) {
		String content;
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl()))
				content = ed->Get();
		}
		if(!content.IsEmpty())
			::Upp::SaveFile(fs.Get(), content);
	}
}

void PythonIDE::OnRevert()
{
	if(active_file < 0) return;
	FileInfo& fi = open_files[active_file];
	if(fi.path.IsEmpty()) return;
	
	if(PromptYesNo("Discard unsaved changes and reload " + GetFileName(fi.path) + "?")) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(fi.editor)) {
			h->Load(fi.path);
			fi.dirty = false;
			SyncTabsWithFiles();
		}
	}
}

void PythonIDE::OnTabMenu(Bar& bar)
{
	int idx = editor_tabs->GetCursor();
	if(idx < 0) return;
	
	bar.Add("Close", [=] { OnCloseFile(); });
	bar.Add("Close others", [=] {
		for(int i = open_files.GetCount() - 1; i >= 0; i--) {
			if(i != idx) {
				active_file = i;
				OnCloseFile();
			}
		}
	});
	bar.Add("Close all", [=] { OnCloseAll(); });
	bar.Separator();
	bar.Add("Copy path", [=] { WriteClipboardText(open_files[idx].path); });
	bar.Add("Open in explorer", [=] { LaunchWebBrowser(GetFileDirectory(open_files[idx].path)); });
}

void PythonIDE::SyncTabsWithFiles()
{
	for(int i = 0; i < open_files.GetCount(); i++) {
		FileInfo& fi = open_files[i];
		String title = fi.path.IsEmpty() ? "<untitled>" : GetFileName(fi.path);
		
		bool dirty = fi.dirty;
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(fi.editor))
			dirty = h->IsModified();
			
		if(dirty) title = "*" + title;
		
		editor_tabs->Set(i, i, title);
	}
}

void PythonIDE::AddRecentFile(const String& path)
{
	if(path.IsEmpty()) return;
	Vector<String>& rf = settings.application.recent_files;
	for(int i = 0; i < rf.GetCount(); i++) {
		if(rf[i] == path) {
			rf.Remove(i);
			break;
		}
	}
	rf.Insert(0, path);
	while(rf.GetCount() > 20) rf.Remove(rf.GetCount() - 1);
}

void PythonIDE::UpdateRecentFilesMenu(Bar& bar)
{
	Vector<String>& rf = settings.application.recent_files;
	for(int i = 0; i < rf.GetCount(); i++) {
		String path = rf[i];
		bar.Add(path, [=] { LoadFile(path); });
	}
	if(rf.GetCount() > 0) bar.Separator();
	bar.Add("Clear list", [this] { settings.application.recent_files.Clear(); });
}

void PythonIDE::OnFileSwitcher() { Todo("File switcher"); }
void PythonIDE::OnSymbolFinder() { Todo("Symbol finder"); }
void PythonIDE::OnRestart() { Todo("Restart IDE"); }

void PythonIDE::OnUndo() { 
	if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) h->Undo(); }
}
void PythonIDE::OnRedo() { 
	if(active_editor) { if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) h->Redo(); }
}
void PythonIDE::OnComment() { 
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl()))
				ed->ToggleComments();
		}
	}
}
void PythonIDE::OnBlockComment() { 
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl()))
				ed->ToggleBlockComments();
		}
	}
}
void PythonIDE::OnUncomment() { 
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl()))
				ed->ToggleComments(); 
		}
	}
}
void PythonIDE::OnToggleCase(bool upper) { 
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
				String s = ed->GetSelection();
				if(!s.IsEmpty()) {
					ed->Paste(upper ? ToUpper(s).ToWString() : ToLower(s).ToWString());
				}
			}
		}
	}
}
void PythonIDE::OnConvertEOL(const String& mode)
{
	if(active_file < 0) return;
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
				String s = ed->Get();
				if(mode == "LF") s.Replace("\r\n", "\n");
				else if(mode == "CRLF") { s.Replace("\r\n", "\n"); s.Replace("\n", "\r\n"); }
				ed->Set(s);
			}
		}
	}
}

void PythonIDE::OnRemoveTrailingSpaces()
{
	if(active_file < 0) return;
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
				String s = ed->Get();
				Vector<String> lines = Split(s, '\n', false);
				String res;
				for(int i = 0; i < lines.GetCount(); i++) {
					if(i > 0) res << '\n';
					res << TrimRight(lines[i]);
				}
				if(s.EndsWith("\n")) res << '\n';
				ed->Set(res);
			}
		}
	}
}

void PythonIDE::OnTabsToSpaces()
{
	if(active_file < 0) return;
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
				String s = ed->Get();
				s.Replace("\t", String(' ', settings.editor.tab_width));
				ed->Set(s);
			}
		}
	}
}

bool PythonIDE::ConfirmSave(int idx)
{
	if(idx < 0 || idx >= open_files.GetCount()) return true;
	FileInfo& fi = open_files[idx];
	
	bool dirty = fi.dirty;
	if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(fi.editor))
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
		
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(fi.editor))
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

PythonIDE::FileInfo* PythonIDE::GetActiveFile()
{
	if(active_file >= 0 && active_file < open_files.GetCount())
		return &open_files[active_file];
	return nullptr;
}

void PythonIDE::OnCloseFile()
{
	if(active_file < 0) return;
	if(!ConfirmSave(active_file)) return;
	
	int to_close = active_file;
	open_files.Remove(to_close);
	editor_tabs->Close(to_close);
	
	if(open_files.GetCount() > 0) {
		active_file = editor_tabs->GetCursor();
		OnTabChanged();
	}
	else {
		active_file = -1;
		active_editor = nullptr;
		OnTabChanged();
	}
}

void PythonIDE::OnCloseAll()
{
	if(!ConfirmSaveAll()) return;
	open_files.Clear();
	editor_tabs->Clear();
	OnTabChanged();
}

void PythonIDE::OnOpenLastClosed() { Todo("Open last closed"); }

void PythonIDE::OnBreakpointHit(const String& file, int line)
{
	LoadFile(file);
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
				ed->SetCursor(ed->GetPos(line - 1));
			}
		}
	}
	
	debugger_pane->SetStack(vm.GetCallStack());
	UpdateVariableExplorer();
	
	console_pane->Write(Format("Breakpoint hit at %s:%d\n", file, line));
	Show();
}

void PythonIDE::OnTabChanged()
{
	if(active_editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor))
			h->DeactivateUI();
		active_editor->Hide();
	}
	int idx = editor_tabs->GetCursor();
	if(idx >= 0 && idx < open_files.GetCount()) {
		active_file = idx;
		active_editor = &open_files[active_file].editor->GetCtrl();
		
		if(active_editor) {
			active_editor->Show();
			active_editor->SetFocus();
			if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor))
				h->ActivateUI();
		}
	}
	else {
		active_file = -1;
		active_editor = nullptr;
	}
	
	AddCursorHistory();
	
	menubar.Set([=](Bar& bar) { MainMenu(bar); });
	toolbar.Set([=](Bar& bar) { MainToolbar(bar); });
}

void PythonIDE::OnTogglePane(DockableCtrl& pane)
{
	if(pane.IsDocked())
		Float(pane);
	else
		DockBottom(pane);
}

void PythonIDE::OnLayoutDefault() 
{ 
	StringStream s(default_layout);
	SerializeLayout(s);
}

void PythonIDE::OnLayoutRstudio() 
{ 
	for(DockableCtrl *d : GetDockableCtrls()) d->Hide();
	
	DockLeft(*files_pane);
	DockRight(*plots_pane);
	DockBottom(*console_pane);
	DockRight(*var_explorer);
	Tabify(*var_explorer, *history_pane);
	
	files_pane->Show();
	plots_pane->Show();
	console_pane->Show();
	var_explorer->Show();
}

void PythonIDE::OnLayoutMatlab() 
{ 
	for(DockableCtrl *d : GetDockableCtrls()) d->Hide();
	
	DockLeft(*files_pane);
	DockBottom(*console_pane);
	DockRight(*var_explorer);
	
	files_pane->Show();
	console_pane->Show();
	var_explorer->Show();
}

void PythonIDE::OnFullscreen() 
{ 
	FullScreen(!IsFullScreen()); 
}

void PythonIDE::OnMaximizePane() 
{ 
	if(active_pane) {
		Float(*active_pane);
		active_pane->SetRect(GetSize());
	}
}

void PythonIDE::OnClosePane() 
{ 
	if(active_pane)
		active_pane->Hide();
}

void PythonIDE::UpdateStatusBar()
{
	String text;
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
				Point p = ed->GetColumnLine(ed->GetCursor64());
				text << "Line: " << p.y + 1 << "  Col: " << p.x + 1;
			}
		}
	}
	
	size_t used = MemoryUsedKb();
	if(used > 0)
		text << "    Memory: " << used << " KB";
		
	statusbar.Set(text);
}

void PythonIDE::UpdateVariableExplorer()
{
	var_explorer->SetVariables(vm.GetGlobals());
}

void PythonIDE::OnAnalyze() { Todo("Analyze"); }

void PythonIDE::SyncPluginPanes()
{
}

void PythonIDE::OnPrevCursor()
{
	if(cursor_history_idx > 0) {
		cursor_history_idx--;
		CursorPos& cp = cursor_history[cursor_history_idx];
		LoadFile(cp.path);
		if(active_file >= 0 && open_files[active_file].editor) {
			if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
				if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
					ed->SetCursor(cp.pos);
				}
			}
		}
	}
}

void PythonIDE::OnNextCursor()
{
	if(cursor_history_idx < cursor_history.GetCount() - 1) {
		cursor_history_idx++;
		CursorPos& cp = cursor_history[cursor_history_idx];
		LoadFile(cp.path);
		if(active_file >= 0 && open_files[active_file].editor) {
			if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
				if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
					ed->SetCursor(cp.pos);
				}
			}
		}
	}
}

void PythonIDE::AddCursorHistory()
{
	if(active_file < 0) return;
	if(active_file >= 0 && open_files[active_file].editor) {
		if(IDocumentHost* h = dynamic_cast<IDocumentHost*>(open_files[active_file].editor)) {
			if(PythonEditor* ed = dynamic_cast<PythonEditor*>(&h->GetCtrl())) {
				CursorPos cp;
				cp.path = open_files[active_file].path;
				cp.pos = ed->GetCursor();
				
				if(cursor_history_idx >= 0 && cursor_history_idx < cursor_history.GetCount()) {
					if(cursor_history[cursor_history_idx].path == cp.path &&
					   abs(cursor_history[cursor_history_idx].pos - cp.pos) < 50)
						return;
				}
				
				cursor_history.SetCount(cursor_history_idx + 1);
				cursor_history.Add(cp);
				cursor_history_idx = cursor_history.GetCount() - 1;
				
				while(cursor_history.GetCount() > 50) {
					cursor_history.Remove(0);
					cursor_history_idx--;
				}
			}
		}
	}
}

size_t PythonIDE::MemoryUsedKb() { return 0; }
size_t PythonIDE::MemoryTotalKb() { return 0; }

void PythonIDE::Log(const String& s)
{
	console_pane->Write(s + "\n");
}

void PythonIDE::Error(const String& s)
{
	console_pane->WriteError(s + "\n");
}

void PythonIDE::OnSettings()
{
	PreferencesWindow dlg(*this, settings);
	if(dlg.Run() == IDOK) {
		ApplySettings();
		StoreToFile(settings, ConfigFile("ide_settings.bin"));
	}
}

}
