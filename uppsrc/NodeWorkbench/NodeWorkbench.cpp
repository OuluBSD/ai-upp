#include "NodeWorkbench.h"
#include <Node/Script/Script.h>

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// DomainRegistry
// ---------------------------------------------------------------------------

namespace {
	struct RegEntry : public Moveable<RegEntry> {
		DomainRegistry::Factory factory;
	};
	Vector<RegEntry>& GetRegistry() {
		static Vector<RegEntry> r;
		return r;
	}
}

DomainRegistry::Entry::Entry(Factory f) {
	RegEntry e;
	e.factory = f;
	GetRegistry().Add(pick(e));
}

int DomainRegistry::GetCount() {
	return GetRegistry().GetCount();
}

INodeWorkbenchDomain* DomainRegistry::Create(int i) {
	if(i < 0 || i >= GetRegistry().GetCount()) return nullptr;
	return GetRegistry()[i].factory();
}

INodeWorkbenchDomain* DomainRegistry::CreateById(const String& id) {
	for(auto& e : GetRegistry()) {
		One<INodeWorkbenchDomain> tmp(e.factory());
		if(tmp->GetDomainId() == id)
			return tmp.Detach();
	}
	return nullptr;
}

// ---------------------------------------------------------------------------
// file-local helpers
// ---------------------------------------------------------------------------
namespace {

String MakeTreeKey(const String& kind, const String& path) {
	return kind + "|" + path;
}

bool ParseTreeKey(const Value& value, String& kind, String& path) {
	if(IsNull(value)) return false;
	String key = value;
	int split = key.Find('|');
	if(split < 0) return false;
	kind = key.Left(split);
	path = key.Mid(split + 1);
	return true;
}

bool SkipTreeDirectory(const String& name) {
	String n = ToLower(name);
	return n == "." || n == ".." || n == ".git" || n == ".svn" || n == ".hg" ||
	       n == "build" || n == "bin" || n == "obj" || n == "out" ||
	       n == "__pycache__";
}

void AddDirectoryTree(TreeArrayCtrl& tree, int parent_id,
                      const String& dir, int depth, int& budget) {
	if(depth > 12 || budget <= 0) return;

	Vector<String> dirs, files;
	FindFile ff(AppendFileName(dir, "*"));
	while(ff) {
		String name = ff.GetName();
		String path = NormalizePath(ff.GetPath());
		if(name.GetCount() && name[0] == '.') { ff.Next(); continue; }
		if(ff.IsFolder()) {
			if(!SkipTreeDirectory(name)) dirs.Add(path);
		} else if(ff.IsFile()) {
			String ext = ToLower(GetFileExt(path));
			if(WorkbenchExtensions::IsKnownKind(ext))
				files.Add(path);
		}
		ff.Next();
	}
	Sort(dirs);
	Sort(files);

	for(const String& d : dirs) {
		if(budget-- <= 0) return;
		int id = tree.Add(parent_id, CtrlImg::Dir(),
		                  MakeTreeKey("folder", d), GetFileName(d), true);
		AddDirectoryTree(tree, id, d, depth + 1, budget);
	}
	for(const String& f : files) {
		if(budget-- <= 0) return;
		String kind = WorkbenchExtensions::KindFromPath(f);
		tree.Add(parent_id, CtrlImg::File(),
		         MakeTreeKey(kind, f), GetFileName(f), false);
	}
}

Color DiagColor(DiagSeverity s) {
	switch(s) {
	case DiagSeverity::Error:   return Color(220, 60, 60);
	case DiagSeverity::Warning: return Color(200, 150, 0);
	default:                    return Color(80, 80, 80);
	}
}

String DiagLabel(DiagSeverity s) {
	switch(s) {
	case DiagSeverity::Error:   return "Error";
	case DiagSeverity::Warning: return "Warning";
	default:                    return "Info";
	}
}

} // namespace

// ---------------------------------------------------------------------------
// NodeWorkbenchWindow — constructor
// ---------------------------------------------------------------------------

