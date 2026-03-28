#include "McpServer.h"
#include <iostream>

void McpServer::Run() {
	while(std::cin.good() && !std::cin.eof()) {
		std::string line;
		std::getline(std::cin, line);
		if(line.empty()) continue;
		ProcessRequest(String(line));
	}
}

Json McpServer::ValorizeResponse(bool success, const Value& result, const String& error) {
	Json json;
	json("success", success);
	if(!success) json("error", error);
	else json("result", result);
	return json;
}

void McpServer::ProcessRequest(const String& line) {
	Value req = ParseJSON(line);
	if(req.IsError()) {
		Cout() << ValorizeResponse(false, Value(), "Invalid JSON request").ToString() << "\n";
		return;
	}

	String method = req["method"];
	Value args = req["params"];
	Value res;

	try {
		if(method == "open_project") res = OpenProject(args);
		else if(method == "save_project") res = SaveProject(args);
		else if(method == "get_project_info") res = GetProjectInfo(args);
		else if(method == "list_entries") res = ListEntries(args);
		else if(method == "get_entry") res = GetEntry(args);
		else if(method == "set_note") res = SetNote(args);
		else if(method == "set_numeric") res = SetNumeric(args);
		else if(method == "set_flags") res = SetFlags(args);
		else if(method == "add_tag") res = AddTag(args);
		else if(method == "remove_tag") res = RemoveTag(args);
		else if(method == "add_list_item") res = AddListItem(args);
		else if(method == "update_list_item") res = UpdateListItem(args);
		else if(method == "remove_list_item") res = RemoveListItem(args);
		else if(method == "move_entry") res = MoveEntry(args);
		else if(method == "refresh_scan") res = RefreshScan(args);
		else if(method == "find_entries_with_flag") res = FindEntriesWithFlag(args);
		else if(method == "find_entries_missing_numeric") res = FindEntriesMissingNumeric(args);
		else if(method == "find_entries_by_tag") res = FindEntriesByTag(args);
		else if(method == "get_registry_tags") res = GetRegistryTags(args);
		else if(method == "get_recovery_info") res = GetRecoveryInfo(args);
		else if(method == "write_backup_now") res = WriteBackupNow(args);
		else if(method == "generate_suggestions") res = GenerateSuggestions(args);
		else if(method == "apply_suggestion") res = ApplySuggestion(args);
		else if(method == "reject_suggestion") res = RejectSuggestion(args);
		else if(method == "get_dashboard") res = GetDashboard(args);
		else if(method == "run_consistency_check") res = RunConsistencyCheck(args);
		else if(method == "list_review_items") res = ListReviewItems(args);
		else if(method == "dismiss_review_item") res = DismissReviewItem(args);
		else if(method == "get_history") res = GetHistory(args);
		else if(method == "get_recent_changes") res = GetRecentChanges(args);
		else if(method == "clear_history") res = ClearHistory(args);
		else if(method == "get_action_view") res = GetActionView(args);
		else if(method == "get_entry_score") res = GetEntryScore(args);
		else if(method == "shutdown") {
			Cout() << ValorizeResponse(true, "Shutting down").ToString() << "\n";
			exit(0);
		}
		else {
			Cout() << ValorizeResponse(false, Value(), "Unknown method: " + method).ToString() << "\n";
			return;
		}
		Cout() << ValorizeResponse(true, res).ToString() << "\n";
	} catch(const Exc& e) {
		Cout() << ValorizeResponse(false, Value(), e).ToString() << "\n";
	} catch(...) {
		Cout() << ValorizeResponse(false, Value(), "Internal error").ToString() << "\n";
	}
}

Value McpServer::OpenProject(const Value& args) {
	String path = args["path"];
	if(path.IsEmpty()) throw Exc("Path missing");
	if(!LoadFromJsonFile(project, path)) throw Exc("Failed to load project");
	project.path = path;
	DoScan();
	return "Project opened";
}

