#include "Overviewer.h"

OverviewerWindow::OverviewerWindow() {
	Title("Overviewer");
	Sizeable().Zoomable();

	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));

	tree.WhenSel = THISBACK(OnTreeSelection);
	
	CreateFlagsPane();
	CreateNumericPane();
	CreateInfoPane();
}

void OverviewerWindow::CreateFlagsPane() {
	flags_pane.Add(temporary.SetLabel("TEMPORARY").TopPos(0, 20).HSizePos());
	flags_pane.Add(wrong_location.SetLabel("WRONG_LOCATION").TopPos(20, 20).HSizePos());
	flags_pane.Add(wrong_name.SetLabel("WRONG_NAME").TopPos(40, 20).HSizePos());
	flags_pane.Add(too_large.SetLabel("TOO_LARGE").TopPos(60, 20).HSizePos());
	flags_pane.Add(needs_review.SetLabel("NEEDS_REVIEW").TopPos(80, 20).HSizePos());
	flags_pane.Add(content_needs_review.SetLabel("CONTENT_NEEDS_REVIEW").TopPos(100, 20).HSizePos());
	
	temporary <<= THISBACK(OnMetadataChange);
	wrong_location <<= THISBACK(OnMetadataChange);
	wrong_name <<= THISBACK(OnMetadataChange);
	too_large <<= THISBACK(OnMetadataChange);
	needs_review <<= THISBACK(OnMetadataChange);
	content_needs_review <<= THISBACK(OnMetadataChange);
}

void OverviewerWindow::CreateNumericPane() {
	numeric_pane.Add(quality.LeftPos(0, 100).TopPos(0, 20));
	numeric_pane.Add(completion.LeftPos(0, 100).TopPos(25, 20));
	numeric_pane.Add(priority.LeftPos(0, 100).TopPos(50, 20));
	
	for(int i = 0; i <= 5; i++) {
		quality.Add(i);
		completion.Add(i);
		priority.Add(i);
	}
	
	quality.WhenAction = THISBACK(OnMetadataChange);
	completion.WhenAction = THISBACK(OnMetadataChange);
	priority.WhenAction = THISBACK(OnMetadataChange);
}

void OverviewerWindow::CreateInfoPane() {
	info_pane.Add(path_lbl.TopPos(0, 20).HSizePos());
	info_pane.Add(type_lbl.TopPos(20, 20).HSizePos());
	info_pane.Add(size_lbl.TopPos(40, 20).HSizePos());
}

void OverviewerWindow::DockInit() {
	dock_tree = &Dockable(tree, "File Tree").SizeHint(Size(300, 600));
	dock_flags = &Dockable(flags_pane, "Flags").SizeHint(Size(200, 150));
	dock_numeric = &Dockable(numeric_pane, "Attributes").SizeHint(Size(200, 100));
	dock_info = &Dockable(info_pane, "Info").SizeHint(Size(200, 100));
	
	Register(*dock_tree);
	Register(*dock_flags);
	Register(*dock_numeric);
	Register(*dock_info);
	
	DockLeft(*dock_tree);
	DockRight(*dock_flags);
	DockBottom(*dock_numeric);
	DockBottom(*dock_info);
}

void OverviewerWindow::SyncTitle() {
	String t = "Overviewer";
	if (project.path.IsEmpty())
		t << " - (Untitled)";
	else
		t << " - " << project.path;
	if (dirty)
		t << " *";
	Title(t);
}

void OverviewerWindow::New() {
	if (!ConfirmSave()) return;
	project.Reset();
	RefreshTree();
	ClearDirty();
}

void OverviewerWindow::Open() {
	if (!ConfirmSave()) return;
	FileSel fs;
	fs.Type("Project File", "*.json");
	if (fs.ExecuteOpen()) {
		OpenFile(fs.Get());
	}
}

void OverviewerWindow::OpenFile(const String& path) {
	String content = LoadFile(path);
	if (content.IsEmpty()) {
		Exclamation("Failed to load project file.");
		return;
	}
	try {
		OverviewerProject p;
		LoadFromJson(p, content);
		project = pick(p);
		project.path = path;
		RefreshTree();
		ClearDirty();
	} catch (const Exc& e) {
		Exclamation("Failed to parse project file: " + e);
	}
}

void OverviewerWindow::Save() {
	if (project.path.IsEmpty()) {
		SaveAs();
		return;
	}
	String json = StoreAsJson(project);
	if (!SaveFile(project.path, json)) {
		Exclamation("Failed to save project file.");
		return;
	}
	ClearDirty();
}

void OverviewerWindow::SaveAs() {
	FileSel fs;
	fs.Type("Project File", "*.json");
	if (fs.ExecuteSaveAs()) {
		project.path = fs.Get();
		Save();
	}
}

bool OverviewerWindow::ConfirmSave() {
	if (!dirty) return true;
	int res = PromptYesNoCancel("Save changes to project?");
	if (res == 1) {
		Save();
		return !dirty;
	}
	return res == 0;
}

void OverviewerWindow::Exit() {
	if (ConfirmSave()) {
		Break();
	}
}

void OverviewerWindow::Close() {
	if (ConfirmSave()) {
		TopWindow::Close();
	}
}