NodeWorkbenchWindow::NodeWorkbenchWindow() {
	Title("NodeWorkbench");
	Sizeable().Zoomable().MaximizeBox();

	// ---- project panel ----
	btn_open_item.SetLabel("Open");
	btn_open_item << THISBACK(OpenSelectedProjectTreeItem);
	project_panel.Add(btn_open_item.LeftPos(6, 70).BottomPos(6, 24));

	btn_new_graph.SetLabel("New Graph");
	btn_new_graph << THISBACK(ActionNewGraph);
	project_panel.Add(btn_new_graph.LeftPos(82, 84).BottomPos(6, 24));

	btn_new_folder.SetLabel("New Folder");
	project_panel.Add(btn_new_folder.LeftPos(172, 84).BottomPos(6, 24));

	solution_tree.WhenLeftDouble = [=] { OpenSelectedProjectTreeItem(); };
	solution_tree.WhenMenu = THISBACK(OnProjectTreeMenu);
	project_panel.Add(solution_tree.HSizePos(0).VSizePos(0, 36));

	// ---- palette panel ----
	category_list.AddColumn("Category");
	category_list.WhenSel = THISBACK(RefreshNodeList);
	palette_split.Vert();
	palette_split.Add(category_list);

	node_list.AddColumn("Node");
	palette_split.Add(node_list);

	btn_add_node.SetLabel("Add to Graph");
	palette_panel.Add(btn_add_node.LeftPos(6, 110).BottomPos(6, 24));
	palette_panel.Add(palette_split.HSizePos(0).VSizePos(0, 36));

	// ---- diagnostics panel ----
	diagnostics_list.AddColumn("Severity", 80);
	diagnostics_list.AddColumn("Message");
	diagnostics_list.AddColumn("Entity", 120);
	diagnostics_list.WhenSel = [=] {
		btn_diag_fix.Enable(diagnostics_list.IsCursor());
	};
	btn_diag_verify.SetLabel("Verify");
	btn_diag_verify << THISBACK(ValidateGraph);
	diagnostics_panel.Add(btn_diag_verify.LeftPos(6, 60).BottomPos(6, 24));
	btn_diag_clear.SetLabel("Clear");
	btn_diag_clear << [=] { last_diags.Clear(); RefreshDiagnosticsPane(); };
	diagnostics_panel.Add(btn_diag_clear.LeftPos(72, 55).BottomPos(6, 24));
	btn_diag_fix.SetLabel("Quick Fix...");
	btn_diag_fix.Disable();
	btn_diag_fix << THISBACK(ActionDiagQuickFix);
	diagnostics_panel.Add(btn_diag_fix.LeftPos(133, 90).BottomPos(6, 24));
	diagnostics_panel.Add(diagnostics_list.HSizePos(0).VSizePos(0, 36));

	// ---- viewport ----
	dispatcher.RegisterStandardCommands();
	viewport.SetGraph(graph);
	viewport.SetEditor(editor);
	viewport.SetHistory(history);
	viewport.SetDispatcher(dispatcher);
	Add(viewport.HSizePos(0).VSizePos(0, 0));

	// ---- menu / status ----
	AddFrame(menu);
	AddFrame(status);
	menu.Set(THISBACK(MainMenu));

	SetStatus("Ready.");
}

// ---------------------------------------------------------------------------
// RegisterDomain
// ---------------------------------------------------------------------------

void NodeWorkbenchWindow::RegisterDomain(INodeWorkbenchDomain& d) {
	// Track all registered domains (deduplicate by pointer).
	bool found = false;
	for(auto* p : registered_domains)
		if(p == &d) { found = true; break; }
	if(!found)
		registered_domains.Add(&d);

	// First registered domain becomes the active one.
	if(!domain)
		ActivateDomain(d);
}

void NodeWorkbenchWindow::ActivateDomain(INodeWorkbenchDomain& d) {
	domain = &d;
	String name = d.GetDomainName();
	if(!name.IsEmpty())
		Title(name + " — NodeWorkbench");
	// If already initialized (DockInit was called), rebuild palette.
	if(dock_palette)
		RebuildPalette();
}

bool NodeWorkbenchWindow::PickDomain() {
	int count = registered_domains.GetCount();
	if(count == 0) return false;
	if(count == 1) {
		ActivateDomain(*registered_domains[0]);
		return true;
	}

	TopWindow dlg;
	dlg.Title("Select Domain");
	dlg.SetRect(0, 0, 420, 280);

	ArrayCtrl list;
	list.AddColumn("Domain",      140);
	list.AddColumn("Description");
	list.MultiSelect(false);
	for(auto* p : registered_domains)
		list.Add(p->GetDomainName(), p->GetDomainDesc());
	if(list.GetCount()) list.SetCursor(0);

	// Pre-select current domain.
	if(domain) {
		for(int i = 0; i < registered_domains.GetCount(); i++)
			if(registered_domains[i] == domain) { list.SetCursor(i); break; }
	}

	Button ok_btn, cancel_btn;
	ok_btn.SetLabel("OK");
	cancel_btn.SetLabel("Cancel");
	ok_btn     << [&] { dlg.AcceptBreak(1); };
	cancel_btn << [&] { dlg.RejectBreak(0); };

	dlg.Add(list.HSizePos(8, 8).TopPos(8, 200));
	dlg.Add(ok_btn.RightPos(8, 80).BottomPos(8, 26));
	dlg.Add(cancel_btn.RightPos(96, 80).BottomPos(8, 26));

	if(dlg.Run() != 1) return false;
	int sel = list.GetCursor();
	if(sel < 0 || sel >= registered_domains.GetCount()) return false;

	ActivateDomain(*registered_domains[sel]);
	return true;
}

// ---------------------------------------------------------------------------
// Layout persistence
// ---------------------------------------------------------------------------

String NodeWorkbenchWindow::GetLayoutFileName() const {
	if(domain) {
		String id = domain->GetDomainId();
		String safe;
		for(int i = 0; i < id.GetCount(); i++) {
			char c = id[i];
			safe.Cat((IsAlNum(c) || c == '_') ? c : '_');
		}
		if(!safe.IsEmpty())
			return "nodeworkbench_" + safe + "_layout.dat";
	}
	return "nodeworkbench_layout.dat";
}