Value McpServer::SaveProject(const Value& args) {
	if(project.path.IsEmpty()) throw Exc("No project open");
	if(!StoreAsJsonFile(project, project.path)) throw Exc("Failed to save project");
	return "Project saved";
}

Value McpServer::GetProjectInfo(const Value& args) {
	ValueMap m;
	m.Add("path", project.path);
	m.Add("working_dir", project.working_dir);
	m.Add("dirty", false);
	return m;
}

Value McpServer::ListEntries(const Value& args) {
	String prefix = args["prefix"];
	bool recursive = args["recursive"];
	ValueArray arr;
	for(const String& p : current_scan) {
		if(prefix.IsEmpty() || p.StartsWith(prefix)) {
			if(!recursive && p.Mid(prefix.GetCount()).Find('/') >= 0) continue;
			arr.Add(p);
		}
	}
	return arr;
}

Value McpServer::GetEntry(const Value& args) {
	String rel_path = args["path"];
	if(!IsPathValid(rel_path)) throw Exc("Invalid path");
	
	FileMetadata effective = project.GetEffectiveMetadata(rel_path);
	const FileMetadata* m = project.metadata.FindPtr(rel_path);
	
	ValueMap res;
	res.Add("path", rel_path);
	res.Add("explicit_flags", (int)(m ? m->flags : 0));
	res.Add("priority", (m ? m->priority : 0));
	res.Add("priority_effective", effective.priority);
	res.Add("quality", (m ? m->quality : 0));
	res.Add("quality_effective", effective.quality);
	res.Add("completion", (m ? m->completion : 0));
	res.Add("completion_effective", effective.completion);
	res.Add("notes", (m ? m->notes : ""));
	
	ValueArray ctags, rtags, gtags;
	if(m) {
		for(const String& s : m->current_tags) ctags.Add(s);
		for(const String& s : m->reason_tags) rtags.Add(s);
		for(const String& s : m->gap_tags) gtags.Add(s);
	}
	res.Add("current_tags", ctags);
	res.Add("reason_tags", rtags);
	res.Add("gap_tags", gtags);
	
	auto serialize_list = [](const Vector<ListItem>& src) {
		ValueArray a;
		for(const ListItem& it : src) {
			ValueMap j;
			j.Add("text", it.text);
			j.Add("done", it.done);
			j.Add("date", it.date);
			j.Add("commit", it.commit);
			a.Add(j);
		}
		return a;
	};
	
	if(m) {
		res.Add("problems", serialize_list(m->problems));
		res.Add("tasks", serialize_list(m->tasks));
		res.Add("leads", serialize_list(m->leads));
	} else {
		res.Add("problems", ValueArray());
		res.Add("tasks", ValueArray());
		res.Add("leads", ValueArray());
	}
	
	return res;
}

Value McpServer::SetNote(const Value& args) {
	String path = args["path"];
	String note = args["note"];
	if(!IsPathValid(path)) throw Exc("Invalid path");
	project.metadata.GetAdd(path).notes = note;
	return "Note set";
}

Value McpServer::SetNumeric(const Value& args) {
	String path = args["path"];
	if(!IsPathValid(path)) throw Exc("Invalid path");
	FileMetadata& m = project.metadata.GetAdd(path);
	if(!args["quality"].IsNull()) m.quality = (int)args["quality"];
	if(!args["completion"].IsNull()) m.completion = (int)args["completion"];
	if(!args["priority"].IsNull()) m.priority = (int)args["priority"];
	return "Numeric values set";
}

Value McpServer::SetFlags(const Value& args) {
	String path = args["path"];
	if(!IsPathValid(path)) throw Exc("Invalid path");
	Value arr = args["flags"];
	uint32 bits = 0;
	if(arr.GetCount() > 0) {
		for(int i = 0; i < arr.GetCount(); i++)
			bits |= StringToFlag(arr[i]);
	}
	project.metadata.GetAdd(path).flags = bits;
	return "Flags set";
}

