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
	       << "  Overviewer --create-decision <project> <title>\n"
	       << "  Overviewer --set-decision-status <project> <id> <status>\n"
	       << "  Overviewer --list-decisions <project>\n"
	       << "  Overviewer --get-decision <project> <id>\n"
	       << "  Overviewer --link-decision-entry <project> <id> <path>\n"
	       << "  Overviewer --link-decision-scenario <project> <id> <scenario_id>\n"
	       << "  Overviewer --add-comment <project> <text> [--path <entry_path>] [--decision <decision_id>]\n"
	       << "  Overviewer --list-comments <project>\n"
	       << "  Overviewer --get-comments-entry <project> <path>\n"
	       << "  Overviewer --generate-insights <project>\n"
	       << "  Overviewer --list-insights <project>\n"
	       << "  Overviewer --dismiss-insight <project> <id>\n"
	       << "  Overviewer --get-usage-summary <project>\n"
	       << "  Overviewer --get-friction <project>\n"
	       << "\nOptions:\n"
	       << "  --actor <id> : specify actor id for the CLI run\n"
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

	String actor_id = "cli";
	Vector<String> filtered_args;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--actor" && i + 1 < args.GetCount()) {
			actor_id = args[i+1];
			i++;
		} else {
			filtered_args.Add(args[i]);
		}
	}
	
	const Vector<String>& rargs = filtered_args;
	if (rargs.GetCount() == 0) return 0;

	auto load_p = [&](OverviewerProject& p, const String& path) {
		if(!LoadFromJsonFile(p, path)) return false;
		p.path = path;
		p.StartSession(actor_id, "cli");
		p.DoScan();
		return true;
	};

	if (rargs[0] == "--create-project" && rargs.GetCount() >= 2) {
		OverviewerProject p;
		p.path = rargs[1];
		if (rargs.GetCount() >= 4 && rargs[2] == "--dir")
			p.working_dir = rargs[3];
		p.StartSession(actor_id, "cli");
		p.DoScan();
		p.RecordUsage("create_project", p.path);
		String json = StoreAsJson(p);
		if (SaveFile(p.path, json)) {
			Cout() << "Project created: " << p.path << "\n";
			return 0;
		}
		Cerr() << "Failed to create project: " << p.path << "\n";
		return 1;
	}

	if (rargs[0] == "--set-flag" && rargs.GetCount() >= 4) {
		String p_path = rargs[1];
		String f_path = rargs[2];
		String flag_name = rargs[3];
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
		p.RecordUsage("set_flags_cli", f_path);
		return StoreAsJsonFile(p, p_path) ? 0 : 1;
	}

	if (rargs[0] == "--set-priority" && rargs.GetCount() >= 4) {
		OverviewerProject p;
		if(!load_p(p, rargs[1])) return 1;
		p.GetMetadataWrite(rargs[2]).priority = ScanInt(rargs[3]);
		p.LogEvent(rargs[2], "set_priority", "Priority set to " + rargs[3]);
		p.RecordUsage("set_priority_cli", rargs[2]);
		return StoreAsJsonFile(p, args[1]) ? 0 : 1;
	}

	if (rargs[0] == "--add-tag" && rargs.GetCount() >= 5) {
		OverviewerProject p;
		if(!load_p(p, rargs[1])) return 1;
		String fpath = rargs[2], cat = rargs[3], tag = rargs[4];
		FileMetadata& m = p.GetMetadataWrite(fpath);
		Vector<String>* v = (cat == "current" ? &m.current_tags : (cat == "reason" ? &m.reason_tags : (cat == "gap" ? &m.gap_tags : nullptr)));
		if(!v) return 1;
		if(FindIndex(*v, tag) < 0) v->Add(tag);
		p.LogEvent(fpath, "add_tag", "Tag added: " + tag);
		p.RecordUsage("add_tag_cli", fpath);
		return StoreAsJsonFile(p, rargs[1]) ? 0 : 1;
	}

	if (rargs[0] == "--get-entry" && rargs.GetCount() >= 3) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, rargs[1])) return 1;
		FileMetadata effective = p.GetEffectiveMetadata(rargs[2]);
		const FileMetadata* m = p.metadata.FindPtr(rargs[2]);
		Cout() << "Path: " << rargs[2] << "\n";
		Cout() << "Flags: " << (int)(m ? m->flags : 0) << "\n";
		Cout() << "Priority: " << (m ? m->priority : 0) << " (effective: " << effective.priority << ")\n";
		Cout() << "Note: " << (m ? m->notes : "") << "\n";
		return 0;
	}

	if (rargs[0] == "--get-dashboard" && rargs.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, rargs[1])) return 1;
		ProjectDashboard db = p.GetDashboard();
		Cout() << "Dashboard: " << db.total_files << " files, " << db.flagged_entries << " flagged, " << db.active_insights << " active insights.\n";
		return 0;
	}

	if (rargs[0] == "--get-history" && rargs.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, rargs[1])) return 1;
		for(int i = p.history.GetCount() - 1; i >= 0; i--) {
			const auto& e = p.history[i];
			Cout() << Upp::Format(e.time) << " | " << e.actor_id << " | " << e.path << " | " << e.type << " | " << e.description << "\n";
		}
		return 0;
	}

	if (rargs[0] == "--create-scenario" && rargs.GetCount() >= 3) {
		OverviewerProject p;
		if(!load_p(p, rargs[1])) return 1;
		String id = p.CreateScenario(rargs[2]);
		Cout() << "Scenario created: " << id << "\n";
		return StoreAsJsonFile(p, rargs[1]) ? 0 : 1;
	}

	if (rargs[0] == "--activate-scenario" && rargs.GetCount() >= 3) {
		OverviewerProject p;
		if(!load_p(p, rargs[1])) return 1;
		p.ActivateScenario(rargs[2]);
		return StoreAsJsonFile(p, rargs[1]) ? 0 : 1;
	}

	if (rargs[0] == "--create-decision" && rargs.GetCount() >= 3) {
		OverviewerProject p;
		if(!load_p(p, rargs[1])) return 1;
		String id = p.CreateDecision(rargs[2]);
		Cout() << "Decision created: " << id << "\n";
		return StoreAsJsonFile(p, rargs[1]) ? 0 : 1;
	}

	if (rargs[0] == "--set-decision-status" && rargs.GetCount() >= 4) {
		OverviewerProject p;
		if(!load_p(p, rargs[1])) return 1;
		p.UpdateDecision(rargs[2], "", rargs[3]);
		return StoreAsJsonFile(p, rargs[1]) ? 0 : 1;
	}

	if (rargs[0] == "--add-comment" && rargs.GetCount() >= 3) {
		OverviewerProject p;
		if(!load_p(p, rargs[1])) return 1;
		String text = rargs[2];
		String path = "", dec = "";
		for(int i = 3; i < rargs.GetCount(); i++) {
			if(rargs[i] == "--path" && i + 1 < rargs.GetCount()) path = rargs[++i];
			else if(rargs[i] == "--decision" && i + 1 < rargs.GetCount()) dec = rargs[++i];
		}
		p.AddComment(text, path, dec);
		return StoreAsJsonFile(p, rargs[1]) ? 0 : 1;
	}

	if (rargs[0] == "--generate-insights" && rargs.GetCount() >= 2) {
		OverviewerProject p;
		if(!load_p(p, rargs[1])) return 1;
		p.GenerateInsights();
		return StoreAsJsonFile(p, rargs[1]) ? 0 : 1;
	}

	if (rargs[0] == "--list-insights" && rargs.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, rargs[1])) return 1;
		for(const auto& ins : p.insights) {
			if(!ins.dismissed)
				Cout() << ins.id << " | [" << ins.severity << "] " << ins.type << " | " << ins.title << " | " << ins.description << "\n";
		}
		return 0;
	}

	if (rargs[0] == "--get-usage-summary" && rargs.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, rargs[1])) return 1;
		UsageSummary s = UsageTracker::GetSummary(p);
		Cout() << "Total Actions: " << s.total_actions << "\n";
		Cout() << "Total Sessions: " << s.sessions_count << "\n";
		Cout() << "Top Actions:\n";
		for(int i = 0; i < s.top_actions.GetCount() && i < 5; i++)
			Cout() << "  " << s.top_actions.GetKey(i) << ": " << s.top_actions[i] << "\n";
		Cout() << "Unused Features: " << Join(s.unused_features, ", ") << "\n";
		return 0;
	}

	if (rargs[0] == "--get-friction" && rargs.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, rargs[1])) return 1;
		Vector<FrictionSignal> friction = UsageTracker::GetFriction(p);
		if(friction.IsEmpty()) { Cout() << "No friction detected.\n"; }
		else {
			for(const auto& sig : friction)
				Cout() << "[" << sig.severity << "] " << sig.type << " | " << sig.description << "\n";
		}
		return 0;
	}

	if (rargs[0] == "--generate-overview" && rargs.GetCount() >= 2) {
		OverviewerProject p;
		if(!LoadFromJsonFile(p, rargs[1])) return 1;
		OverviewOptions opt;
		opt.markdown_output = rargs.GetCount() >= 3 && rargs[2] == "--markdown";
		Cout() << OverviewGenerator(p).GenerateProject(opt);
		p.RecordUsage("generate_overview_cli", "");
		StoreAsJsonFile(p, rargs[1]);
		return 0;
	}

	Cerr() << "Unknown arguments. Use --help for usage.\n";
	return 1;
}

GUI_APP_MAIN {
	const Vector<String>& cmdline = CommandLine();
	
	// Check for MCP first before CommandLineArguments consumes it
	bool mcp = false;
	for(const String& s : cmdline) {
		if(s == "--mcp" || s == "-m") {
			mcp = true;
			break;
		}
	}
	
	if (mcp) {
		OverviewerProject p;
		McpServer(p).Run();
		return;
	}

	String project_file;
	for(const String& s : cmdline) {
		if(s.StartsWith("-")) {
			SetExitCode(CliMain(cmdline));
			return;
		}
		if(project_file.IsEmpty())
			project_file = s;
	}

	OverviewerWindow w;
	if(!project_file.IsEmpty())
		w.OpenFile(project_file);
	
	w.LoadLayout();
	w.CheckRecovery();
	w.MarkSession(true);
	w.Run();
}