// ---------------------------------------------------------------------------
// DockInit / Close
// ---------------------------------------------------------------------------

void NodeWorkbenchWindow::DockInit() {
	if(!dock_project) {
		dock_project     = &Dockable(project_panel,     "Solution Explorer").SizeHint(Size(280, 400));
		dock_palette     = &Dockable(palette_panel,     "Node Palette").SizeHint(Size(240, 400));
		dock_diagnostics = &Dockable(diagnostics_panel, "Diagnostics").SizeHint(Size(600, 180));

		Register(*dock_project);
		Register(*dock_palette);
		Register(*dock_diagnostics);

		DockLeft(*dock_palette);
		DockRight(*dock_project);
		DockBottom(*dock_diagnostics);
	}

	FileIn in(GetDataFile(GetLayoutFileName()));
	if(in.IsOpen() && !in.IsError())
		SerializeWindow(in);

	if(domain) {
		domain->OnDomainInit(*this);
		RebuildPalette();
	}

	RefreshProjectTree();
}

void NodeWorkbenchWindow::Close() {
	if(IsOpen()) {
		FileOut out(GetDataFile(GetLayoutFileName()));
		if(out.IsOpen())
			SerializeWindow(out);
		if(domain)
			domain->OnDomainClose(*this);
	}
	TopWindow::Close();
}

// ---------------------------------------------------------------------------
// Status bar
// ---------------------------------------------------------------------------

void NodeWorkbenchWindow::SetStatus(const String& s) {
	status = s;
}

// ---------------------------------------------------------------------------
// Run mode
// ---------------------------------------------------------------------------

void NodeWorkbenchWindow::SetRunMode(WorkbenchRunMode mode) {
	if(run_mode == mode) return;
	run_mode = mode;
	SetStatus(String("Run mode: ") + RunModeLabel(mode));
	if(domain)
		domain->OnRunModeChanged(*this, mode);
}

// ---------------------------------------------------------------------------
// Menu builders
// ---------------------------------------------------------------------------

void NodeWorkbenchWindow::MainMenu(Bar& bar) {
	bar.Add("File",    THISBACK(MenuFile));
	bar.Add("Run",     THISBACK(MenuRun));
	bar.Add("View",    THISBACK(MenuView));
	// When multiple domains are registered, show a "Domain" menu.
	if(registered_domains.GetCount() > 1) {
		bar.Sub("Domain", [=](Bar& b) {
			for(int i = 0; i < registered_domains.GetCount(); i++) {
				auto* d = registered_domains[i];
				String label = (d == domain ? "\u2022 " : "  ") + d->GetDomainName();
				b.Add(label, [=] { ActivateDomain(*registered_domains[i]); });
			}
			if(domain) {
				b.Separator();
				domain->BuildDomainMenu(b);
			}
		});
	} else if(domain) {
		bar.Add(domain->GetDomainName(), THISBACK(MenuDomain));
	}
	bar.Sub("Windows", [=](Bar& b) { DockWindowMenu(b); });
}

void NodeWorkbenchWindow::MenuFile(Bar& bar) {
	bar.Add("Open File...",        THISBACK(ActionOpenFile)).Key(K_CTRL|K_O);
	bar.Separator();
	bar.Add("New Graph",           THISBACK(ActionNewGraph));
	bar.Add("Open Graph...",       THISBACK(ActionOpenGraph));
	bar.Add("Save Graph",          THISBACK(ActionSaveGraph)).Key(K_CTRL|K_S);
	bar.Add("Save Graph As...",    [=] { SaveGraphAs(); });
	bar.Separator();
	bar.Add("New Project",         THISBACK(ActionNewProject));
	bar.Add("Open Project...",     THISBACK(ActionOpenProject));
	bar.Add("Save Project As...",  [=] { SaveProjectAs(); });
	bar.Separator();
	bar.Add("New Solution",        THISBACK(ActionNewSolution));
	bar.Add("Open Solution...",    THISBACK(ActionOpenSolution));
	bar.Add("Save Solution As...", [=] { SaveSolutionAs(); });
	bar.Separator();
	{
		bool has_tpls = false;
		for(auto* d : registered_domains) {
			Vector<INodeWorkbenchDomain::TemplateDesc> tpl;
			d->GetTemplates(tpl);
			if(!tpl.IsEmpty()) { has_tpls = true; break; }
		}
		bar.Add(has_tpls, "New from Template...", THISBACK(ActionNewFromTemplate));
		bar.Separator();
	}
	bar.Add("Exit",                [=] { Close(); });
}

void NodeWorkbenchWindow::MenuRun(Bar& bar) {
	bar.Add("Verify",  THISBACK(ValidateGraph));
	bar.Add("Compile", THISBACK(CompileGraph));
	bar.Add("Run",     THISBACK(RunGraph));
	bool has_startup = !prj.startup_graph.IsEmpty();
	bar.Add(has_startup, "Run Startup Graph", THISBACK(ActionRunStartupGraph));
	bar.Separator();
	// Run mode sub-menu with checkmarks
	bar.Sub("Run Mode", [=](Bar& b) {
		b.Add(run_mode == WorkbenchRunMode::Testing,   "Testing",   [=] { SetRunMode(WorkbenchRunMode::Testing);   });
		b.Add(run_mode == WorkbenchRunMode::Verifying, "Verifying", [=] { SetRunMode(WorkbenchRunMode::Verifying); });
		b.Add(run_mode == WorkbenchRunMode::Running,   "Running",   [=] { SetRunMode(WorkbenchRunMode::Running);   });
	});
}

