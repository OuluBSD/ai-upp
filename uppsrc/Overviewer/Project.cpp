#include "Overviewer.h"

#define LAYOUTFILE <Overviewer/Overviewer.lay>
#include <CtrlCore/lay.h>

SettingsWindow::SettingsWindow() {
	CtrlLayoutOKCancel(*this, "Settings");
	backup_mode.Add(0, "Alongside project (.autosave.json)");
	backup_mode.Add(1, "In project recovery dir");
}

void SettingsWindow::Load() {
	OverviewerSettings& s = GetSettings();
	autosave_enabled = s.autosave_enabled;
	autosave_interval.SetData(s.autosave_interval_minutes);
	backup_mode.SetData(s.backup_mode);
	restore_layout = s.restore_layout;
}

void SettingsWindow::Save() {
	OverviewerSettings& s = GetSettings();
	s.autosave_enabled = autosave_enabled;
	s.autosave_interval_minutes = (int)autosave_interval.GetData();
	s.backup_mode = (int)backup_mode.GetData();
	s.restore_layout = restore_layout;
	s.Save();
}

OverviewerWindow::SuggestionPanel::SuggestionPanel() {
	Add(list.SizePos());
	list.AddColumn("Type");
	list.AddColumn("Category");
	list.AddColumn("Value");
	list.AddColumn("Conf");
	list.AddColumn("Source");
	list.WhenLeftDouble = THISBACK(OnApply);
}

void OverviewerWindow::SuggestionPanel::Refresh() {
	list.Clear();
	if(!suggestions) return;
	auto add_sugs = [&](const char* type, const char* cat, const Vector<Suggestion>& v, int type_id, int cat_id) {
		for(const Suggestion& s : v) {
			if(!s.rejected)
				list.Add(type, cat, s.text, FormatDouble(s.confidence, 2), s.source, type_id, cat_id);
		}
	};
	add_sugs("Tag", "Current", suggestions->current_tags, 0, 0);
	add_sugs("Tag", "Reason", suggestions->reason_tags, 0, 1);
	add_sugs("Tag", "Gap", suggestions->gap_tags, 0, 2);
	add_sugs("Problem", "", suggestions->problems, 1, 0);
	add_sugs("Task", "", suggestions->tasks, 2, 0);
}

void OverviewerWindow::SuggestionPanel::OnApply() {
	int id = list.GetCursor();
	if(id < 0 || !window || !current_path) return;
	window->ApplySuggestion(*current_path, (int)list.Get(id, 5), (int)list.Get(id, 6), (String)list.Get(id, 2));
}

void OverviewerWindow::SuggestionPanel::OnDismiss() {
	int id = list.GetCursor();
	if(id < 0 || !window || !current_path) return;
	window->DismissSuggestion(*current_path, (int)list.Get(id, 5), (int)list.Get(id, 6), (String)list.Get(id, 2));
}

void OverviewerWindow::ApplySuggestion(const String& path, int type, int category, const String& value) {
	FileMetadata& m = project.metadata.GetAdd(path);
	String desc = "Applied suggestion: " + value;
	if(type == 0) { // Tag
		Vector<String>* v = (category == 0 ? &m.current_tags : (category == 1 ? &m.reason_tags : &m.gap_tags));
		if(FindIndex(*v, value) < 0) {
			v->Add(value);
			project.LogEvent(path, "add_tag", desc, "", value, "suggestion");
		}
	} else if(type == 1) {
		m.problems.Add().text = value;
		project.LogEvent(path, "add_problem", desc, "", value, "suggestion");
	} else if(type == 2) {
		m.tasks.Add().text = value;
		project.LogEvent(path, "add_task", desc, "", value, "suggestion");
	}
	DismissSuggestion(path, type, category, value);
	MarkDirty();
	UpdatePanels();
}

void OverviewerWindow::DismissSuggestion(const String& path, int type, int category, const String& value) {
	EntrySuggestions* sug = project.suggestions.FindPtr(path);
	if(!sug) return;
	auto dismiss = [&](Vector<Suggestion>& v) {
		for(Suggestion& s : v) if(s.text == value) s.rejected = true;
	};
	if(type == 0) {
		if(category == 0) dismiss(sug->current_tags);
		else if(category == 1) dismiss(sug->reason_tags);
		else dismiss(sug->gap_tags);
	} else if(type == 1) dismiss(sug->problems);
	else if(type == 2) dismiss(sug->tasks);
	suggestion_pane.Refresh();
}