Value McpServer::AddTag(const Value& args) {
	String path = args["path"];
	String cat = args["category"];
	String tag = args["tag"];
	if(!IsPathValid(path)) throw Exc("Invalid path");
	FileMetadata& m = project.metadata.GetAdd(path);
	Vector<String>* v = nullptr;
	if(cat == "current") v = &m.current_tags;
	else if(cat == "reason") v = &m.reason_tags;
	else if(cat == "gap") v = &m.gap_tags;
	if(!v) throw Exc("Invalid category");
	if(FindIndex(*v, tag) < 0) v->Add(tag);
	return "Tag added";
}

Value McpServer::RemoveTag(const Value& args) {
	String path = args["path"];
	String cat = args["category"];
	String tag = args["tag"];
	if(!IsPathValid(path)) throw Exc("Invalid path");
	FileMetadata* m = project.metadata.FindPtr(path);
	if(!m) return "No metadata";
	Vector<String>* v = nullptr;
	if(cat == "current") v = &m->current_tags;
	else if(cat == "reason") v = &m->reason_tags;
	else if(cat == "gap") v = &m->gap_tags;
	if(!v) throw Exc("Invalid category");
	int idx = FindIndex(*v, tag);
	if(idx >= 0) v->Remove(idx);
	return "Tag removed";
}

Value McpServer::AddListItem(const Value& args) {
	String path = args["path"];
	String ltype = args["list_type"];
	String text = args["text"];
	if(!IsPathValid(path)) throw Exc("Invalid path");
	FileMetadata& m = project.metadata.GetAdd(path);
	Vector<ListItem>* v = nullptr;
	if(ltype == "problems") v = &m.problems;
	else if(ltype == "tasks") v = &m.tasks;
	else if(ltype == "leads") v = &m.leads;
	if(!v) throw Exc("Invalid list type");
	v->Add().text = text;
	return "Item added";
}

Value McpServer::UpdateListItem(const Value& args) {
	String path = args["path"];
	String ltype = args["list_type"];
	int idx = args["index"];
	if(!IsPathValid(path)) throw Exc("Invalid path");
	FileMetadata* m = project.metadata.FindPtr(path);
	if(!m) throw Exc("No metadata");
	Vector<ListItem>* v = nullptr;
	if(ltype == "problems") v = &m->problems;
	else if(ltype == "tasks") v = &m->tasks;
	else if(ltype == "leads") v = &m->leads;
	if(!v || idx < 0 || idx >= v->GetCount()) throw Exc("Invalid list type or index");
	ListItem& it = (*v)[idx];
	if(!args["text"].IsNull()) it.text = (String)args["text"];
	if(!args["done"].IsNull()) it.done = (bool)args["done"];
	if(!args["date"].IsNull()) it.date = (String)args["date"];
	if(!args["commit"].IsNull()) it.commit = (String)args["commit"];
	return "Item updated";
}

Value McpServer::RemoveListItem(const Value& args) {
	String path = args["path"];
	String ltype = args["list_type"];
	int idx = args["index"];
	if(!IsPathValid(path)) throw Exc("Invalid path");
	FileMetadata* m = project.metadata.FindPtr(path);
	if(!m) throw Exc("No metadata");
	Vector<ListItem>* v = nullptr;
	if(ltype == "problems") v = &m->problems;
	else if(ltype == "tasks") v = &m->tasks;
	else if(ltype == "leads") v = &m->leads;
	if(!v || idx < 0 || idx >= v->GetCount()) throw Exc("Invalid list type or index");
	v->Remove(idx);
	return "Item removed";
}