void NodeWorkbenchWindow::MenuView(Bar& bar) {
	bar.Add("Zoom to Fit",  [=] { viewport.ZoomToFit(); });
	bar.Add("Apply Layout", [=] { viewport.ApplyLayout(); });
}

void NodeWorkbenchWindow::MenuDomain(Bar& bar) {
	if(domain)
		domain->BuildDomainMenu(bar);
}

// ---------------------------------------------------------------------------
// Palette
// ---------------------------------------------------------------------------

void NodeWorkbenchWindow::RebuildPalette() {
	category_list.Clear();
	node_list.Clear();
	if(!domain) return;

	Vector<INodeWorkbenchDomain::PaletteItem> items;
	domain->BuildPalette(items);

	// collect unique ordered categories
	Index<String> cats;
	for(auto& it : items)
		cats.FindAdd(it.category);
	for(int i = 0; i < cats.GetCount(); i++)
		category_list.Add(cats[i]);

	// store items for RefreshNodeList to filter
	// attach them as row data on category_list using SetCtrl-free trick:
	// we repopulate node_list on category selection
	// stash items vector for use in RefreshNodeList via a member
	// (we use last_diags-style storage via a dedicated member — added below)
}

void NodeWorkbenchWindow::RefreshCategoryList() {
	// Called externally; just delegate to RebuildPalette
	RebuildPalette();
}

void NodeWorkbenchWindow::RefreshNodeList() {
	node_list.Clear();
	if(!domain) return;

	int row = category_list.GetCursor();
	if(row < 0) return;
	String cat = category_list.Get(row, 0).ToString();

	Vector<INodeWorkbenchDomain::PaletteItem> items;
	domain->BuildPalette(items);
	for(auto& it : items)
		if(it.category == cat)
			node_list.Add(it.label);
}

// ---------------------------------------------------------------------------
// Project tree
// ---------------------------------------------------------------------------

void NodeWorkbenchWindow::RefreshProjectTree() {
	solution_tree.Clear();

	String root_dir;
	if(!current_sln_path.IsEmpty())
		root_dir = GetFileDirectory(current_sln_path);
	else if(!current_prj_path.IsEmpty())
		root_dir = GetFileDirectory(current_prj_path);

	if(root_dir.IsEmpty()) return;

	int budget = 2000;
	AddDirectoryTree(solution_tree, 0, root_dir, 0, budget);
	solution_tree.OpenDeep(0, 2);
}

void NodeWorkbenchWindow::OpenSelectedProjectTreeItem() {
	if(!solution_tree.IsCursor()) return;
	Value v = solution_tree.Get(solution_tree.GetCursor());
	if(IsNull(v)) return;

	String kind, path;
	if(!ParseTreeKey(v, kind, path)) return;

	if(kind == "graph")    OpenGraphFile(path);
	else if(kind == "project")  OpenProjectFile(path);
	else if(kind == "solution") OpenSolutionFile(path);
}

void NodeWorkbenchWindow::OnProjectTreeMenu(Bar& bar) {
	bool has_cursor = solution_tree.IsCursor();
	String kind, path;
	if(has_cursor) {
		Value v = solution_tree.Get(solution_tree.GetCursor());
		ParseTreeKey(v, kind, path);
	}

	bar.Add(has_cursor, "Open", THISBACK(OpenSelectedProjectTreeItem));
	bar.Separator();
	bar.Add("New Graph...", THISBACK(ActionNewGraph));
	bar.Add(!current_prj_path.IsEmpty(), "Add Graph to Project...", THISBACK(ActionAddGraphToProject));
	bar.Add(!current_sln_path.IsEmpty(), "Add Project to Solution...", THISBACK(ActionAddProjectToSolution));
	bar.Separator();
	bar.Add(has_cursor && kind == "graph" && !current_prj_path.IsEmpty(),
	        "Set as Startup Graph", THISBACK(ActionSetStartupGraph));
	bar.Separator();
	bar.Add(has_cursor && kind != "folder", "Rename...", THISBACK(ActionRenameTreeItem));
	bar.Add(has_cursor && kind != "folder", "Remove from Project/Solution", THISBACK(ActionRemoveTreeItem));
}

void NodeWorkbenchWindow::ActionAddProjectToSolution() {
	if(current_sln_path.IsEmpty()) return;
	String filter = domain ? domain->GetProjectFileFilter()
	                       : "Project files (*.grfproj *.nnprj)\t*.grfproj *.nnprj";
	String path = SelectFileOpen(filter);
	if(path.IsEmpty()) return;
	String rel = NativePath(path);
	if(FindIndex(sln.projects, rel) < 0) {
		sln.projects.Add(rel);
		sln.Save(current_sln_path);
		RefreshProjectTree();
		SetStatus("Added project: " + GetFileName(rel));
	}
}