void OverviewerWindow::DashboardPanel::Refresh(const ProjectDashboard& db) {
	stats.Clear();
	stats.Add("Total Files", db.total_files);
	stats.Add("Total Directories", db.total_dirs);
	stats.Add("Recent Changes", db.recent_changes);
	stats.Add("Stale Entries", db.stale_entries);
	stats.Add("Flagged Entries", db.flagged_entries);
	stats.Add("Needs Review (Flagged)", db.needs_review);
	stats.Add("Suggestions Pending", db.suggestions_pending);
	
	if(!db.top_action_items.IsEmpty()) {
		stats.Add("--- TOP ACTIONS ---", "");
		for(int i = 0; i < db.top_action_items.GetCount(); i++)
			stats.Add(db.top_action_items.GetKey(i), db.top_action_items[i]);
	}
}

OverviewerWindow::ReviewQueuePanel::ReviewQueuePanel() {
	Add(list.SizePos());
	list.AddColumn("Path");
	list.AddColumn("Type");
	list.AddColumn("Message");
	list.AddColumn("Sev");
	list.WhenLeftDouble = THISBACK(OnJump);
}

void OverviewerWindow::ReviewQueuePanel::Refresh(const Vector<ReviewItem>& queue) {
	list.Clear();
	for(const auto& it : queue) {
		if(!it.dismissed)
			list.Add(it.path, it.type, it.message, it.severity);
	}
}

void OverviewerWindow::ReviewQueuePanel::OnJump() {
	int id = list.GetCursor();
	if(id < 0 || !window) return;
}

void OverviewerWindow::ReviewQueuePanel::OnDismiss() {
	int id = list.GetCursor();
	if(id < 0 || !window) return;
	String p = (String)list.Get(id, 0);
	String msg = (String)list.Get(id, 2);
	window->project.dismissed_review_ids.FindAdd(p + ":" + msg);
	window->RefreshReviewQueue();
}

OverviewerWindow::TimelinePanel::TimelinePanel() {
	Add(list.SizePos());
	list.AddColumn("Time");
	list.AddColumn("Path");
	list.AddColumn("Type");
	list.AddColumn("Description");
	list.WhenLeftDouble = THISBACK(OnJump);
}

void OverviewerWindow::TimelinePanel::Refresh(const Vector<HistoryEvent>& history) {
	list.Clear();
	for(int i = history.GetCount() - 1; i >= 0; i--) {
		const auto& e = history[i];
		list.Add(Format(e.time), e.path, e.type, e.description);
	}
}

void OverviewerWindow::TimelinePanel::OnJump() {
	int id = list.GetCursor();
	if(id < 0 || !window) return;
}

OverviewerWindow::ActionViewPanel::ActionViewPanel() {
	Add(list.SizePos());
	list.AddColumn("Score", 20);
	list.AddColumn("Path");
	list.AddColumn("Factors");
	list.WhenLeftDouble = THISBACK(OnJump);
}

void OverviewerWindow::ActionViewPanel::Refresh(const VectorMap<String, EntryScore>& view) {
	list.Clear();
	for(int i = 0; i < view.GetCount(); i++)
		list.Add(FormatDouble(view[i].score, 1), view.GetKey(i), Join(view[i].factors, ", "));
}

void OverviewerWindow::ActionViewPanel::OnJump() {
	int id = list.GetCursor();
	if(id < 0 || !window) return;
}

OverviewerWindow::OverviewPreviewPanel::OverviewPreviewPanel() {
	Add(view.VSizePos(0, 40).HSizePos());
	Add(refresh.SetLabel("Refresh").LeftPos(5, 80).BottomPos(5, 25));
	Add(markdown.SetLabel("Markdown").LeftPos(90, 100).BottomPos(5, 25));
	Add(export_btn.SetLabel("Export...").RightPos(5, 80).BottomPos(5, 25));
	markdown = true;
	refresh.WhenAction = THISBACK(Refresh);
	export_btn.WhenAction = [this]{ if(window) window->OnExportOverview(); };
}

void OverviewerWindow::OverviewPreviewPanel::Refresh() {
	if(!window) return;
	OverviewOptions opt;
	opt.markdown_output = markdown;
	String text = OverviewGenerator(window->project).GenerateProject(opt);
	view.SetData(text);
}

OverviewerWindow::GitHistoryPanel::GitHistoryPanel() {
	Add(list.SizePos());
	list.AddColumn("Hash", 20);
	list.AddColumn("Author");
	list.AddColumn("Date");
	list.AddColumn("Subject");
	list.WhenLeftDouble = THISBACK(OnLink);
}

void OverviewerWindow::GitHistoryPanel::Refresh(const Vector<GitCommit>& history) {
	list.Clear();
	for(const auto& c : history)
		list.Add(c.hash, c.author, Format(c.date), c.subject);
}

void OverviewerWindow::GitHistoryPanel::OnLink() {
	int id = list.GetCursor();
	if(id < 0 || !window) return;
}