Value McpServer::MoveEntry(const Value& args) {
	String src = args["source"];
	String dst = args["destination"];
	bool review_location = args["review_wrong_location_flag"];
	
	if(!IsPathValid(src) || !IsPathValid(dst)) throw Exc("Invalid path");
	
	String abs_src = GetAbsPath(src);
	String abs_dst = GetAbsPath(dst);
	
	if(!FileExists(abs_src) && !DirectoryExists(abs_src)) throw Exc("Source does not exist");
	if(FileExists(abs_dst) || DirectoryExists(abs_dst)) throw Exc("Destination already exists");
	
	RealizeDirectory(GetFileDirectory(abs_dst));
	
	if(rename(abs_src, abs_dst) != 0) throw Exc("Filesystem move failed");
	
	VectorMap<String, FileMetadata> new_meta;
	for(int i = 0; i < project.metadata.GetCount(); i++) {
		String p = project.metadata.GetKey(i);
		FileMetadata& m = project.metadata[i];
		String new_p = p;
		if(p == src) {
			new_p = dst;
			if(review_location) m.flags &= ~FLAG_WRONG_LOCATION;
		} else if(p.StartsWith(src + "/")) {
			new_p = dst + p.Mid(src.GetCount());
		}
		new_meta.Add(new_p, pick(m));
	}
	project.metadata = pick(new_meta);
	
	DoScan();
	return "Entry moved";
}

Value McpServer::RefreshScan(const Value& args) {
	DoScan();
	return "Scan refreshed";
}

Value McpServer::FindEntriesWithFlag(const Value& args) {
	uint32 bit = StringToFlag(args["flag"]);
	ValueArray arr;
	for(int i = 0; i < project.metadata.GetCount(); i++) {
		if(project.metadata[i].flags & bit)
			arr.Add(project.metadata.GetKey(i));
	}
	return arr;
}

Value McpServer::FindEntriesMissingNumeric(const Value& args) {
	String field = args["field"];
	ValueArray arr;
	for(const String& p : current_scan) {
		const FileMetadata* m = project.metadata.FindPtr(p);
		bool missing = false;
		if(field == "quality") missing = (!m || m->quality == 0);
		else if(field == "completion") missing = (!m || m->completion == 0);
		else if(field == "priority") missing = (!m || m->priority == 0);
		else throw Exc("Invalid field");
		if(missing) arr.Add(p);
	}
	return arr;
}

Value McpServer::FindEntriesByTag(const Value& args) {
	String cat = args["category"];
	String tag = args["tag"];
	ValueArray arr;
	for(int i = 0; i < project.metadata.GetCount(); i++) {
		const FileMetadata& m = project.metadata[i];
		const Vector<String>* v = nullptr;
		if(cat == "current") v = &m.current_tags;
		else if(cat == "reason") v = &m.reason_tags;
		else if(cat == "gap") v = &m.gap_tags;
		if(v && FindIndex(*v, tag) >= 0)
			arr.Add(project.metadata.GetKey(i));
	}
	return arr;
}

Value McpServer::GetRegistryTags(const Value& args) {
	ValueMap res;
	ValueArray c, r, g;
	for(const String& s : project.known_current_tags) c.Add(s);
	for(const String& s : project.known_reason_tags) r.Add(s);
	for(const String& s : project.known_gap_tags) g.Add(s);
	res.Add("current", c);
	res.Add("reason", r);
	res.Add("gap", g);
	return res;
}

Value McpServer::GetRecoveryInfo(const Value& args) {
	String bpath = project.GetBackupPath();
	bool exists = !bpath.IsEmpty() && FileExists(bpath);
	ValueMap m;
	m.Add("has_recovery", exists);
	m.Add("recovery_path", bpath);
	m.Add("timestamp", exists ? Format(FileGetTime(bpath)) : "");
	m.Add("previous_unclean_shutdown", FileExists(ConfigFile("session.active")));
	return m;
}

Value McpServer::WriteBackupNow(const Value& args) {
	if(project.path.IsEmpty()) throw Exc("No project path set");
	if(project.WriteBackup()) return "Backup written";
	throw Exc("Failed to write backup");
}