void NodeWorkbenchWindow::ActionAddGraphToProject() {
	if(current_prj_path.IsEmpty()) return;
	String filter = domain ? domain->GetGraphFileFilter()
	                       : "Graph files (*.grf *.nngrf)\t*.grf *.nngrf";
	String path = SelectFileOpen(filter);
	if(path.IsEmpty()) return;
	String rel = NativePath(path);
	if(FindIndex(prj.graphs, rel) < 0) {
		prj.graphs.Add(rel);
		prj.Save(current_prj_path);
		RefreshProjectTree();
		SetStatus("Added graph: " + GetFileName(rel));
	}
}

void NodeWorkbenchWindow::ActionSetStartupGraph() {
	if(!solution_tree.IsCursor() || current_prj_path.IsEmpty()) return;
	Value v = solution_tree.Get(solution_tree.GetCursor());
	String kind, path;
	if(!ParseTreeKey(v, kind, path) || kind != "graph") return;
	prj.startup_graph = NativePath(path);
	prj.Save(current_prj_path);
	RefreshProjectTree();
	SetStatus("Startup graph: " + GetFileName(path));
}

void NodeWorkbenchWindow::ActionRemoveTreeItem() {
	if(!solution_tree.IsCursor()) return;
	Value v = solution_tree.Get(solution_tree.GetCursor());
	String kind, path;
	if(!ParseTreeKey(v, kind, path)) return;

	if(kind == "graph" && !current_prj_path.IsEmpty()) {
		String rel = NativePath(path);
		int i = FindIndex(prj.graphs, rel);
		if(i >= 0) {
			prj.graphs.Remove(i);
			if(prj.startup_graph == rel)
				prj.startup_graph = String();
			prj.Save(current_prj_path);
			RefreshProjectTree();
			SetStatus("Removed from project: " + GetFileName(path));
		}
	}
	else if(kind == "project" && !current_sln_path.IsEmpty()) {
		String rel = NativePath(path);
		int i = FindIndex(sln.projects, rel);
		if(i >= 0) {
			sln.projects.Remove(i);
			if(sln.active_project == rel)
				sln.active_project = String();
			sln.Save(current_sln_path);
			RefreshProjectTree();
			SetStatus("Removed from solution: " + GetFileName(path));
		}
	}
}

void NodeWorkbenchWindow::ActionRenameTreeItem() {
	if(!solution_tree.IsCursor()) return;
	Value v = solution_tree.Get(solution_tree.GetCursor());
	String kind, path;
	if(!ParseTreeKey(v, kind, path)) return;

	String cur_name = GetFileName(path);
	String ext = GetFileExt(cur_name);
	String base = cur_name.Left(cur_name.GetCount() - ext.GetCount());

	String new_base = base;
	if(!EditText(new_base, "Rename", "New name (without extension):"))
		return;
	new_base = TrimBoth(new_base);
	if(new_base.IsEmpty() || new_base == base) return;

	String new_path = AppendFileName(GetFileDirectory(path), new_base + ext);
	if(FileExists(new_path)) {
		PromptOK("A file with that name already exists.");
		return;
	}
	if(!FileMove(path, new_path)) {
		SetStatus("Rename failed: " + cur_name);
		return;
	}

	// Update references in project/solution
	String old_rel = NativePath(path);
	String new_rel = NativePath(new_path);
	if(kind == "graph" && !current_prj_path.IsEmpty()) {
		int i = FindIndex(prj.graphs, old_rel);
		if(i >= 0) prj.graphs[i] = new_rel;
		if(prj.startup_graph == old_rel) prj.startup_graph = new_rel;
		prj.Save(current_prj_path);
	}
	else if(kind == "project" && !current_sln_path.IsEmpty()) {
		int i = FindIndex(sln.projects, old_rel);
		if(i >= 0) sln.projects[i] = new_rel;
		if(sln.active_project == old_rel) sln.active_project = new_rel;
		sln.Save(current_sln_path);
	}

	if(path == current_graph_path) current_graph_path = new_path;
	if(path == current_prj_path)   current_prj_path   = new_path;
	if(path == current_sln_path)   current_sln_path   = new_path;

	RefreshProjectTree();
	SetStatus("Renamed: " + new_base + ext);
}

// ---------------------------------------------------------------------------
// Diagnostics pane
// ---------------------------------------------------------------------------

void NodeWorkbenchWindow::RefreshDiagnosticsPane() {
	diagnostics_list.Clear();
	for(auto& d : last_diags) {
		diagnostics_list.Add(DiagLabel(d.severity), d.message, d.entity_id);
		diagnostics_list.SetLineColor(
			diagnostics_list.GetCount() - 1, DiagColor(d.severity));
	}
}

void NodeWorkbenchWindow::SetDiagnostics(const Vector<WorkbenchDiagnostic>& diags) {
	last_diags <<= diags;
	RefreshDiagnosticsPane();
}