FileMetadata OverviewerProject::GetEffectiveMetadata(const String& rel_path) const {
	FileMetadata res;
	const FileMetadata* m = metadata.FindPtr(rel_path);
	if(m) {
		res.flags = m->flags;
		res.quality = m->quality;
		res.completion = m->completion;
		res.priority = m->priority;
		res.notes = m->notes;
		for(const auto& x : m->current_tags) res.current_tags.Add(x);
		for(const auto& x : m->reason_tags) res.reason_tags.Add(x);
		for(const auto& x : m->gap_tags) res.gap_tags.Add(x);
	}

	auto inherit = [&](int& val, int (FileMetadata::*field)) {
		if(val != 0) return;
		String p = rel_path;
		while(!p.IsEmpty()) {
			p = GetFileDirectory(p);
			if(p.EndsWith("/") || p.EndsWith("\\")) p.Trim(p.GetCount()-1);
			if(p.IsEmpty()) break;
			const FileMetadata* pm = metadata.FindPtr(p);
			if(pm && pm->*field != 0) {
				val = pm->*field;
				break;
			}
		}
	};

	inherit(res.quality, &FileMetadata::quality);
	inherit(res.completion, &FileMetadata::completion);
	inherit(res.priority, &FileMetadata::priority);

	return res;
}

String OverviewerProject::GetBackupPath() const {
	if(path.IsEmpty()) return "";
	if(GetSettings().backup_mode == 0)
		return path + ".autosave.json";
	else
		return AppendFileName(GetFileDirectory(path), ".overviewer_recovery/" + GetFileName(path));
}

bool OverviewerProject::WriteBackup() const {
	String bpath = GetBackupPath();
	if(bpath.IsEmpty()) return false;
	RealizeDirectory(GetFileDirectory(bpath));
	return StoreAsJsonFile(*this, bpath);
}

void OverviewerProject::LogEvent(const String& path, const String& type, const String& desc, const String& old_val, const String& new_val, const String& src) {
	HistoryEvent& e = history.Add();
	e.time = GetSysTime();
	e.path = path;
	e.type = type;
	e.description = desc;
	e.old_value = old_val;
	e.new_value = new_val;
	e.source = src;
	
	if(history.GetCount() > max_history)
		history.Remove(0, history.GetCount() - max_history);
}

void OverviewerProject::RefreshGit() {
	if(!working_dir.IsEmpty()) git.Refresh(working_dir);
}

BatchEditDialog::BatchEditDialog(OverviewerProject& p, const String& start_path) : project(p), initial_path(start_path) {
	Title("Batch Edit");
	Sizeable().Zoomable();
	SetRect(0, 0, 600, 500);

	Add(list.LeftPos(10, 200).VSizePos(10, 70));
	list.AddColumn("Path");
	list.MultiSelect();

	for(int i = 0; i < project.metadata.GetCount(); i++)
		list.Add(project.metadata.GetKey(i));
	
	if(!initial_path.IsEmpty()) {
		for(int i = 0; i < list.GetCount(); i++) {
			if((String)list.Get(i, 0) == initial_path) {
				list.SetCursor(i);
				break;
			}
		}
	}

	Add(recursive.SetLabel("Include subdirectories (recursive)").LeftPos(10, 250).BottomPos(40, 20));

	int right_x = 220;
	Add(apply_flags.SetLabel("Modify Flags").LeftPos(right_x, 150).TopPos(10, 20));
	Add(op_flags.LeftPos(right_x + 160, 100).TopPos(10, 20));
	op_flags.Add("Add").Add("Remove");
	op_flags.SetIndex(0);

	Add(f_temp.SetLabel("TEMPORARY").LeftPos(right_x + 10, 150).TopPos(35, 20));
	Add(f_loc.SetLabel("WRONG_LOCATION").LeftPos(right_x + 10, 150).TopPos(55, 20));
	Add(f_name.SetLabel("WRONG_NAME").LeftPos(right_x + 10, 150).TopPos(75, 20));
	Add(f_large.SetLabel("TOO_LARGE").LeftPos(right_x + 10, 150).TopPos(95, 20));
	Add(f_needs.SetLabel("NEEDS_REVIEW").LeftPos(right_x + 10, 150).TopPos(115, 20));
	Add(f_content.SetLabel("CONTENT_NEEDS_REVIEW").LeftPos(right_x + 10, 150).TopPos(135, 20));

	Add(apply_numeric.SetLabel("Modify Numeric").LeftPos(right_x, 150).TopPos(165, 20));
	Add(en_q.SetLabel("Q").LeftPos(right_x + 10, 30).TopPos(190, 20));
	Add(quality.LeftPos(right_x + 45, 60).TopPos(190, 20));
	Add(en_c.SetLabel("C").LeftPos(right_x + 115, 30).TopPos(190, 20));
	Add(completion.LeftPos(right_x + 150, 60).TopPos(190, 20));
	Add(en_p.SetLabel("P").LeftPos(right_x + 220, 30).TopPos(190, 20));
	Add(priority.LeftPos(right_x + 255, 60).TopPos(190, 20));
	for(int i = 0; i <= 5; i++) { quality.Add(i); completion.Add(i); priority.Add(i); }
	quality.SetIndex(0); completion.SetIndex(0); priority.SetIndex(0);

	Add(apply_tags.SetLabel("Modify Tags").LeftPos(right_x, 150).TopPos(220, 20));
	Add(op_tags.LeftPos(right_x + 160, 100).TopPos(220, 20));
	op_tags.Add("Add").Add("Remove");
	op_tags.SetIndex(0);
	Add(en_tc.SetLabel("Current").LeftPos(right_x + 10, 80).TopPos(245, 20));
	Add(tag_current.LeftPos(right_x + 100, 200).TopPos(245, 20));
	Add(en_tr.SetLabel("Reason").LeftPos(right_x + 10, 80).TopPos(270, 20));
	Add(tag_reason.LeftPos(right_x + 100, 200).TopPos(270, 20));
	Add(en_tg.SetLabel("Gap").LeftPos(right_x + 10, 80).TopPos(295, 20));
	Add(tag_gap.LeftPos(right_x + 100, 200).TopPos(295, 20));

	Add(ok.SetLabel("Apply").RightPos(110, 80).BottomPos(10, 30));
	Add(cancel.SetLabel("Cancel").RightPos(10, 80).BottomPos(10, 30));
	ok.WhenAction = THISBACK(OnApply);
	cancel.WhenAction = [this]{ Reject(); };
}