static void ScanDir(TreeCtrl& tree, int parent, const String& dir, const String& base, const OverviewerProject& project, int filter_mode) {
	for(FindFile ff(AppendFileName(dir, "*")); ff; ff.Next()) {
		String rel = ff.GetName();
		if(!base.IsEmpty()) rel = AppendFileName(base, rel);
		
		bool visible = true;
		if(filter_mode > 0) {
			const FileMetadata* m = project.metadata.FindPtr(rel);
			if(filter_mode == 1) visible = (m && m->flags != 0);
			else if(filter_mode == 2) visible = (m && m->priority == 5);
		}
		
		if(ff.IsFolder()) {
			int node = tree.Add(parent, CtrlImg::Dir(), rel, ff.GetName());
			ScanDir(tree, node, ff.GetPath(), rel, project, filter_mode);
		} else if(visible) {
			tree.Add(parent, CtrlImg::File(), rel, ff.GetName());
		}
	}
}

void OverviewerWindow::RefreshTree() {
	tree.Clear();
	String root_dir = project.working_dir;
	if(root_dir.IsEmpty() && !project.path.IsEmpty())
		root_dir = GetFileDirectory(project.path);
	
	if(root_dir.IsEmpty()) return;
	
	int root = tree.Add(0, CtrlImg::Dir(), ".", root_dir);
	ScanDir(tree, root, root_dir, "", project, filter_mode);
	tree.Open(root);
}

void OverviewerWindow::OnTreeSelection() {
	int id = tree.GetCursor();
	if(id >= 0) {
		current_selection = (String)tree.Get(id);
		UpdatePanels();
	} else {
		current_selection = "";
		UpdatePanels();
	}
}

void OverviewerWindow::UpdatePanels() {
	if(current_selection.IsEmpty() || current_selection == ".") {
		temporary = 0; wrong_location = 0; wrong_name = 0;
		too_large = 0; needs_review = 0; content_needs_review = 0;
		quality = 0; completion = 0; priority = 0;
		path_lbl = ""; type_lbl = ""; size_lbl = "";
		return;
	}
	
	const FileMetadata* m = project.metadata.FindPtr(current_selection);
	if(m) {
		temporary = !!(m->flags & FLAG_TEMPORARY);
		wrong_location = !!(m->flags & FLAG_WRONG_LOCATION);
		wrong_name = !!(m->flags & FLAG_WRONG_NAME);
		too_large = !!(m->flags & FLAG_TOO_LARGE);
		needs_review = !!(m->flags & FLAG_NEEDS_REVIEW);
		content_needs_review = !!(m->flags & FLAG_CONTENT_NEEDS_REVIEW);
		quality.SetIndex(m->quality);
		completion.SetIndex(m->completion);
		priority.SetIndex(m->priority);
	} else {
		temporary = 0; wrong_location = 0; wrong_name = 0;
		too_large = 0; needs_review = 0; content_needs_review = 0;
		quality.SetIndex(0); completion.SetIndex(0); priority.SetIndex(0);
	}
	
	path_lbl = current_selection;
	String root_path = project.working_dir;
	if(root_path.IsEmpty() && !project.path.IsEmpty()) root_path = GetFileDirectory(project.path);
	String abs_path = AppendFileName(root_path, current_selection);
	
	if(DirectoryExists(abs_path)) {
		type_lbl = "Type: Directory";
		size_lbl = "";
	} else {
		type_lbl = "Type: File";
		size_lbl = "Size: " + FormatInt64(GetFileLength(abs_path));
	}
}

void OverviewerWindow::OnMetadataChange() {
	if(current_selection.IsEmpty() || current_selection == ".") return;
	
	FileMetadata& m = project.metadata.GetAdd(current_selection);
	m.flags = 0;
	if(temporary) m.flags |= FLAG_TEMPORARY;
	if(wrong_location) m.flags |= FLAG_WRONG_LOCATION;
	if(wrong_name) m.flags |= FLAG_WRONG_NAME;
	if(too_large) m.flags |= FLAG_TOO_LARGE;
	if(needs_review) m.flags |= FLAG_NEEDS_REVIEW;
	if(content_needs_review) m.flags |= FLAG_CONTENT_NEEDS_REVIEW;
	
	m.quality = quality.GetIndex();
	m.completion = completion.GetIndex();
	m.priority = priority.GetIndex();
	
	MarkDirty();
}

void OverviewerWindow::MainMenu(Bar& bar) {
	bar.Add("File", THISBACK(FileMenu));
	bar.Add("Edit", THISBACK(EditMenu));
	bar.Add("View", THISBACK(ViewMenu));
	bar.Add("Help", THISBACK(HelpMenu));
}

void OverviewerWindow::FileMenu(Bar& bar) {
	bar.Add("New", THISBACK(New));
	bar.Add("Open", THISBACK(Open));
	bar.Add("Save", THISBACK(Save));
	bar.Add("Save As", THISBACK(SaveAs));
	bar.Separator();
	bar.Add("Exit", THISBACK(Exit));
}

void OverviewerWindow::EditMenu(Bar& bar) {
	bar.Add("Mark Dirty (debug)", THISBACK(MarkDirty));
}

void OverviewerWindow::ViewMenu(Bar& bar) {
	bar.Add("Show All", [=] { filter_mode = 0; RefreshTree(); }).Check(filter_mode == 0);
	bar.Add("Show with Any Flag", [=] { filter_mode = 1; RefreshTree(); }).Check(filter_mode == 1);
	bar.Add("Show with Priority 5", [=] { filter_mode = 2; RefreshTree(); }).Check(filter_mode == 2);
}

void OverviewerWindow::HelpMenu(Bar& bar) {
	bar.Add("About", [] { PromptOK("Overviewer Milestone 2"); });
}