void NodeWorkbenchWindow::ActionDiagQuickFix() {
	if(!domain) return;
	int row = diagnostics_list.GetCursor();
	if(row < 0 || row >= last_diags.GetCount()) return;

	const WorkbenchDiagnostic& diag = last_diags[row];
	Vector<String> fixes = domain->GetQuickFixes(diag);
	if(fixes.IsEmpty()) {
		SetStatus("No quick fixes available for this diagnostic.");
		return;
	}

	if(fixes.GetCount() == 1) {
		// Apply single fix directly.
		domain->ApplyQuickFix(*this, diag, 0);
		SetStatus("Applied: " + fixes[0]);
		ValidateGraph();
		return;
	}

	// Let user pick among available fixes.
	TopWindow dlg;
	dlg.Title("Quick Fix");
	dlg.SetRect(0, 0, 400, 240);

	ArrayCtrl list;
	list.AddColumn("Fix");
	list.MultiSelect(false);
	for(const String& f : fixes)
		list.Add(f);
	list.SetCursor(0);

	Button ok_btn, cancel_btn;
	ok_btn.SetLabel("Apply");
	cancel_btn.SetLabel("Cancel");
	ok_btn     << [&] { dlg.AcceptBreak(1); };
	cancel_btn << [&] { dlg.RejectBreak(0); };

	dlg.Add(list.HSizePos(8, 8).TopPos(8, 160));
	dlg.Add(ok_btn.RightPos(8, 80).BottomPos(8, 26));
	dlg.Add(cancel_btn.RightPos(96, 80).BottomPos(8, 26));

	if(dlg.Run() != 1) return;
	int sel = list.GetCursor();
	if(sel < 0) return;

	domain->ApplyQuickFix(*this, diag, sel);
	SetStatus("Applied: " + fixes[sel]);
	ValidateGraph();
}

void NodeWorkbenchWindow::ValidateGraph() {
	if(!domain) { SetStatus("No domain — nothing to validate."); return; }
	last_diags.Clear();
	domain->ValidateGraph(*this, last_diags);
	RefreshDiagnosticsPane();
	int errs = 0, warns = 0;
	for(auto& d : last_diags) {
		if(d.severity == DiagSeverity::Error)   errs++;
		if(d.severity == DiagSeverity::Warning) warns++;
	}
	SetStatus(Format("[%s] Validation: %d error(s), %d warning(s).",
	                 RunModeLabel(run_mode), errs, warns));
}

void NodeWorkbenchWindow::CompileGraph() {
	if(!domain) { SetStatus("No domain — nothing to compile."); return; }
	String log;
	bool ok = domain->CompileGraph(*this, log);
	SetStatus(String("[") + RunModeLabel(run_mode) + (ok ? "] Compile OK." : "] Compile FAILED."));
	if(!log.IsEmpty()) {
		last_diags.Clear();
		WorkbenchDiagnostic d;
		d.severity = ok ? DiagSeverity::Info : DiagSeverity::Error;
		d.message  = log;
		d.source   = "compiler";
		last_diags.Add(d);
		RefreshDiagnosticsPane();
	}
}

void NodeWorkbenchWindow::RunGraph() {
	if(!domain) { SetStatus("No domain — nothing to run."); return; }
	String log;
	bool ok = domain->RunGraph(*this, log);
	SetStatus(String("[") + RunModeLabel(run_mode) + (ok ? "] Run OK." : "] Run FAILED."));
}

// ---------------------------------------------------------------------------
// OpenPath — auto-dispatch by extension  (Task 03)
// ---------------------------------------------------------------------------

bool NodeWorkbenchWindow::OpenPath(const String& path) {
	String kind = WorkbenchExtensions::KindFromPath(path);
	if(kind == "solution") return OpenSolutionFile(path);
	if(kind == "project")  return OpenProjectFile(path);
	if(kind == "graph")    return OpenGraphFile(path);
	if(kind == "script") {
		// Script files (.nnpy) — notify domain, open in external editor or log
		if(domain) domain->OnGraphLoaded(*this, path);
		SetStatus("Script: " + GetFileName(path));
		return true;
	}

	// Let any registered domain handle extra extensions.
	String ext = ToLower(GetFileExt(path));
	for(auto* d : registered_domains) {
		if(d->GetExtraExtensions().Find(ext) >= 0) {
			// Activate this domain if not already active.
			if(d != domain) ActivateDomain(*d);
			domain->OnGraphLoaded(*this, path);
			return true;
		}
	}
	SetStatus("Unknown file type: " + GetFileName(path));
	return false;
}

// ---------------------------------------------------------------------------
// File I/O
// ---------------------------------------------------------------------------

bool NodeWorkbenchWindow::OpenGraphFile(const String& path) {
	if(!FileExists(path)) { SetStatus("File not found: " + path); return false; }
	graph.Clear();
	Vector<Node::ValidationMessage> vm;
	if(!Node::LoadEonFile(graph, path, vm)) {
		String msg;
		for(const Node::ValidationMessage& m : vm)
			msg << m.message << "\n";
		if(msg.IsEmpty())
			msg = "Failed to load graph: " + path;
		SetStatus("Graph load failed: " + GetFileName(path));
		PromptOK(DeQtf(msg));
		return false;
	}
	graph.RebuildIndexPublic();
	graph.Invalidate();
	viewport.SetGraph(graph);
	PostCallback([this] { viewport.ApplyLayout(); });
	current_graph_path = NormalizePath(path);
	SetStatus("Graph: " + GetFileName(current_graph_path));
	if(domain) domain->OnGraphLoaded(*this, path);
	return true;
}