void BatchEditDialog::OnApply() {
	Vector<String> targets;
	for(int i = 0; i < list.GetCount(); i++)
		if(list.IsSelected(i)) targets.Add(list.Get(i, 0));
	
	if(targets.IsEmpty()) { Exclamation("No targets selected."); return; }

	auto apply_to = [&](const String& path) {
		FileMetadata& m = project.metadata.GetAdd(path);
		if(apply_flags) {
			uint32 bits = 0;
			if(f_temp) bits |= FLAG_TEMPORARY;
			if(f_loc) bits |= FLAG_WRONG_LOCATION;
			if(f_name) bits |= FLAG_WRONG_NAME;
			if(f_large) bits |= FLAG_TOO_LARGE;
			if(f_needs) bits |= FLAG_NEEDS_REVIEW;
			if(f_content) bits |= FLAG_CONTENT_NEEDS_REVIEW;
			if(op_flags.GetIndex() == 0) m.flags |= bits;
			else m.flags &= ~bits;
		}
		if(apply_numeric) {
			if(en_q) m.quality = quality.GetIndex();
			if(en_c) m.completion = completion.GetIndex();
			if(en_p) m.priority = priority.GetIndex();
		}
		if(apply_tags) {
			auto mod_tag = [&](const String& val, Vector<String>& v) {
				if(val.IsEmpty()) return;
				int idx = FindIndex(v, val);
				if(op_tags.GetIndex() == 0) { if(idx < 0) v.Add(val); }
				else { if(idx >= 0) v.Remove(idx); }
			};
			if(en_tc) mod_tag(~tag_current, m.current_tags);
			if(en_tr) mod_tag(~tag_reason, m.reason_tags);
			if(en_tg) mod_tag(~tag_gap, m.gap_tags);
		}
	};

	project.LogEvent("", "batch_update", "Batch edit applied to " + AsString(targets.GetCount()) + " entries");

	for(const String& t : targets) {
		apply_to(t);
		if(recursive) {
			for(int i = 0; i < project.metadata.GetCount(); i++) {
				String p = project.metadata.GetKey(i);
				if(p.StartsWith(t + "/") || p.StartsWith(t + "\\"))
					apply_to(p);
			}
		}
	}
	Break(IDOK);
}

void OverviewerWindow::TagPanel::Refresh() {
	list.Clear();
	if(assigned) {
		for(const String& s : *assigned)
			list.Add(s);
	}
}

void OverviewerWindow::TagPanel::OnAdd() {
	if(!assigned || !global_known) return;
	String name;
	if(!EditText(name, "Add Tag", "Tag name:")) return;
	name = TrimBoth(name);
	if(name.IsEmpty()) return;
	
	if(FindIndex(*assigned, name) < 0) {
		assigned->Add(name);
		if(FindIndex(*global_known, name) < 0)
			global_known->Add(name);
		Refresh();
		if(when_change) when_change();
	}
}

void OverviewerWindow::TagPanel::OnRemove() {
	if(!assigned) return;
	int id = list.GetCursor();
	if(id < 0) return;
	String name = (String)list.Get(id, 0);
	int idx = FindIndex(*assigned, name);
	if(idx >= 0) {
		assigned->Remove(idx);
		Refresh();
		if(when_change) when_change();
	}
}

