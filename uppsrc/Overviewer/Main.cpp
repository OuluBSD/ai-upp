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
	       << "  Overviewer --get-sessions <project>\n"
	       << "  Overviewer --get-history-by-actor <project> <actor_id>\n"
	       << "  Overviewer --get-actor-summary <project>\n"
	       << "  Overviewer --create-scenario <project> <name>\n"
	       << "  Overviewer --activate-scenario <project> <id>\n"
	       << "  Overviewer --deactivate-scenario <project>\n"
	       << "  Overviewer --list-scenarios <project>\n"
	       << "  Overviewer --delete-scenario <project> <id>\n"
	       << "  Overviewer --compare-scenario <project>\n"
	       << "  Overviewer --apply-scenario <project> [id]\n"
	       << "\nOptions:\n"
	       << "  --actor <id> : specify actor id for the CLI run\n"
	       << "\nFlags: TEMPORARY, WRONG_LOCATION, WRONG_NAME, TOO_LARGE, NEEDS_REVIEW, CONTENT_NEEDS_REVIEW\n"
	       << "Categories: current, reason, gap\n"
	       << "ListTypes: problems, tasks, leads\n";
}

int CliMain(Vector<String>& args) {
	if (args.GetCount() == 0 || args[0] == "--help") {
		PrintHelp();
		return 0;
	}

	String actor_id = "cli";
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--actor" && i + 1 < args.GetCount()) {
			actor_id = args[i+1];
			args.Remove(i, 2);
			break;
		}
	}

	auto load_p = [&](OverviewerProject& p, const String& path) {
		if(!LoadFromJsonFile(p, path)) return false;
		p.path = path;
		p.StartSession(actor_id, "cli");
		return true;
	};

	if (args[0] == "--create-project" && args.GetCount() >= 2) {
		OverviewerProject p;
		p.path = args[1];
		if (args.GetCount() >= 4 && args[2] == "--dir")
			p.working_dir = args[3];
		p.StartSession(actor_id, "cli");
		String json = StoreAsJson(p);
		if (SaveFile(p.path, json)) {
			Cout() << "Project created: " << p.path << "\n";
			return 0;
		}
		Cerr() << "Failed to create project: " << p.path << "\n";
		return 1;
	}

	if (args[0] == "--set-flag" && args.GetCount() >= 4) {
		String p_path = args[1];
		String f_path = args[2];
		String flag_name = args[3];
		OverviewerProject p;
		if(!load_p(p, p_path)) return 1;
		uint32 bit = 0;
		if(flag_name == "TEMPORARY") bit = FLAG_TEMPORARY;
		else if(flag_name == "WRONG_LOCATION") bit = FLAG_WRONG_LOCATION;
		else if(flag_name == "WRONG_NAME") bit = FLAG_WRONG_NAME;
		else if(flag_name == "TOO_LARGE") bit = FLAG_TOO_LARGE;
		else if(flag_name == "NEEDS_REVIEW") bit = FLAG_NEEDS_REVIEW;
		else if(flag_name == "CONTENT_NEEDS_REVIEW") bit = FLAG_CONTENT_NEEDS_REVIEW;
		else return 1;
		p.GetMetadataWrite(f_path).flags |= bit;
		p.LogEvent(f_path, "set_flags", "Flag set via CLI: " + flag_name);
		return StoreAsJsonFile(p, p_path) ? 0 : 1;
	}

	if (args[0] == "--set-priority" && args.GetCount() >= 4) {
		OverviewerProject p;
		if(!load_p(p, args[1])) return 1;
		p.GetMetadataWrite(args[2]).priority = ScanInt(args[3]);
		p.LogEvent(args[2], "set_priority", "Priority set to " + args[3]);
		return StoreAsJsonFile(p, args[1]) ? 0 : 1;
	}

	if (args[0] == "--set-note" && args.GetCount() >= 4) {
		OverviewerProject p;
		if(!load_p(p, args[1])) return 1;
		p.GetMetadataWrite(args[2]).notes = args[3];
		p.LogEvent(args[2], "set_note", "Note modified via CLI");
		return StoreAsJsonFile(p, args[1]) ? 0 : 1;
	}

	if (args[0] == "--add-tag" && args.GetCount() >= 5) {
		OverviewerProject p;
		if(!load_p(p, args[1])) return 1;
		String fpath = args[2], cat = args[3], tag = args[4];
		FileMetadata& m = p.GetMetadataWrite(fpath);
		Vector<String>* v = (cat == "current" ? &m.current_tags : (cat == "reason" ? &m.reason_tags : (cat == "gap" ? &m.gap_tags : nullptr)));
		if(!v) return 1;
		if(FindIndex(*v, tag) < 0) v->Add(tag);
		p.LogEvent(fpath, "add_tag", "Tag added: " + tag);
		return StoreAsJsonFile(p, args[1]) ? 0 : 1;
	}

	if (args[0] == "--add-list-item" && args.GetCount() >= 5) {
		OverviewerProject p;
		if(!load_p(p, args[1])) return 1;
		String fpath = args[2], ltype = args[3], text = args[4];
		FileMetadata& m = p.GetMetadataWrite(fpath);
		Vector<ListItem>* target = (ltype == "problems" ? &m.problems : (ltype == "tasks" ? &m.tasks : (ltype == "leads" ? &m.leads : nullptr)));
		if(!target) return 1;
		target->Add().text = text;
		p.LogEvent(fpath, "add_list_item", "List item added to " + ltype);
		return StoreAsJsonFile(p, args[1]) ? 0 : 1;
	}

	if (args[0] == "--get-entry" && args.GetCount() >= 3) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, args[1])) return 1;
		FileMetadata effective = p.GetEffectiveMetadata(args[2]);
		const FileMetadata* m = p.metadata.FindPtr(args[2]);
		Cout() << "Path: " << args[2] << "\n";
		Cout() << "Flags: " << (int)(m ? m->flags : 0) << "\n";
		Cout() << "Priority: " << (m ? m->priority : 0) << " (effective: " << effective.priority << ")\n";
		Cout() << "Quality: " << (m ? m->quality : 0) << " (effective: " << effective.quality << ")\n";
		Cout() << "Completion: " << (m ? m->completion : 0) << " (effective: " << effective.completion << ")\n";
		Cout() << "Note: " << (m ? m->notes : "") << "\n";
		return 0;
	}

	if (args[0] == "--get-dashboard" && args.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, args[1])) return 1;
		ProjectDashboard db = p.GetDashboard();
		Cout() << "Dashboard: " << db.total_files << " files, " << db.flagged_entries << " flagged.\n";
		return 0;
	}

	if (args[0] == "--get-history" && args.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, args[1])) return 1;
		for(int i = p.history.GetCount() - 1; i >= 0; i--) {
			const auto& e = p.history[i];
			Cout() << Format(e.time) << " | " << e.actor_id << " | " << e.path << " | " << e.type << " | " << e.description << "\n";
		}
		return 0;
	}

	if (args[0] == "--create-scenario" && args.GetCount() >= 3) {
		OverviewerProject p;
		if(!load_p(p, args[1])) return 1;
		String id = p.CreateScenario(args[2]);
		Cout() << "Scenario created: " << id << "\n";
		return StoreAsJsonFile(p, args[1]) ? 0 : 1;
	}

	if (args[0] == "--activate-scenario" && args.GetCount() >= 3) {
		OverviewerProject p;
		if(!load_p(p, args[1])) return 1;
		p.ActivateScenario(args[2]);
		return StoreAsJsonFile(p, args[1]) ? 0 : 1;
	}

	if (args[0] == "--deactivate-scenario" && args.GetCount() >= 2) {
		OverviewerProject p;
		if(!load_p(p, args[1])) return 1;
		p.DeactivateScenario();
		return StoreAsJsonFile(p, args[1]) ? 0 : 1;
	}

	if (args[0] == "--list-scenarios" && args.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, args[1])) return 1;
		for(int i = 0; i < p.scenarios.GetCount(); i++)
			Cout() << p.scenarios.GetKey(i) << " | " << p.scenarios[i].name << (p.active_scenario_id == p.scenarios.GetKey(i) ? " [ACTIVE]" : "") << "\n";
		return 0;
	}

	if (args[0] == "--delete-scenario" && args.GetCount() >= 3) {
		OverviewerProject p;
		if(!load_p(p, args[1])) return 1;
		int idx = p.scenarios.Find(args[2]);
		if(idx >= 0) {
			if(p.active_scenario_id == args[2]) p.active_scenario_id = "";
			p.scenarios.Remove(idx);
			return StoreAsJsonFile(p, args[1]) ? 0 : 1;
		}
		return 1;
	}

	if (args[0] == "--compare-scenario" && args.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, args[1])) return 1;
		if(p.active_scenario_id.IsEmpty()) { Cerr() << "No active scenario\n"; return 1; }
		int idx = p.scenarios.Find(p.active_scenario_id);
		Scenario& s = p.scenarios[idx];
		Cout() << "Comparison for scenario: " << s.name << "\n";
		for(int i = 0; i < s.metadata_delta.GetCount(); i++)
			Cout() << "  " << s.metadata_delta.GetKey(i) << " : Modified\n";
		return 0;
	}

	if (args[0] == "--apply-scenario" && args.GetCount() >= 2) {
		OverviewerProject p;
		if(!load_p(p, args[1])) return 1;
		String id = args.GetCount() >= 3 ? args[2] : p.active_scenario_id;
		if(id.IsEmpty()) return 1;
		p.ApplyScenario(id);
		return StoreAsJsonFile(p, args[1]) ? 0 : 1;
	}

	if (args[0] == "--generate-overview" && args.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, args[1])) return 1;
		OverviewOptions opt;
		opt.markdown_output = args.GetCount() >= 3 && args[2] == "--markdown";
		Cout() << OverviewGenerator(p).GenerateProject(opt);
		return 0;
	}

	if (args[0] == "--link-list-item-commit" && args.GetCount() >= 6) {
		OverviewerProject p;
		if(!load_p(p, args[1])) return 1;
		String fpath = args[2], ltype = args[3], commit = args[5];
		int idx = ScanInt(args[4]);
		FileMetadata& m = p.GetMetadataWrite(fpath);
		Vector<ListItem>* target = (ltype == "problems" ? &m.problems : (ltype == "tasks" ? &m.tasks : (ltype == "leads" ? &m.leads : nullptr)));
		if(!target || idx < 0 || idx >= target->GetCount()) return 1;
		(*target)[idx].commit = commit;
		p.LogEvent(fpath, "link_commit", "Linked commit " + commit + " to " + ltype + "[" + AsString(idx) + "]");
		return StoreAsJsonFile(p, args[1]) ? 0 : 1;
	}

	Cerr() << "Unknown arguments. Use --help for usage.\n";
	return 1;
}

GUI_APP_MAIN {
	const Vector<String>& cmdline = CommandLine();
	Vector<String> args;
	for(const String& s : cmdline) args.Add(s);

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
