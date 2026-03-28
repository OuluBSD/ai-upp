#include "Overviewer.h"

void PrintHelp() {
	Cout() << "Overviewer CLI Mode\n"
	       << "Usage:\n"
	       << "  Overviewer [--help]\n"
	       << "  Overviewer --create-project <file> [--dir <working_dir>]\n"
	       << "  Overviewer --open-project <file>\n"
	       << "  Overviewer --roundtrip-project <input> <output>\n"
	       << "  Overviewer --set-flag <project> <path> <flag>\n"
	       << "  Overviewer --set-priority <project> <path> <value>\n"
	       << "  Overviewer --get-entry <project> <path>\n"
	       << "  Overviewer --set-note <project> <path> <text>\n"
	       << "  Overviewer --add-tag <project> <path> <category> <tag>\n"
	       << "  Overviewer --remove-tag <project> <path> <category> <tag>\n"
	       << "  Overviewer --add-list-item <project> <path> <listtype> <text>\n"
	       << "  Overviewer --set-list-item-done <project> <path> <listtype> <index> <0|1>\n"
	       << "\nFlags: TEMPORARY, WRONG_LOCATION, WRONG_NAME, TOO_LARGE, NEEDS_REVIEW, CONTENT_NEEDS_REVIEW\n"
	       << "Categories: current, reason, gap\n"
	       << "ListTypes: problems, tasks, leads\n";
}