void OverviewerWindow::ListPanel::Refresh() {
	list.Clear();
	if(items) {
		for(const ListItem& it : *items)
			list.Add(it.done ? "X" : "", it.text, it.date, it.commit);
	}
}

void OverviewerWindow::ListPanel::OnAdd() {
	if(!items) return;
	String text;
	if(!EditText(text, "Add Item", "Text:")) return;
	text = TrimBoth(text);
	if(text.IsEmpty()) return;
	ListItem& it = items->Add();
	it.text = text;
	Refresh();
	if(when_change) when_change();
}

void OverviewerWindow::ListPanel::OnEdit() {
	if(!items) return;
	int id = list.GetCursor();
	if(id < 0) return;
	ListItem& it = (*items)[id];
	if(!EditText(it.text, "Edit Item", "Text:")) return;
	it.text = TrimBoth(it.text);
	Refresh();
	if(when_change) when_change();
}

void OverviewerWindow::ListPanel::OnRemove() {
	if(!items) return;
	int id = list.GetCursor();
	if(id < 0) return;
	items->Remove(id);
	Refresh();
	if(when_change) when_change();
}

void OverviewerWindow::ListPanel::OnToggleDone() {
	if(!items) return;
	int id = list.GetCursor();
	if(id < 0) return;
	(*items)[id].done = !(*items)[id].done;
	Refresh();
	if(when_change) when_change();
}

OverviewerWindow::OverviewerWindow() 
	: current_tags_pane(&dummy_metadata.current_tags, &project.known_current_tags)
	, reason_tags_pane(&dummy_metadata.reason_tags, &project.known_reason_tags)
	, gap_tags_pane(&dummy_metadata.gap_tags, &project.known_gap_tags)
	, problems_pane(&dummy_metadata.problems)
	, tasks_pane(&dummy_metadata.tasks)
	, leads_pane(&dummy_metadata.leads)
{
	dummy_metadata.flags = 0;
	dummy_metadata.quality = 0;
	dummy_metadata.completion = 0;
	dummy_metadata.priority = 0;

	suggestion_pane.window = this;
	suggestion_pane.current_path = &current_selection;
	suggestion_pane.suggestions = &dummy_suggestions;
	
	review_queue_pane.window = this;
	timeline_pane.window = this;
	action_view_pane.window = this;
	overview_preview_pane.window = this;
	git_history_pane.window = this;

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
		p.when_change = [=]{ OnMetadataChange(); };
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
		p.when_change = [=]{ OnMetadataChange(); };
		p.Add(p.add.LeftPos(0, 50).BottomPos(0, 20));
		p.Add(p.edit.LeftPos(55, 50).BottomPos(0, 20));
		p.Add(p.remove.LeftPos(110, 60).BottomPos(0, 20));
		p.Add(p.toggle_done.LeftPos(175, 50).BottomPos(0, 20));
	};
	wire_list(problems_pane);
	wire_list(tasks_pane);
	wire_list(leads_pane);

	last_autosave = GetSysTime();
	SetTimeCallback(-1000, THISBACK(CheckAutosave));
}

void OverviewerWindow::CheckAutosave() {
	OverviewerSettings& s = GetSettings();
	if(s.autosave_enabled && !project.path.IsEmpty() && dirty) {
		if(GetSysTime() - last_autosave > s.autosave_interval_minutes * 60) {
			if(project.WriteBackup()) {
				last_autosave = GetSysTime();
			}
		}
	}
	SetTimeCallback(-1000, THISBACK(CheckAutosave));
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
	dock_suggestions = &Dockable(suggestion_pane, "Suggestions").SizeHint(Size(400, 200));
	dock_dashboard = &Dockable(dashboard_pane, "Dashboard").SizeHint(Size(300, 400));
	dock_review_queue = &Dockable(review_queue_pane, "Review Queue").SizeHint(Size(400, 300));
	dock_timeline = &Dockable(timeline_pane, "Timeline").SizeHint(Size(400, 300));
	dock_action_view = &Dockable(action_view_pane, "Action View").SizeHint(Size(400, 300));
	dock_overview_preview = &Dockable(overview_preview_pane, "Overview Preview").SizeHint(Size(600, 400));
	dock_git_history = &Dockable(git_history_pane, "Git History").SizeHint(Size(400, 200));
	
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
	Register(*dock_suggestions);
	Register(*dock_dashboard);
	Register(*dock_review_queue);
	Register(*dock_timeline);
	Register(*dock_action_view);
	Register(*dock_overview_preview);
	Register(*dock_git_history);
	
	DockLeft(*dock_tree);
	DockRight(*dock_flags);
	DockBottom(*dock_numeric);
	DockBottom(*dock_info);
	
	DockBottom(*dock_notes);
	
	DockBottom(*dock_current_tags);
	DockBottom(*dock_reason_tags);
	DockBottom(*dock_gap_tags);
	
	DockRight(*dock_problems);
	DockBottom(*dock_tasks);
	DockBottom(*dock_leads);
	DockBottom(*dock_suggestions);
	
	DockLeft(*dock_dashboard);
	DockBottom(*dock_review_queue);
	DockBottom(*dock_timeline);
	DockBottom(*dock_action_view);
	DockBottom(*dock_overview_preview);
	DockBottom(*dock_git_history);
}