bool NodeWorkbenchWindow::SaveGraphFile(const String& path) {
	if(path.IsEmpty()) return false;
	if(domain) domain->OnGraphSaving(*this, path);
	String out = Node::SaveEon(graph);
	if(!SaveFile(path, out)) {
		SetStatus("Failed to save graph: " + GetFileName(path));
		return false;
	}
	current_graph_path = NormalizePath(path);
	SetStatus("Saved: " + GetFileName(current_graph_path));
	return true;
}

bool NodeWorkbenchWindow::OpenProjectFile(const String& path) {
	WorkbenchProject tmp;
	if(!tmp.Load(path)) {
		SetStatus("Failed to load project: " + GetFileName(path));
		return false;
	}
	prj = pick(tmp);
	current_prj_path = path;
	RefreshProjectTree();
	SetStatus("Project: " + prj.name);
	if(domain) domain->OnProjectOpened(*this, prj, path);
	return true;
}

bool NodeWorkbenchWindow::SaveProjectFile(const String& path) {
	if(path.IsEmpty()) return false;
	return prj.Save(path);
}

bool NodeWorkbenchWindow::OpenSolutionFile(const String& path) {
	WorkbenchSolution tmp;
	if(!tmp.Load(path)) {
		SetStatus("Failed to load solution: " + GetFileName(path));
		return false;
	}
	sln = pick(tmp);
	current_sln_path = path;
	RefreshProjectTree();
	SetStatus("Solution: " + sln.name);
	if(domain) domain->OnSolutionOpened(*this, sln, path);
	return true;
}

bool NodeWorkbenchWindow::SaveSolutionFile(const String& path) {
	if(path.IsEmpty()) return false;
	return sln.Save(path);
}

void NodeWorkbenchWindow::SaveGraphAs() {
	String filter = domain ? domain->GetGraphFileFilter()
	                       : "Graph files (*.grf)\t*.grf";
	String path = SelectFileSaveAs(filter);
	if(path.IsEmpty()) return;
	current_graph_path = path;
	SaveGraphFile(path);
}

void NodeWorkbenchWindow::SaveProjectAs() {
	String filter = domain ? domain->GetProjectFileFilter()
	                       : "Project files (*.grfproj)\t*.grfproj";
	String path = SelectFileSaveAs(filter);
	if(path.IsEmpty()) return;
	current_prj_path = path;
	SaveProjectFile(path);
}

void NodeWorkbenchWindow::SaveSolutionAs() {
	String filter = domain ? domain->GetSolutionFileFilter()
	                       : "Solution files (*.slnx *.sln)\t*.slnx *.sln";
	String path = SelectFileSaveAs(filter);
	if(path.IsEmpty()) return;
	current_sln_path = path;
	SaveSolutionFile(path);
}

// ---------------------------------------------------------------------------
// Template generation dialog + action
// ---------------------------------------------------------------------------

void NodeWorkbenchWindow::ActionNewFromTemplate() {
	if(registered_domains.IsEmpty()) return;

	// Collect templates from all registered domains; tag each with domain index.
	struct TaggedTpl : public Moveable<TaggedTpl> {
		INodeWorkbenchDomain::TemplateDesc desc;
		int domain_idx;
		int tpl_idx;
	};
	Vector<TaggedTpl> all;
	for(int di = 0; di < registered_domains.GetCount(); di++) {
		Vector<INodeWorkbenchDomain::TemplateDesc> tpls;
		registered_domains[di]->GetTemplates(tpls);
		for(int ti = 0; ti < tpls.GetCount(); ti++) {
			TaggedTpl t;
			t.desc       = tpls[ti];
			t.domain_idx = di;
			t.tpl_idx    = ti;
			all.Add(pick(t));
		}
	}
	if(all.IsEmpty()) { SetStatus("No templates available."); return; }

	TopWindow dlg;
	dlg.Title("New from Template");
	dlg.SetRect(0, 0, 580, 360);
	dlg.Sizeable();

	ArrayCtrl list;
	list.AddColumn("Name",     180);
	list.AddColumn("Domain",   110);
	list.AddColumn("Category", 120);
	list.AddColumn("Description");
	list.MultiSelect(false);
	for(auto& t : all)
		list.Add(t.desc.name,
		         registered_domains[t.domain_idx]->GetDomainName(),
		         t.desc.category,
		         t.desc.description);
	if(list.GetCount()) list.SetCursor(0);

	Button ok_btn, cancel_btn;
	ok_btn.SetLabel("OK");
	cancel_btn.SetLabel("Cancel");
	ok_btn << [&] { dlg.AcceptBreak(1); };
	cancel_btn << [&] { dlg.RejectBreak(0); };

	dlg.Add(list.HSizePos(8, 8).TopPos(8, 260));
	dlg.Add(ok_btn.RightPos(8, 80).BottomPos(8, 26));
	dlg.Add(cancel_btn.RightPos(96, 80).BottomPos(8, 26));

	if(dlg.Run() != 1) return;
	int sel = list.GetCursor();
	if(sel < 0 || sel >= all.GetCount()) return;

	String dest_dir = SelectDirectory();
	if(dest_dir.IsEmpty()) return;

	const TaggedTpl& chosen = all[sel];
	INodeWorkbenchDomain* tpl_domain = registered_domains[chosen.domain_idx];

	String err;
	String result_path = tpl_domain->GenerateTemplate(chosen.tpl_idx, dest_dir, err);
	if(result_path.IsEmpty()) {
		PromptOK("Template generation failed:\n" + DeQtf(err));
		return;
	}

	// Activate the domain that owns this template.
	ActivateDomain(*tpl_domain);

	OpenPath(result_path);
	RefreshProjectTree();
	SetStatus("Template created: " + GetFileName(result_path));
}

