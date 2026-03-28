#include "McpServer.h"

void McpServer::Run() {
	while(!Cout().IsError() && !Cin().IsEof()) {
		String line = Cin().GetLine();
		if(line.IsEmpty()) continue;
		ProcessRequest(line);
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
		Cout() << ValorizeResponse(false, Value(), "Invalid JSON request") << "\n";
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
		else if(method == "shutdown") {
			Cout() << ValorizeResponse(true, "Shutting down") << "\n";
			exit(0);
		}
		else {
			Cout() << ValorizeResponse(false, Value(), "Unknown method: " + method) << "\n";
			return;
		}
		Cout() << ValorizeResponse(true, res) << "\n";
	} catch(const Exc& e) {
		Cout() << ValorizeResponse(false, Value(), e) << "\n";
	} catch(...) {
		Cout() << ValorizeResponse(false, Value(), "Internal error") << "\n";
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
	Json json;
	json("path", project.path)
	    ("working_dir", project.working_dir)
	    ("dirty", false); // We don't track dirty state properly in McpServer yet as it's meant for headless
	return json;
}

Value McpServer::ListEntries(const Value& args) {
	String prefix = args["prefix"];
	bool recursive = args["recursive"];
	JsonArray arr;
	for(const String& p : current_scan) {
		if(prefix.IsEmpty() || p.StartsWith(prefix)) {
			if(!recursive && p.Mid(prefix.GetCount()).Find('/') >= 0) continue;
			arr << p;
		}
	}
	return arr;
}

Value McpServer::GetEntry(const Value& args) {
	String rel_path = args["path"];
	if(!IsPathValid(rel_path)) throw Exc("Invalid path");
	
	FileMetadata effective = project.GetEffectiveMetadata(rel_path);
	const FileMetadata* m = project.metadata.FindPtr(rel_path);
	
	Json json;
	json("path", rel_path)
	    ("explicit_flags", (int)(m ? m->flags : 0))
	    ("priority", (m ? m->priority : 0))
	    ("priority_effective", effective.priority)
	    ("quality", (m ? m->quality : 0))
	    ("quality_effective", effective.quality)
	    ("completion", (m ? m->completion : 0))
	    ("completion_effective", effective.completion)
	    ("notes", (m ? m->notes : ""));
	
	JsonArray ctags, rtags, gtags;
	if(m) {
		for(const String& s : m->current_tags) ctags << s;
		for(const String& s : m->reason_tags) rtags << s;
		for(const String& s : m->gap_tags) gtags << s;
	}
	json("current_tags", ctags)("reason_tags", rtags)("gap_tags", gtags);
	
	auto serialize_list = [](const Vector<ListItem>& src) {
		JsonArray a;
		for(const ListItem& it : src) {
			Json j;
			j("text", it.text)("done", it.done)("date", it.date)("commit", it.commit);
			a << j;
		}
		return a;
	};
	
	if(m) {
		json("problems", serialize_list(m->problems))
		    ("tasks", serialize_list(m->tasks))
		    ("leads", serialize_list(m->leads));
	} else {
		json("problems", JsonArray())("tasks", JsonArray())("leads", JsonArray());
	}
	
	return json;
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
	JsonArray arr = args["flags"];
	uint32 bits = 0;
	for(int i = 0; i < arr.GetCount(); i++)
		bits |= StringToFlag(arr[i]);
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
	
	// Ensure parent dir of dst exists
	RealizeDirectory(GetFileDirectory(abs_dst));
	
	if(rename(abs_src, abs_dst) != 0) throw Exc("Filesystem move failed");
	
	// Update metadata
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
	JsonArray arr;
	for(int i = 0; i < project.metadata.GetCount(); i++) {
		if(project.metadata[i].flags & bit)
			arr << project.metadata.GetKey(i);
	}
	return arr;
}

Value McpServer::FindEntriesMissingNumeric(const Value& args) {
	String field = args["field"];
	JsonArray arr;
	for(const String& p : current_scan) {
		const FileMetadata* m = project.metadata.FindPtr(p);
		bool missing = false;
		if(field == "quality") missing = (!m || m->quality == 0);
		else if(field == "completion") missing = (!m || m->completion == 0);
		else if(field == "priority") missing = (!m || m->priority == 0);
		else throw Exc("Invalid field");
		if(missing) arr << p;
	}
	return arr;
}

Value McpServer::FindEntriesByTag(const Value& args) {
	String cat = args["category"];
	String tag = args["tag"];
	JsonArray arr;
	for(int i = 0; i < project.metadata.GetCount(); i++) {
		const FileMetadata& m = project.metadata[i];
		const Vector<String>* v = nullptr;
		if(cat == "current") v = &m.current_tags;
		else if(cat == "reason") v = &m.reason_tags;
		else if(cat == "gap") v = &m.gap_tags;
		if(v && FindIndex(*v, tag) >= 0)
			arr << project.metadata.GetKey(i);
	}
	return arr;
}

Value McpServer::GetRegistryTags(const Value& args) {
	Json json;
	JsonArray c, r, g;
	for(const String& s : project.known_current_tags) c << s;
	for(const String& s : project.known_reason_tags) r << s;
	for(const String& s : project.known_gap_tags) g << s;
	json("current", c)("reason", r)("gap", g);
	return json;
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
	// Further validation could check against working_dir
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
