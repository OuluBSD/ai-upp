#include "Overviewer.h"

void OverviewerWindow::TagPanel::Refresh() {
	list.Clear();
	for(const String& s : assigned)
		list.Add(s);
}

void OverviewerWindow::TagPanel::OnAdd() {
	String name;
	if(!EditText(name, "Add Tag", "Tag name:")) return;
	name = TrimBoth(name);
	if(name.IsEmpty()) return;
	
	if(FindIndex(assigned, name) < 0) {
		assigned.Add(name);
		if(FindIndex(global_known, name) < 0)
			global_known.Add(name);
		Refresh();
		when_change();
	}
}

void OverviewerWindow::TagPanel::OnRemove() {
	int id = list.GetCursor();
	if(id < 0) return;
	String name = list.Get(id, 0);
	int idx = FindIndex(assigned, name);
	if(idx >= 0) {
		assigned.Remove(idx);
		Refresh();
		when_change();
	}
}

void OverviewerWindow::ListPanel::Refresh() {
	list.Clear();
	for(const ListItem& it : items)
		list.Add(it.done ? "X" : "", it.text, it.date, it.commit);
}

void OverviewerWindow::ListPanel::OnAdd() {
	String text;
	if(!EditText(text, "Add Item", "Text:")) return;
	text = TrimBoth(text);
	if(text.IsEmpty()) return;
	ListItem& it = items.Add();
	it.text = text;
	Refresh();
	when_change();
}

void OverviewerWindow::ListPanel::OnEdit() {
	int id = list.GetCursor();
	if(id < 0) return;
	ListItem& it = items[id];
	if(!EditText(it.text, "Edit Item", "Text:")) return;
	it.text = TrimBoth(it.text);
	Refresh();
	when_change();
}

void OverviewerWindow::ListPanel::OnRemove() {
	int id = list.GetCursor();
	if(id < 0) return;
	items.Remove(id);
	Refresh();
	when_change();
}

void OverviewerWindow::ListPanel::OnToggleDone() {
	int id = list.GetCursor();
	if(id < 0) return;
	items[id].done = !items[id].done;
	Refresh();
	when_change();
}

