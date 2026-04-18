#include "NodeWorkbench.h"

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// helpers (file-local)
// ---------------------------------------------------------------------------
namespace {

String MakeTreeKey(const String& kind, const String& path) {
	return kind + "|" + path;
}

bool ParseTreeKey(const Value& value, String& kind, String& path) {
	if(IsNull(value))
		return false;
	String key = value;
	int split = key.Find('|');
	if(split < 0)
		return false;
	kind = key.Left(split);
	path = key.Mid(split + 1);
	return true;
}

String FileKindFromPath(const String& path) {
	String ext = ToLower(GetFileExt(path));
	// new formats
	if(ext == ".sln" || ext == ".slnx") return "solution";
	if(ext == ".grfproj")               return "project";
	if(ext == ".grf")                   return "graph";
	// legacy compatibility
	if(ext == ".nnsln")  return "solution";
	if(ext == ".nnprj")  return "project";
	if(ext == ".nngrf")  return "graph";
	return "file";
}

bool SkipTreeDirectory(const String& name) {
	String n = ToLower(name);
	return n == "." || n == ".." || n == ".git" || n == ".svn" || n == ".hg" ||
	       n == "build" || n == "bin" || n == "obj" || n == "out" ||
	       n == "__pycache__";
}

void AddDirectoryTree(TreeArrayCtrl& tree, int parent_id,
                      const String& dir, int depth, int& budget) {
	if(depth > 12 || budget <= 0)
		return;

	Vector<String> dirs, files;
	FindFile ff(AppendFileName(dir, "*"));
	while(ff) {
		String name = ff.GetName();
		String path = NormalizePath(ff.GetPath());
		if(name.GetCount() && name[0] == '.') { ff.Next(); continue; }
		if(ff.IsFolder()) {
			if(!SkipTreeDirectory(name))
				dirs.Add(path);
		} else if(ff.IsFile()) {
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
		String kind = FileKindFromPath(f);
		tree.Add(parent_id, CtrlImg::File(),
		         MakeTreeKey(kind, f), GetFileName(f), false);
	}
}

} // namespace

// ---------------------------------------------------------------------------
// NodeWorkbenchWindow
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
	btn_diag_clear.SetLabel("Clear");
	btn_diag_clear << [=] { diagnostics_list.Clear(); };
	diagnostics_panel.Add(btn_diag_clear.LeftPos(6, 60).BottomPos(6, 24));
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

void NodeWorkbenchWindow::RegisterDomain(INodeWorkbenchDomain& d) {
	domain = &d;
	String title = d.GetDomainName();
	if(!title.IsEmpty())
		Title(title + " — NodeWorkbench");
}

String NodeWorkbenchWindow::GetLayoutFileName() const {
	if(domain) {
		String name = domain->GetDomainName();
		// sanitise: keep only alphanumeric and underscore
		String safe;
		for(int i = 0; i < name.GetCount(); i++) {
			char c = name[i];
			if(IsAlNum(c) || c == '_')
				safe.Cat(c);
			else
				safe.Cat('_');
		}
		if(!safe.IsEmpty())
			return "nodeworkbench_" + ToLower(safe) + "_layout.dat";
	}
	return "nodeworkbench_layout.dat";
}

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

	// restore persisted layout
	FileIn in(GetDataFile(GetLayoutFileName()));
	if(in.IsOpen() && !in.IsError())
		SerializeWindow(in);