// ---------------------------------------------------------------------------
// Action wrappers
// ---------------------------------------------------------------------------

void NodeWorkbenchWindow::ActionNewGraph() {
	// If multiple domains are available, let user pick which one to use.
	if(registered_domains.GetCount() > 1) {
		if(!PickDomain()) return;
	}
	graph.Clear();
	viewport.SetGraph(graph);
	current_graph_path = String();
	SetStatus(String("New graph") + (domain ? " [" + domain->GetDomainName() + "]" : "") + ".");
}

void NodeWorkbenchWindow::ActionOpenGraph() {
	String filter = domain ? domain->GetGraphFileFilter()
	                       : "Graph files (*.grf *.nngrf)\t*.grf *.nngrf";
	String path = SelectFileOpen(filter);
	if(path.IsEmpty()) return;
	OpenGraphFile(path);
}

void NodeWorkbenchWindow::ActionSaveGraph() {
	if(current_graph_path.IsEmpty()) SaveGraphAs();
	else SaveGraphFile(current_graph_path);
}

void NodeWorkbenchWindow::ActionNewProject() {
	prj = WorkbenchProject();
	current_prj_path = String();
	RefreshProjectTree();
	SetStatus("New project.");
}

void NodeWorkbenchWindow::ActionOpenProject() {
	String filter = domain ? domain->GetProjectFileFilter()
	                       : "Project files (*.grfproj *.nnprj)\t*.grfproj *.nnprj";
	String path = SelectFileOpen(filter);
	if(path.IsEmpty()) return;
	OpenProjectFile(path);
}

void NodeWorkbenchWindow::ActionNewSolution() {
	sln = WorkbenchSolution();
	prj = WorkbenchProject();
	current_sln_path = String();
	current_prj_path = String();
	RefreshProjectTree();
	SetStatus("New solution.");
}

void NodeWorkbenchWindow::ActionOpenSolution() {
	String filter = domain ? domain->GetSolutionFileFilter()
	                       : "Solution files (*.slnx *.sln *.nnsln)\t*.slnx *.sln *.nnsln";
	String path = SelectFileOpen(filter);
	if(path.IsEmpty()) return;
	OpenSolutionFile(path);
}

void NodeWorkbenchWindow::ActionOpenFile() {
	// Build a combined filter from all registered domains.
	String all_exts;
	String filter_parts;
	for(auto* d : registered_domains) {
		auto collect = [&](const String& f) {
			// f is "Label\text1 ext2"; extract the extension part
			int tab = f.Find('\t');
			if(tab >= 0) {
				String exts = f.Mid(tab + 1);
				if(!all_exts.IsEmpty()) all_exts << " ";
				all_exts << exts;
				filter_parts << f << "\n";
			}
		};
		collect(d->GetSolutionFileFilter());
		collect(d->GetProjectFileFilter());
		collect(d->GetGraphFileFilter());
	}
	// Also add the known built-in extensions if no domains are registered.
	if(all_exts.IsEmpty())
		all_exts = "*.slnx *.sln *.nnsln *.grfproj *.nnprj *.grf *.nngrf *.nnpy";

	String combined = "All known files (" + all_exts + ")\t" + all_exts;
	if(!filter_parts.IsEmpty())
		combined << "\n" << filter_parts;

	String path = SelectFileOpen(combined);
	if(path.IsEmpty()) return;
	if(!OpenPath(path))
		PromptOK("Could not open file:\n" + DeQtf(path));
}

void NodeWorkbenchWindow::ActionRunStartupGraph() {
	if(prj.startup_graph.IsEmpty()) {
		SetStatus("No startup graph set in the active project.");
		return;
	}
	String path = prj.startup_graph;
	// Resolve relative paths against the project directory.
	if(!IsFullPath(path) && !current_prj_path.IsEmpty())
		path = NormalizePath(AppendFileName(GetFileDirectory(current_prj_path), path));

	if(!FileExists(path)) {
		SetStatus("Startup graph not found: " + GetFileName(path));
		PromptOK("Startup graph not found:\n" + DeQtf(path));
		return;
	}
	if(!OpenGraphFile(path)) return;
	RunGraph();
}

END_UPP_NAMESPACE