OverviewerWindow::OverviewerWindow() 
	: current_tags_pane(dummy_metadata.current_tags, project.known_current_tags)
	, reason_tags_pane(dummy_metadata.reason_tags, project.known_reason_tags)
	, gap_tags_pane(dummy_metadata.gap_tags, project.known_gap_tags)
	, problems_pane(dummy_metadata.problems)
	, tasks_pane(dummy_metadata.tasks)
	, leads_pane(dummy_metadata.leads)
{
	Title("Overviewer");
	Sizeable().Zoomable();

	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));

	tree.WhenSel = THISBACK(OnTreeSelection);
	
	CreateFlagsPane();
	CreateNumericPane();
	CreateInfoPane();
	
	notes_editor.WhenAction = THISBACK(OnNoteChange);

	auto wire_tags = [&](TagPanel& p) {
		p.add.SetLabel("Add").WhenAction = [&p]{ p.OnAdd(); };
		p.remove.SetLabel("Remove").WhenAction = [&p]{ p.OnRemove(); };
		p.when_change = THISBACK(OnMetadataChange);
		// Note: Button layout is simple for now
		p.Add(p.add.LeftPos(0, 60).BottomPos(0, 20));
		p.Add(p.remove.LeftPos(65, 60).BottomPos(0, 20));
	};
	wire_tags(current_tags_pane);
	wire_tags(reason_tags_pane);
	wire_tags(gap_tags_pane);

	auto wire_list = [&](ListPanel& p) {
		p.add.SetLabel("Add").WhenAction = [&p]{ p.OnAdd(); };
		p.edit.SetLabel("Edit").WhenAction = [&p]{ p.OnEdit(); };
		p.remove.SetLabel("Remove").WhenAction = [&p]{ p.OnRemove(); };
		p.toggle_done.SetLabel("Done").WhenAction = [&p]{ p.OnToggleDone(); };
		p.when_change = THISBACK(OnMetadataChange);
		p.Add(p.add.LeftPos(0, 50).BottomPos(0, 20));
		p.Add(p.edit.LeftPos(55, 50).BottomPos(0, 20));
		p.Add(p.remove.LeftPos(110, 60).BottomPos(0, 20));
		p.Add(p.toggle_done.LeftPos(175, 50).BottomPos(0, 20));
	};
	wire_list(problems_pane);
	wire_list(tasks_pane);
	wire_list(leads_pane);
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
	dock_notes = &Dockable(notes_editor, "Notes").SizeHint(Size(400, 300));
	
	dock_current_tags = &Dockable(current_tags_pane, "Current Tags").SizeHint(Size(200, 200));
	dock_reason_tags = &Dockable(reason_tags_pane, "Reason Tags").SizeHint(Size(200, 200));
	dock_gap_tags = &Dockable(gap_tags_pane, "Gap/Future Tags").SizeHint(Size(200, 200));
	
	dock_problems = &Dockable(problems_pane, "Problems").SizeHint(Size(300, 200));
	dock_tasks = &Dockable(tasks_pane, "Tasks").SizeHint(Size(300, 200));
	dock_leads = &Dockable(leads_pane, "Leads").SizeHint(Size(300, 200));
	
	Register(*dock_tree);
	Register(*dock_flags);
	Register(*dock_numeric);
	Register(*dock_info);
	Register(*dock_notes);
	Register(*dock_current_tags);
	Register(*dock_reason_tags);
	Register(*dock_gap_tags);
	Register(*dock_problems);
	Register(*dock_tasks);
	Register(*dock_leads);
	
	DockLeft(*dock_tree);
	DockRight(*dock_flags);
	DockBottom(*dock_numeric);
	DockBottom(*dock_info);
	
	DockBottom(*dock_notes, *dock_tree);
	
	DockBottom(*dock_current_tags, *dock_flags);
	DockBottom(*dock_reason_tags, *dock_current_tags);
	DockBottom(*dock_gap_tags, *dock_reason_tags);
	
	DockRight(*dock_problems);
	DockBottom(*dock_tasks, *dock_problems);
	DockBottom(*dock_leads, *dock_tasks);
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
	// Re-bind panes to selected metadata
	FileMetadata* m = current_selection.IsEmpty() || current_selection == "." ? nullptr : &project.metadata.GetAdd(current_selection);
	
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
		
		notes_editor.SetData(m->notes);
		
		current_tags_pane.assigned = m->current_tags;
		reason_tags_pane.assigned = m->reason_tags;
		gap_tags_pane.assigned = m->gap_tags;
		
		problems_pane.items = m->problems;
		tasks_pane.items = m->tasks;
		leads_pane.items = m->leads;
	} else {
		temporary = 0; wrong_location = 0; wrong_name = 0;
		too_large = 0; needs_review = 0; content_needs_review = 0;
		quality.SetIndex(0); completion.SetIndex(0); priority.SetIndex(0);
		notes_editor.SetData("");
		
		current_tags_pane.assigned = dummy_metadata.current_tags;
		reason_tags_pane.assigned = dummy_metadata.reason_tags;
		gap_tags_pane.assigned = dummy_metadata.gap_tags;
		
		problems_pane.items = dummy_metadata.problems;
		tasks_pane.items = dummy_metadata.tasks;
		leads_pane.items = dummy_metadata.leads;
	}
	
	current_tags_pane.Refresh();
	reason_tags_pane.Refresh();
	gap_tags_pane.Refresh();
	problems_pane.Refresh();
	tasks_pane.Refresh();
	leads_pane.Refresh();
	
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

void OverviewerWindow::OnNoteChange() {
	if(current_selection.IsEmpty() || current_selection == ".") return;
	FileMetadata& m = project.metadata.GetAdd(current_selection);
	String n = notes_editor.GetData();
	if(m.notes != n) {
		m.notes = n;
		MarkDirty();
	}
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
	bar.Add("About", [] { PromptOK("Overviewer Milestone 3"); });
}