	// let the domain add its own registrations
	if(domain)
		domain->OnDomainInit(*this);

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
// Menu builders
// ---------------------------------------------------------------------------

void NodeWorkbenchWindow::MainMenu(Bar& bar) {
	bar.Add("File",   THISBACK(MenuFile));
	bar.Add("View",   THISBACK(MenuView));
	if(domain)
		bar.Add(domain->GetDomainName(), THISBACK(MenuDomain));
	bar.Sub("Windows", [=](Bar& b) { DockWindowMenu(b); });
}

void NodeWorkbenchWindow::MenuFile(Bar& bar) {
	bar.Add("New Graph",       THISBACK(ActionNewGraph));
	bar.Add("Open Graph...",   THISBACK(ActionOpenGraph));
	bar.Add("Save Graph",      THISBACK(ActionSaveGraph));
	bar.Separator();
	bar.Add("Open Project...", THISBACK(ActionOpenProject));
	bar.Separator();
	bar.Add("Open Solution...",THISBACK(ActionOpenSolution));
	bar.Separator();
	bar.Add("Exit",            [=] { Close(); });
}

void NodeWorkbenchWindow::MenuView(Bar& bar) {
	bar.Add("Zoom to Fit", [=] { viewport.ZoomToFit(); });
	bar.Add("Apply Layout", [=] { viewport.ApplyLayout(); });
}

void NodeWorkbenchWindow::MenuDomain(Bar& bar) {
	if(domain)
		domain->BuildDomainMenu(bar);
}

// ---------------------------------------------------------------------------
// Project tree
// ---------------------------------------------------------------------------

void NodeWorkbenchWindow::RefreshProjectTree() {
	solution_tree.Clear();

	// If a solution root directory is known, populate from it.
	String root_dir;
	if(!current_sln_path.IsEmpty())
		root_dir = GetFileDirectory(current_sln_path);
	else if(!current_prj_path.IsEmpty())
		root_dir = GetFileDirectory(current_prj_path);

	if(root_dir.IsEmpty())
		return;

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

	if(kind == "graph")
		OpenGraphFile(path);
	else if(kind == "project")
		OpenProjectFile(path);
	else if(kind == "solution")
		OpenSolutionFile(path);
}

void NodeWorkbenchWindow::OnProjectTreeMenu(Bar& bar) {
	bar.Add("Open", THISBACK(OpenSelectedProjectTreeItem));
	bar.Separator();
	bar.Add("New Graph", THISBACK(ActionNewGraph));
}

// ---------------------------------------------------------------------------
// Palette
// ---------------------------------------------------------------------------

void NodeWorkbenchWindow::RefreshCategoryList() {
	// Populated by domain in OnDomainInit via viewport.RegisterNodeType().
	// For now leave the list empty — it is a stub hook point.
	category_list.Clear();
}

void NodeWorkbenchWindow::RefreshNodeList() {
	node_list.Clear();
}

// ---------------------------------------------------------------------------
// File I/O
// ---------------------------------------------------------------------------

bool NodeWorkbenchWindow::OpenGraphFile(const String& path) {
	String text = LoadFile(path);
	if(text.IsEmpty() && !FileExists(path)) {
		SetStatus("Failed to open: " + path);
		return false;
	}
	// TODO: deserialise graph from text (domain-specific or generic .grf JSON)
	current_graph_path = path;
	SetStatus("Opened: " + GetFileName(path));
	if(domain)
		domain->OnGraphLoaded(*this, path);
	return true;
}

bool NodeWorkbenchWindow::SaveGraphFile(const String& path) {
	if(path.IsEmpty()) return false;
	if(domain)
		domain->OnGraphSaving(*this, path);
	// TODO: serialise graph to file
	SetStatus("Saved: " + GetFileName(path));
	return true;
}

bool NodeWorkbenchWindow::OpenProjectFile(const String& path) {
	current_prj_path = path;
	RefreshProjectTree();
	SetStatus("Project: " + GetFileName(path));
	return true;
}

bool NodeWorkbenchWindow::SaveProjectFile(const String& path) {
	(void)path;
	return true;
}

bool NodeWorkbenchWindow::OpenSolutionFile(const String& path) {
	current_sln_path = path;
	RefreshProjectTree();
	SetStatus("Solution: " + GetFileName(path));
	return true;
}

bool NodeWorkbenchWindow::SaveSolutionFile(const String& path) {
	(void)path;
	return true;
}

// ---- action wrappers ----

void NodeWorkbenchWindow::ActionNewGraph() {
	graph.Clear();
	viewport.SetGraph(graph);
	current_graph_path = String();
	SetStatus("New graph.");
}

void NodeWorkbenchWindow::ActionOpenGraph() {
	String filter = domain ? domain->GetGraphFileFilter()
	                       : "Graph files (*.grf *.nngrf)\t*.grf *.nngrf";
	String path = SelectFileOpen(filter);
	if(path.IsEmpty()) return;
	OpenGraphFile(path);
}

void NodeWorkbenchWindow::ActionSaveGraph() {
	if(current_graph_path.IsEmpty()) {
		String filter = domain ? domain->GetGraphFileFilter()
		                       : "Graph files (*.grf)\t*.grf";
		String path = SelectFileSaveAs(filter);
		if(path.IsEmpty()) return;
		current_graph_path = path;
	}
	SaveGraphFile(current_graph_path);
}

void NodeWorkbenchWindow::ActionOpenProject() {
	String filter = domain ? domain->GetProjectFileFilter()
	                       : "Project files (*.grfproj *.nnprj)\t*.grfproj *.nnprj";
	String path = SelectFileOpen(filter);
	if(path.IsEmpty()) return;
	OpenProjectFile(path);
}

void NodeWorkbenchWindow::ActionOpenSolution() {
	String filter = domain ? domain->GetSolutionFileFilter()
	                       : "Solution files (*.slnx *.sln *.nnsln)\t*.slnx *.sln *.nnsln";
	String path = SelectFileOpen(filter);
	if(path.IsEmpty()) return;
	OpenSolutionFile(path);
}

END_UPP_NAMESPACE
