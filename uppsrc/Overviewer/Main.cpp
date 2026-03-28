#include "Overviewer.h"
#include "McpServer.h"

void PrintHelp() {
	Cout() << "Overviewer CLI Mode\n"
	       << "Usage:\n"
	       << "  Overviewer [--help]\n"
	       << "  Overviewer --mcp\n"
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
	       << "  Overviewer --batch-set-priority <project> <path> <value> [--recursive]\n"
	       << "  Overviewer --batch-add-flag <project> <path> <flag> [--recursive]\n"
	       << "  Overviewer --batch-add-tag <project> <path> <category> <tag> [--recursive]\n"
	       << "  Overviewer --write-backup <project>\n"
	       << "  Overviewer --show-recovery-info <project>\n"
	       << "  Overviewer --generate-suggestions <project> <path> [--recursive]\n"
	       << "  Overviewer --apply-suggestion <project> <path> <type_id> <category_id> <value>\n"
	       << "  Overviewer --get-dashboard <project>\n"
	       << "  Overviewer --run-consistency-check <project>\n"
	       << "  Overviewer --list-review-items <project>\n"
	       << "  Overviewer --dismiss-review-item <project> <path> <message>\n"
	       << "  Overviewer --get-history <project> [path]\n"
	       << "  Overviewer --get-recent-changes <project>\n"
	       << "  Overviewer --clear-history <project>\n"
	       << "  Overviewer --generate-overview <project> [--markdown]\n"
	       << "  Overviewer --generate-overview-subtree <project> <path> [--markdown]\n"
	       << "  Overviewer --generate-overview-entry <project> <path> [--markdown]\n"
	       << "  Overviewer --export-overview <project> <output_path>\n"
	       << "  Overviewer --get-git-info <project>\n"
	       << "  Overviewer --refresh-git-status <project>\n"
	       << "  Overviewer --get-entry-git-status <project> <path>\n"
	       << "  Overviewer --get-entry-commits <project> <path> [limit]\n"
	       << "  Overviewer --link-list-item-commit <project> <path> <listtype> <index> <commit>\n"
	       << "\nFlags: TEMPORARY, WRONG_LOCATION, WRONG_NAME, TOO_LARGE, NEEDS_REVIEW, CONTENT_NEEDS_REVIEW\n"
	       << "Categories: current, reason, gap\n"
	       << "ListTypes: problems, tasks, leads\n";
}