Value McpServer::GenerateSuggestions(const Value& args) {
	String path = args["path"];
	bool recursive = args["recursive"];
	if(!IsPathValid(path)) throw Exc("Invalid path");
	
	auto analyze = [&](const String& p) {
		project.AnalyzeEntry(p);
	};

	analyze(path);
	if(recursive) {
		for(const String& p : current_scan) {
			if(p.StartsWith(path + "/") || p.StartsWith(path + "\\"))
				analyze(p);
		}
	}
	return "Suggestions generated";
}

Value McpServer::ApplySuggestion(const Value& args) {
	String path = args["path"];
	int type = args["type"];
	int category = args["category"];
	String value = args["value"];
	
	FileMetadata& m = project.metadata.GetAdd(path);
	if(type == 0) { // Tag
		Vector<String>* v = (category == 0 ? &m.current_tags : (category == 1 ? &m.reason_tags : &m.gap_tags));
		if(v && FindIndex(*v, value) < 0) v->Add(value);
	} else if(type == 1) { // Problem
		m.problems.Add().text = value;
	} else if(type == 2) { // Task
		m.tasks.Add().text = value;
	}
	
	// Dismiss
	EntrySuggestions* sug = project.suggestions.FindPtr(path);
	if(sug) {
		auto dismiss = [&](Vector<Suggestion>& v) {
			for(Suggestion& s : v) if(s.text == value) s.rejected = true;
		};
		if(type == 0) {
			if(category == 0) dismiss(sug->current_tags);
			else if(category == 1) dismiss(sug->reason_tags);
			else dismiss(sug->gap_tags);
		} else if(type == 1) dismiss(sug->problems);
		else if(type == 2) dismiss(sug->tasks);
	}
	
	return "Suggestion applied";
}

Value McpServer::RejectSuggestion(const Value& args) {
	String path = args["path"];
	int type = args["type"];
	int category = args["category"];
	String value = args["value"];
	
	EntrySuggestions* sug = project.suggestions.FindPtr(path);
	if(sug) {
		auto dismiss = [&](Vector<Suggestion>& v) {
			for(Suggestion& s : v) if(s.text == value) s.rejected = true;
		};
		if(type == 0) {
			if(category == 0) dismiss(sug->current_tags);
			else if(category == 1) dismiss(sug->reason_tags);
			else dismiss(sug->gap_tags);
		} else if(type == 1) dismiss(sug->problems);
		else if(type == 2) dismiss(sug->tasks);
	}
	return "Suggestion rejected";
}

Value McpServer::GetDashboard(const Value& args) {
	ProjectDashboard db = project.GetDashboard();
	ValueMap res;
	res.Add("total_files", db.total_files);
	res.Add("total_dirs", db.total_dirs);
	res.Add("flagged_entries", db.flagged_entries);
	res.Add("needs_review", db.needs_review);
	res.Add("missing_priority", db.missing_priority);
	res.Add("missing_completion", db.missing_completion);
	res.Add("with_notes", db.with_notes);
	res.Add("with_problems", db.with_problems);
	res.Add("with_tasks", db.with_tasks);
	res.Add("with_leads", db.with_leads);
	res.Add("suggestions_pending", db.suggestions_pending);
	ValueArray pc;
	for(int i = 0; i < 6; i++) pc.Add(db.priority_counts[i]);
	res.Add("priority_counts", pc);
	return res;
}

Value McpServer::RunConsistencyCheck(const Value& args) {
	project.RunConsistencyCheck();
	return ListReviewItems(args);
}

Value McpServer::ListReviewItems(const Value& args) {
	ValueArray arr;
	for(const auto& it : project.review_queue) {
		ValueMap m;
		m.Add("path", it.path);
		m.Add("type", it.type);
		m.Add("message", it.message);
		m.Add("severity", it.severity);
		m.Add("source", it.source);
		m.Add("dismissed", it.dismissed);
		arr.Add(m);
	}
	return arr;
}