int CliMain(const Vector<String>& args) {
	if (args.GetCount() == 0 || args[0] == "--help") {
		PrintHelp();
		return 0;
	}

	if (args[0] == "--create-project" && args.GetCount() >= 2) {
		OverviewerProject p;
		p.path = args[1];
		if (args.GetCount() >= 4 && args[2] == "--dir")
			p.working_dir = args[3];
		String json = StoreAsJson(p);
		if (SaveFile(p.path, json)) {
			Cout() << "Project created: " << p.path << "\n";
			return 0;
		}
		Cerr() << "Failed to create project: " << p.path << "\n";
		return 1;
	}

	if (args[0] == "--open-project" && args.GetCount() >= 2) {
		String content = LoadFile(args[1]);
		if (content.IsEmpty()) {
			Cerr() << "Failed to load project: " << args[1] << "\n";
			return 1;
		}
		OverviewerProject p;
		try {
			LoadFromJson(p, content);
			Cout() << "Project loaded: " << args[1] << "\n";
			Cout() << "Working directory: " << p.working_dir << "\n";
			Cout() << "Entries: " << p.metadata.GetCount() << "\n";
			return 0;
		} catch (const Exc& e) {
			Cerr() << "Failed to parse project: " << e << "\n";
			return 1;
		}
	}

	if (args[0] == "--roundtrip-project" && args.GetCount() >= 3) {
		String content = LoadFile(args[1]);
		if (content.IsEmpty()) {
			Cerr() << "Failed to load input project: " << args[1] << "\n";
			return 1;
		}
		OverviewerProject p;
		try {
			LoadFromJson(p, content);
			String json = StoreAsJson(p);
			if (SaveFile(args[2], json)) {
				Cout() << "Project roundtripped to: " << args[2] << "\n";
				return 0;
			}
			Cerr() << "Failed to save output project: " << args[2] << "\n";
			return 1;
		} catch (const Exc& e) {
			Cerr() << "Failed to parse input project: " << e << "\n";
			return 1;
		}
	}

	if (args[0] == "--set-flag" && args.GetCount() >= 4) {
		String p_path = args[1];
		String f_path = args[2];
		String flag_name = args[3];
		
		OverviewerProject p;
		if(!LoadFromJsonFile(p, p_path)) {
			Cerr() << "Failed to load project: " << p_path << "\n";
			return 1;
		}
		
		uint32 bit = 0;
		if(flag_name == "TEMPORARY") bit = FLAG_TEMPORARY;
		else if(flag_name == "WRONG_LOCATION") bit = FLAG_WRONG_LOCATION;
		else if(flag_name == "WRONG_NAME") bit = FLAG_WRONG_NAME;
		else if(flag_name == "TOO_LARGE") bit = FLAG_TOO_LARGE;
		else if(flag_name == "NEEDS_REVIEW") bit = FLAG_NEEDS_REVIEW;
		else if(flag_name == "CONTENT_NEEDS_REVIEW") bit = FLAG_CONTENT_NEEDS_REVIEW;
		else {
			Cerr() << "Invalid flag name: " << flag_name << "\n";
			return 1;
		}
		
		p.metadata.GetAdd(f_path).flags |= bit;
		if(StoreAsJsonFile(p, p_path)) {
			Cout() << "Flag " << flag_name << " set for " << f_path << "\n";
			return 0;
		}
		return 1;
	}

	if (args[0] == "--set-priority" && args.GetCount() >= 4) {
		String p_path = args[1];
		String f_path = args[2];
		int val = ScanInt(args[3]);
		
		OverviewerProject p;
		if(!LoadFromJsonFile(p, p_path)) {
			Cerr() << "Failed to load project: " << p_path << "\n";
			return 1;
		}
		
		p.metadata.GetAdd(f_path).priority = val;
		if(StoreAsJsonFile(p, p_path)) {
			Cout() << "Priority set to " << val << " for " << f_path << "\n";
			return 0;
		}
		return 1;
	}

	if (args[0] == "--get-entry" && args.GetCount() >= 3) {
		String p_path = args[1];
		String f_path = args[2];
		
		OverviewerProject p;
		if(!LoadFromJsonFile(p, p_path)) {
			Cerr() << "Failed to load project: " << p_path << "\n";
			return 1;
		}
		
		const FileMetadata* m = p.metadata.FindPtr(f_path);
		if(m) {
			Cout() << "Path: " << f_path << "\n";
			Cout() << "Flags: " << (int)m->flags << "\n";
			Cout() << "Priority: " << m->priority << "\n";
			Cout() << "Quality: " << m->quality << "\n";
			Cout() << "Completion: " << m->completion << "\n";
			Cout() << "Note: " << m->notes << "\n";
			auto print_tags = [](const char* title, const Vector<String>& tags) {
				Cout() << title << ": " << Join(tags, ", ") << "\n";
			};
			print_tags("Current Tags", m->current_tags);
			print_tags("Reason Tags", m->reason_tags);
			print_tags("Gap Tags", m->gap_tags);
			auto print_list = [](const char* title, const Vector<ListItem>& items) {
				Cout() << title << ":\n";
				for(int i = 0; i < items.GetCount(); i++)
					Cout() << "  [" << i << "] " << (items[i].done ? "[X] " : "[ ] ") << items[i].text << "\n";
			};
			print_list("Problems", m->problems);
			print_list("Tasks", m->tasks);
			print_list("Leads", m->leads);
			return 0;
		} else {
			Cerr() << "No metadata for path: " << f_path << "\n";
			return 1;
		}
	}

	if (args[0] == "--set-note" && args.GetCount() >= 4) {
		String p_path = args[1];
		String f_path = args[2];
		String note = args[3];
		OverviewerProject p;
		if(!LoadFromJsonFile(p, p_path)) return 1;
		p.metadata.GetAdd(f_path).notes = note;
		return StoreAsJsonFile(p, p_path) ? 0 : 1;
	}

	if (args[0] == "--add-tag" && args.GetCount() >= 5) {
		String p_path = args[1];
		String f_path = args[2];
		String cat = args[3];
		String tag = args[4];
		OverviewerProject p;
		if(!LoadFromJsonFile(p, p_path)) return 1;
		FileMetadata& m = p.metadata.GetAdd(f_path);
		Vector<String>* target = nullptr;
		if(cat == "current") target = &m.current_tags;
		else if(cat == "reason") target = &m.reason_tags;
		else if(cat == "gap") target = &m.gap_tags;
		if(!target) return 1;
		if(FindIndex(*target, tag) < 0) target->Add(tag);
		return StoreAsJsonFile(p, p_path) ? 0 : 1;
	}

	if (args[0] == "--remove-tag" && args.GetCount() >= 5) {
		String p_path = args[1];
		String f_path = args[2];
		String cat = args[3];
		String tag = args[4];
		OverviewerProject p;
		if(!LoadFromJsonFile(p, p_path)) return 1;
		FileMetadata& m = p.metadata.GetAdd(f_path);
		Vector<String>* target = nullptr;
		if(cat == "current") target = &m.current_tags;
		else if(cat == "reason") target = &m.reason_tags;
		else if(cat == "gap") target = &m.gap_tags;
		if(!target) return 1;
		int idx = FindIndex(*target, tag);
		if(idx >= 0) target->Remove(idx);
		return StoreAsJsonFile(p, p_path) ? 0 : 1;
	}

	if (args[0] == "--add-list-item" && args.GetCount() >= 5) {
		String p_path = args[1];
		String f_path = args[2];
		String ltype = args[3];
		String text = args[4];
		OverviewerProject p;
		if(!LoadFromJsonFile(p, p_path)) return 1;
		FileMetadata& m = p.metadata.GetAdd(f_path);
		Vector<ListItem>* target = nullptr;
		if(ltype == "problems") target = &m.problems;
		else if(ltype == "tasks") target = &m.tasks;
		else if(ltype == "leads") target = &m.leads;
		if(!target) return 1;
		target->Add().text = text;
		return StoreAsJsonFile(p, p_path) ? 0 : 1;
	}

	if (args[0] == "--set-list-item-done" && args.GetCount() >= 6) {
		String p_path = args[1];
		String f_path = args[2];
		String ltype = args[3];
		int idx = ScanInt(args[4]);
		bool done = ScanInt(args[5]) != 0;
		OverviewerProject p;
		if(!LoadFromJsonFile(p, p_path)) return 1;
		FileMetadata& m = p.metadata.GetAdd(f_path);
		Vector<ListItem>* target = nullptr;
		if(ltype == "problems") target = &m.problems;
		else if(ltype == "tasks") target = &m.tasks;
		else if(ltype == "leads") target = &m.leads;
		if(!target || idx < 0 || idx >= target->GetCount()) return 1;
		(*target)[idx].done = done;
		return StoreAsJsonFile(p, p_path) ? 0 : 1;
	}

	Cerr() << "Unknown arguments. Use --help for usage.\n";
	return 1;
}

GUI_APP_MAIN {
	const Vector<String>& args = CommandLine();
	if (args.GetCount() > 0 && args[0].StartsWith("--")) {
		SetExitCode(CliMain(args));
		return;
	}

	OverviewerWindow().Run();
}