static Vector<String> GetAffectedPaths(const OverviewerProject& project, const String& start_path, bool recursive) {
	Vector<String> res;
	res.Add(start_path);
	if(recursive) {
		for(int i = 0; i < project.metadata.GetCount(); i++) {
			String p = project.metadata.GetKey(i);
			if(p.StartsWith(start_path + "/") || p.StartsWith(start_path + "\\"))
				res.Add(p);
		}
	}
	return res;
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

	if (args[0] == "--set-flag" && args.GetCount() >= 4) {
		String p_path = args[1];
		String f_path = args[2];
		String flag_name = args[3];
		OverviewerProject p;
		if(!LoadFromJsonFile(p, p_path)) return 1;
		uint32 bit = 0;
		if(flag_name == "TEMPORARY") bit = FLAG_TEMPORARY;
		else if(flag_name == "WRONG_LOCATION") bit = FLAG_WRONG_LOCATION;
		else if(flag_name == "WRONG_NAME") bit = FLAG_WRONG_NAME;
		else if(flag_name == "TOO_LARGE") bit = FLAG_TOO_LARGE;
		else if(flag_name == "NEEDS_REVIEW") bit = FLAG_NEEDS_REVIEW;
		else if(flag_name == "CONTENT_NEEDS_REVIEW") bit = FLAG_CONTENT_NEEDS_REVIEW;
		else return 1;
		p.metadata.GetAdd(f_path).flags |= bit;
		p.LogEvent(f_path, "set_flags", "Flag set via CLI: " + flag_name);
		return StoreAsJsonFile(p, p_path) ? 0 : 1;
	}

	if (args[0] == "--get-entry" && args.GetCount() >= 3) {
		String p_path = args[1];
		String f_path = args[2];
		OverviewerProject p;
		if(!LoadFromJsonFile(p, p_path)) return 1;
		FileMetadata effective = p.GetEffectiveMetadata(f_path);
		const FileMetadata* m = p.metadata.FindPtr(f_path);
		if(m || effective.priority != 0) {
			Cout() << "Path: " << f_path << "\n";
			Cout() << "Flags: " << (int)(m ? m->flags : 0) << "\n";
			Cout() << "Priority: " << (m ? m->priority : 0) << " (effective: " << effective.priority << ")\n";
			Cout() << "Quality: " << (m ? m->quality : 0) << " (effective: " << effective.quality << ")\n";
			Cout() << "Completion: " << (m ? m->completion : 0) << " (effective: " << effective.completion << ")\n";
			Cout() << "Note: " << (m ? m->notes : "") << "\n";
			if(m) {
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
			}
			return 0;
		}
		return 1;
	}

	if (args[0] == "--get-dashboard" && args.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, args[1])) return 1;
		ProjectDashboard db = p.GetDashboard();
		Cout() << "Dashboard for " << args[1] << ":\n"
		       << "  Total Files: " << db.total_files << "\n"
		       << "  Total Dirs: " << db.total_dirs << "\n"
		       << "  Flagged: " << db.flagged_entries << "\n"
		       << "  Suggestions Pending: " << db.suggestions_pending << "\n";
		return 0;
	}

	if (args[0] == "--get-history" && args.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, args[1])) return 1;
		String filter_path = args.GetCount() >= 3 ? args[2] : "";
		for(int i = p.history.GetCount() - 1; i >= 0; i--) {
			const auto& e = p.history[i];
			if(filter_path.IsEmpty() || e.path == filter_path)
				Cout() << Format(e.time) << " | " << e.path << " | " << e.type << " | " << e.description << "\n";
		}
		return 0;
	}

	if (args[0] == "--generate-overview" && args.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, args[1])) return 1;
		OverviewOptions opt;
		opt.markdown_output = args.GetCount() >= 3 && args[2] == "--markdown";
		Cout() << OverviewGenerator(p).GenerateProject(opt);
		return 0;
	}

	if (args[0] == "--get-git-info" && args.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, args[1])) return 1;
		p.RefreshGit();
		Cout() << "Repo detected: " << (p.git.repo_detected ? "YES" : "NO") << "\n";
		if(p.git.repo_detected) {
			Cout() << "Root: " << p.git.repo_root << "\n";
			Cout() << "Branch: " << p.git.branch << "\n";
			Cout() << "HEAD: " << p.git.head_hash << "\n";
		}
		return 0;
	}

	if (args[0] == "--get-entry-git-status" && args.GetCount() >= 3) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, args[1])) return 1;
		p.RefreshGit();
		int s = p.git.GetStatus(args[2]);
		Cout() << "Status for " << args[2] << ": " << s << "\n";
		return 0;
	}

	if (args[0] == "--get-entry-commits" && args.GetCount() >= 3) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, args[1])) return 1;
		p.RefreshGit();
		int limit = args.GetCount() >= 4 ? ScanInt(args[3]) : 10;
		Vector<GitCommit> cs = p.git.GetHistory(p.working_dir, args[2], limit);
		for(const auto& c : cs)
			Cout() << c.hash << " | " << c.author << " | " << c.subject << "\n";
		return 0;
	}

	if (args[0] == "--link-list-item-commit" && args.GetCount() >= 6) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, args[1])) return 1;
		p.path = args[1];
		String fpath = args[2];
		String ltype = args[3];
		int idx = ScanInt(args[4]);
		String commit = args[5];
		
		FileMetadata& m = p.metadata.GetAdd(fpath);
		Vector<ListItem>* target = (ltype == "problems" ? &m.problems : (ltype == "tasks" ? &m.tasks : (ltype == "leads" ? &m.leads : nullptr)));
		if(!target || idx < 0 || idx >= target->GetCount()) return 1;
		(*target)[idx].commit = commit;
		p.LogEvent(fpath, "link_commit", "Linked commit " + commit + " to " + ltype + "[" + AsString(idx) + "]");
		return StoreAsJsonFile(p, p.path) ? 0 : 1;
	}

	Cerr() << "Unknown arguments. Use --help for usage.\n";
	return 1;
}

GUI_APP_MAIN {
	const Vector<String>& args = CommandLine();
	if (args.GetCount() > 0 && args[0] == "--mcp") {
		OverviewerProject p;
		McpServer(p).Run();
		return;
	}
	if (args.GetCount() > 0 && args[0].StartsWith("--")) {
		SetExitCode(CliMain(args));
		return;
	}

	OverviewerWindow w;
	w.LoadLayout();
	w.CheckRecovery();
	w.MarkSession(true);
	w.Run();
}