Value McpServer::DismissReviewItem(const Value& args) {
	String path = args["path"];
	String msg = args["message"];
	if(path.IsEmpty() || msg.IsEmpty()) throw Exc("Path and message required");
	project.dismissed_review_ids.FindAdd(path + ":" + msg);
	return "Review item dismissed";
}

Value McpServer::GetHistory(const Value& args) {
	String path = args["path"];
	int limit = args["limit"];
	if(limit <= 0) limit = 100;
	
	ValueArray arr;
	int count = 0;
	for(int i = project.history.GetCount() - 1; i >= 0 && count < limit; i--) {
		const auto& e = project.history[i];
		if(path.IsEmpty() || e.path == path) {
			ValueMap m;
			m.Add("time", Format(e.time));
			m.Add("path", e.path);
			m.Add("type", e.type);
			m.Add("description", e.description);
			m.Add("source", e.source);
			arr.Add(m);
			count++;
		}
	}
	return arr;
}

Value McpServer::GetRecentChanges(const Value& args) {
	ProjectDashboard db = project.GetDashboard();
	ValueMap res;
	res.Add("recent_changes_count", db.recent_changes);
	ValueArray rm;
	for(const String& p : db.recently_modified) rm.Add(p);
	res.Add("recently_modified", rm);
	return res;
}

Value McpServer::GetActionView(const Value& args) {
	int limit = args["limit"];
	if(limit <= 0) limit = 20;
	VectorMap<String, EntryScore> view = project.GetActionView(limit);
	ValueArray arr;
	for(int i = 0; i < view.GetCount(); i++) {
		ValueMap m;
		m.Add("path", view.GetKey(i));
		m.Add("score", view[i].score);
		m.Add("explanation", Join(view[i].factors, ", "));
		arr.Add(m);
	}
	return arr;
}

Value McpServer::GetEntryScore(const Value& args) {
	String path = args["path"];
	if(path.IsEmpty()) throw Exc("Path required");
	EntryScore s = project.ComputeScore(path);
	ValueMap m;
	m.Add("score", s.score);
	ValueArray factors;
	for(const String& f : s.factors) factors.Add(f);
	m.Add("factors", factors);
	return m;
}

static void RecursiveScan(const String& dir, const String& base, Vector<String>& res) {
	for(FindFile ff(AppendFileName(dir, "*")); ff; ff.Next()) {
		String rel = ff.GetName();
		if(!base.IsEmpty()) rel = AppendFileName(base, rel);
		res.Add(rel);
		if(ff.IsFolder())
			RecursiveScan(ff.GetPath(), rel, res);
	}
}

void McpServer::DoScan() {
	current_scan.Clear();
	String root = project.working_dir;
	if(root.IsEmpty() && !project.path.IsEmpty())
		root = GetFileDirectory(project.path);
	if(!root.IsEmpty() && DirectoryExists(root))
		RecursiveScan(root, "", current_scan);
}

bool McpServer::IsPathValid(const String& path) {
	if(path.StartsWith("/") || path.StartsWith("\\") || path.Find("..") >= 0) return false;
	return true;
}

String McpServer::GetAbsPath(const String& rel_path) {
	String root = project.working_dir;
	if(root.IsEmpty() && !project.path.IsEmpty())
		root = GetFileDirectory(project.path);
	return AppendFileName(root, rel_path);
}

uint32 McpServer::StringToFlag(const String& name) {
	if(name == "TEMPORARY") return FLAG_TEMPORARY;
	if(name == "WRONG_LOCATION") return FLAG_WRONG_LOCATION;
	if(name == "WRONG_NAME") return FLAG_WRONG_NAME;
	if(name == "TOO_LARGE") return FLAG_TOO_LARGE;
	if(name == "NEEDS_REVIEW") return FLAG_NEEDS_REVIEW;
	if(name == "CONTENT_NEEDS_REVIEW") return FLAG_CONTENT_NEEDS_REVIEW;
	return 0;
}