void OverviewerWindow::SaveLayout() {
	if(GetSettings().restore_layout) {
		FileOut out(ConfigFile("layout.bin"));
		Serialize(out);
	}
}

void OverviewerWindow::LoadLayout() {
	if(GetSettings().restore_layout) {
		FileIn in(ConfigFile("layout.bin"));
		if(in) Serialize(in);
	}
}

void OverviewerWindow::MarkSession(bool active) {
	String path = ConfigFile("session.active");
	if(active) SaveFile(path, "1");
	else DeleteFile(path);
}

bool OverviewerWindow::CheckRecovery() {
	if(FileExists(ConfigFile("session.active"))) {
		String bpath = project.GetBackupPath();
		if(FileExists(bpath)) {
			int res = PromptYesNoCancel("Previous session ended unexpectedly. Recover from autosave?");
			if(res == 1) {
				OpenFile(bpath);
				project.path = ""; // Clear path so user has to Save As
				MarkDirty();
				project.LogEvent("", "recovery_loaded", "Recovered project from autosave", "", bpath);
				return true;
			} else if(res == 0) {
				DeleteFile(bpath);
			}
		}
	}
	return false;
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
	RefreshDashboard();
	RefreshReviewQueue();
	RefreshTimeline();
	RefreshActionView();
	RefreshOverviewPreview();
	RefreshGitHistory();
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
		project.path = path;
		project.working_dir = p.working_dir;
		project.version = p.version;
		project.metadata <<= p.metadata;
		project.suggestions <<= p.suggestions;
		project.dismissed_review_ids <<= p.dismissed_review_ids;
		project.history <<= p.history;
		project.known_current_tags <<= p.known_current_tags;
		project.known_reason_tags <<= p.known_reason_tags;
		project.known_gap_tags <<= p.known_gap_tags;
		RefreshTree();
		RefreshDashboard();
		RefreshReviewQueue();
		RefreshTimeline();
		RefreshActionView();
		RefreshOverviewPreview();
		RefreshGitHistory();
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
		MarkSession(false);
		SaveLayout();
		Break();
	}
}

void OverviewerWindow::Close() {
	if (ConfirmSave()) {
		MarkSession(false);
		SaveLayout();
		TopWindow::Close();
	}
}

static void ScanDir(TreeCtrl& tree, int parent, const String& dir, const String& base, const OverviewerProject& project, const OverviewerWindow::FilterConfig& filter) {
	for(FindFile ff(AppendFileName(dir, "*")); ff; ff.Next()) {
		String rel = ff.GetName();
		if(!base.IsEmpty()) rel = AppendFileName(base, rel);
		
		bool visible = true;
		if(filter.mode == 1) {
			FileMetadata m = project.GetEffectiveMetadata(rel);
			if(filter.flags != 0 && (m.flags & filter.flags) == 0) visible = false;
			if(filter.priority_min > 0 && m.priority < filter.priority_min) visible = false;
			if(filter.missing_priority && m.priority != 0) visible = false;
			if(filter.missing_completion && m.completion != 0) visible = false;
			if(!filter.tag_current.IsEmpty() && FindIndex(m.current_tags, filter.tag_current) < 0) visible = false;
			if(!filter.tag_reason.IsEmpty() && FindIndex(m.reason_tags, filter.tag_reason) < 0) visible = false;
			if(!filter.tag_gap.IsEmpty() && FindIndex(m.gap_tags, filter.tag_gap) < 0) visible = false;
		}
		
		if(ff.IsFolder()) {
			int node = tree.Add(parent, CtrlImg::Dir(), rel, ff.GetName());
			ScanDir(tree, node, ff.GetPath(), rel, project, filter);
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
	ScanDir(tree, root, root_dir, "", project, filter);
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
	FileMetadata effective = project.GetEffectiveMetadata(current_selection);
	FileMetadata* m = current_selection.IsEmpty() || current_selection == "." ? nullptr : &project.metadata.GetAdd(current_selection);
	EntrySuggestions* s = current_selection.IsEmpty() || current_selection == "." ? nullptr : project.suggestions.FindPtr(current_selection);

	if(m) {
		temporary = !!(m->flags & FLAG_TEMPORARY);
		wrong_location = !!(m->flags & FLAG_WRONG_LOCATION);
		wrong_name = !!(m->flags & FLAG_WRONG_NAME);
		too_large = !!(m->flags & FLAG_TOO_LARGE);
		needs_review = !!(m->flags & FLAG_NEEDS_REVIEW);
		content_needs_review = !!(m->flags & FLAG_CONTENT_NEEDS_REVIEW);
		
		quality.SetIndex(effective.quality);
		completion.SetIndex(effective.completion);
		priority.SetIndex(effective.priority);
		
		notes_editor.SetData(m->notes);
		current_tags_pane.assigned = &m->current_tags;
		reason_tags_pane.assigned = &m->reason_tags;
		gap_tags_pane.assigned = &m->gap_tags;
		problems_pane.items = &m->problems;
		tasks_pane.items = &m->tasks;
		leads_pane.items = &m->leads;
	} else {
		temporary = 0; wrong_location = 0; wrong_name = 0;
		too_large = 0; needs_review = 0; content_needs_review = 0;
		quality.SetIndex(0); completion.SetIndex(0); priority.SetIndex(0);
		notes_editor.SetData("");
		current_tags_pane.assigned = &dummy_metadata.current_tags;
		reason_tags_pane.assigned = &dummy_metadata.reason_tags;
		gap_tags_pane.assigned = &dummy_metadata.gap_tags;
		problems_pane.items = &dummy_metadata.problems;
		tasks_pane.items = &dummy_metadata.tasks;
		leads_pane.items = &dummy_metadata.leads;
	}
	
	suggestion_pane.suggestions = s ? s : &dummy_suggestions;
	suggestion_pane.Refresh();

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
	
	RefreshTimeline();
	RefreshActionView();
	RefreshGitHistory();
}

void OverviewerWindow::OnMetadataChange() {
	if(current_selection.IsEmpty() || current_selection == ".") return;
	FileMetadata& m = project.metadata.GetAdd(current_selection);
	
	// Logging
	auto log_if_changed = [&](int& field, int new_val, const char* type) {
		if(field != new_val) {
			project.LogEvent(current_selection, type, String().Cat() << "Changed " << type << " from " << field << " to " << new_val, AsString(field), AsString(new_val));
			field = new_val;
		}
	};
	
	log_if_changed(m.quality, quality.GetIndex(), "set_quality");
	log_if_changed(m.completion, completion.GetIndex(), "set_completion");
	log_if_changed(m.priority, priority.GetIndex(), "set_priority");

	uint32 bits = 0;
	if(temporary) bits |= FLAG_TEMPORARY;
	if(wrong_location) bits |= FLAG_WRONG_LOCATION;
	if(wrong_name) bits |= FLAG_WRONG_NAME;
	if(too_large) bits |= FLAG_TOO_LARGE;
	if(needs_review) bits |= FLAG_NEEDS_REVIEW;
	if(content_needs_review) bits |= FLAG_CONTENT_NEEDS_REVIEW;
	
	if(m.flags != bits) {
		project.LogEvent(current_selection, "set_flags", "Metadata flags changed", AsString((int)m.flags), AsString((int)bits));
		m.flags = bits;
	}

	MarkDirty();
	RefreshTimeline();
	RefreshActionView();
}

void OverviewerWindow::OnNoteChange() {
	if(current_selection.IsEmpty() || current_selection == ".") return;
	FileMetadata& m = project.metadata.GetAdd(current_selection);
	String n = notes_editor.GetData();
	if(m.notes != n) {
		project.LogEvent(current_selection, "set_note", "Note modified");
		m.notes = n;
		MarkDirty();
		RefreshTimeline();
		RefreshActionView();
	}
}

void OverviewerWindow::OnBatchEdit() {
	BatchEditDialog dlg(project, current_selection);
	if(dlg.Run() == IDOK) { MarkDirty(); RefreshTree(); RefreshTimeline(); RefreshActionView(); }
}

void OverviewerWindow::OnSettings() {
	SettingsWindow dlg;
	dlg.Load();
	if(dlg.Run() == IDOK) {
		dlg.Save();
	}
}

void OverviewerWindow::OnAnalyze() {
	if(!current_selection.IsEmpty() && current_selection != ".") {
		project.AnalyzeEntry(current_selection);
		project.LogEvent(current_selection, "generate_suggestions", "Heuristic analysis triggered");
		UpdatePanels();
	}
}

void OverviewerWindow::OnRunConsistencyCheck() {
	project.RunConsistencyCheck();
	project.LogEvent("", "run_consistency_check", "Project-wide consistency check finished");
	RefreshReviewQueue();
	RefreshDashboard();
	RefreshTimeline();
	RefreshActionView();
}

void OverviewerWindow::OnShowDashboard() {
	RefreshDashboard();
	if(dock_dashboard) dock_dashboard->Show();
}

void OverviewerWindow::OnShowReviewQueue() {
	RefreshReviewQueue();
	if(dock_review_queue) dock_review_queue->Show();
}

void OverviewerWindow::OnShowTimeline() {
	RefreshTimeline();
	if(dock_timeline) dock_timeline->Show();
}

void OverviewerWindow::OnShowActionView() {
	RefreshActionView();
	if(dock_action_view) dock_action_view->Show();
}

void OverviewerWindow::OnShowOverviewPreview() {
	RefreshOverviewPreview();
	if(dock_overview_preview) dock_overview_preview->Show();
}

void OverviewerWindow::OnShowGitHistory() {
	RefreshGitHistory();
	if(dock_git_history) dock_git_history->Show();
}

void OverviewerWindow::RefreshReviewQueue() {
	review_queue_pane.Refresh(project.review_queue);
}

void OverviewerWindow::RefreshDashboard() {
	dashboard_pane.Refresh(project.GetDashboard());
}

void OverviewerWindow::RefreshTimeline() {
	timeline_pane.Refresh(project.history);
}

void OverviewerWindow::RefreshActionView() {
	action_view_pane.Refresh(project.GetActionView(20));
}

void OverviewerWindow::RefreshOverviewPreview() {
	overview_preview_pane.Refresh();
}

void OverviewerWindow::RefreshGitHistory() {
	git_history_pane.Refresh(project.git.GetHistory(project.working_dir, current_selection));
}

void OverviewerWindow::OnExportOverview() {
	FileSel fs;
	fs.Type("Markdown", "*.md");
	fs.Type("Text", "*.txt");
	if(fs.ExecuteSaveAs("Export Overview")) {
		OverviewOptions opt;
		opt.markdown_output = GetFileExt(fs.Get()) == ".md";
		String text = OverviewGenerator(project).GenerateProject(opt);
		SaveFile(fs.Get(), text);
	}
}

void OverviewerWindow::OnRefreshGit() {
	project.RefreshGit();
	RefreshGitHistory();
	RefreshDashboard();
	RefreshReviewQueue();
}

void OverviewerWindow::MainMenu(Bar& bar) {
	bar.Add("File", THISBACK(FileMenu));
	bar.Add("Edit", THISBACK(EditMenu));
	bar.Add("View", THISBACK(ViewMenu));
	bar.Add("Tools", THISBACK(ToolsMenu));
	bar.Add("Help", THISBACK(HelpMenu));
}

void OverviewerWindow::FileMenu(Bar& bar) {
	bar.Add("New", THISBACK(New));
	bar.Add("Open", THISBACK(Open));
	bar.Add("Save", THISBACK(Save));
	bar.Add("Save As", THISBACK(SaveAs));
	bar.Separator();
	bar.Add("Export Overview...", THISBACK(OnExportOverview));
	bar.Separator();
	bar.Add("Exit", THISBACK(Exit));
}

void OverviewerWindow::EditMenu(Bar& bar) {
	bar.Add("Batch Edit...", THISBACK(OnBatchEdit));
	bar.Add("Settings...", THISBACK(OnSettings));
	bar.Add("Mark Dirty (debug)", THISBACK(MarkDirty));
}

void OverviewerWindow::ViewMenu(Bar& bar) {
	bar.Add("Show All", [=] { filter.mode = 0; RefreshTree(); }).Check(filter.mode == 0);
	bar.Add("Advanced Filter", [=] {
		filter.mode = 1;
		String tags;
		if(EditText(tags, "Filter Tags", "Current tag:")) filter.tag_current = tags;
		RefreshTree();
	}).Check(filter.mode == 1);
	bar.Separator();
	bar.Add("Dashboard", THISBACK(OnShowDashboard));
	bar.Add("Review Queue", THISBACK(OnShowReviewQueue));
	bar.Add("Timeline", THISBACK(OnShowTimeline));
	bar.Add("Action View", THISBACK(OnShowActionView));
	bar.Add("Overview Preview", THISBACK(OnShowOverviewPreview));
	bar.Add("Git History", THISBACK(OnShowGitHistory));
}

void OverviewerWindow::ToolsMenu(Bar& bar) {
	bar.Add("Analyze Selection", THISBACK(OnAnalyze));
	bar.Add("Run Consistency Check", THISBACK(OnRunConsistencyCheck));
	bar.Separator();
	bar.Add("Refresh Git Status", THISBACK(OnRefreshGit));
}

void OverviewerWindow::HelpMenu(Bar& bar) {
	bar.Add("About", [] { PromptOK("Overviewer Milestone 12"); });
}
