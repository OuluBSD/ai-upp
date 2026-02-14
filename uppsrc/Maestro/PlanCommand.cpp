#include "Maestro.h"

namespace Upp {

void PlanCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI plan [-h] {add,list,ls,remove,show,sh,add-item,ai,remove-item,ri,status}\n"
	       << "\n"
	       << "Plan subcommands:\n"
	       << "    add (a)             Add a new plan (track)\n"
	       << "    list (ls)           List all plans\n"
	       << "    remove (rm)         Remove a plan\n"
	       << "    show (sh)           Show a plan and its items\n"
	       << "    add-item (ai)       Add an item to a plan\n"
	       << "    remove-item (ri)    Remove an item from a plan\n"
	       << "    status              Update task status\n";
}

void PlanCommand::Execute(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.AddPositional("subcommand", UNKNOWN_V);
	cla.AddPositional("arg1", UNKNOWN_V);
	cla.AddPositional("arg2", UNKNOWN_V);
	cla.AddPositional("arg3", UNKNOWN_V);
	cla.AddPositional("arg4", UNKNOWN_V);
	cla.Parse(args);
	
	if (cla.GetPositionalCount() == 0) {
		ShowHelp();
		return;
	}
	
	String root = FindPlanRoot();
	if(root.IsEmpty()) {
		Cerr() << "Error: Could not find project plan directory.\n";
		return;
	}
	String plan_root = AppendFileName(root, "docs/maestro/tasks");
	String docs_root = GetDocsRoot(root);

	PlanParser parser;
	parser.Load(plan_root);

	String sub = AsString(cla.GetPositional(0));
	int track_idx = -1;
	if(!sub.IsEmpty() && IsDigit(sub[0])) {
		track_idx = StrInt(sub);
		if(cla.GetPositionalCount() > 1) sub = AsString(cla.GetPositional(1));
		else sub = "show";
	}

	if (sub == "list" || sub == "ls") {
		Cout() << "Plans (Tracks) found in " << plan_root << ":\n";
		for(int i = 0; i < parser.tracks.GetCount(); i++) {
			const auto& track = parser.tracks[i];
			Cout() << Format(" %d. [%-10s] %s (%d phases)\n", 
			                 i, track.status, track.name, track.phases.GetCount());
		}
	}
	else if (sub == "show" || sub == "sh") {
		const Track* found = nullptr;
		if(track_idx >= 0) {
			if(track_idx < parser.tracks.GetCount()) found = &parser.tracks[track_idx];
			else { Cerr() << "Error: Track index out of range.\n"; return; }
		} else {
			if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires track name or index.\n"; return; }
			String name = AsString(cla.GetPositional(1));
			for(const auto& track : parser.tracks) {
				if(track.name == name || track.id == name) { found = &track; break; }
			}
		}
		if(!found) { Cerr() << "Error: Track not found.\n"; return; }
		
		Cout() << "Track: " << found->name << " [" << found->status << "]\n";
		Cout() << "Path:  " << found->path << "\n\n";
		for(const auto& phase : found->phases) {
			Cout() << "Phase: " << phase.name << " [" << phase.status << "]\n";
			for(int i = 0; i < phase.tasks.GetCount(); i++) {
				const Task& task = phase.tasks[i];
				Cout() << Format("  %d. [%-11s] %s\n", i + 1, StatusToString(task.status), task.name);
			}
			Cout() << "\n";
		}
	}
	else if (sub == "add" || sub == "a") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires track name.\n"; return; }
		String name = AsString(cla.GetPositional(1));
		String path = AppendFileName(plan_root, name);
		if(DirectoryExists(path)) { Cerr() << "Error: Track already exists.\n"; return; }
		if(RealizeDirectory(path)) Cout() << "Created track directory: " << path << "\n";
		else Cerr() << "Error: Failed to create track directory.\n";
	}
	else if (sub == "add-item" || sub == "ai") {
		if(cla.GetPositionalCount() < 4) { Cerr() << "Error: Requires <track> <phase> <task_title>.\n"; return; }
		String track = AsString(cla.GetPositional(1));
		String phase = AsString(cla.GetPositional(2));
		String title = AsString(cla.GetPositional(3));
		String track_path = AppendFileName(plan_root, track);
		if(!DirectoryExists(track_path)) { Cerr() << "Error: Track not found.\n"; return; }
		String phase_path = AppendFileName(track_path, phase);
		RealizeDirectory(phase_path);
		String task_file = AppendFileName(phase_path, title + ".md");
		if(FileExists(task_file)) { Cerr() << "Error: Task already exists.\n"; return; }
		String content = "# Task: " + title + "\n# Status: TODO\n\n## Objective\n" + title + "\n";
		if(SaveFile(task_file, content)) Cout() << "Created task file: " << task_file << "\n";
		else Cerr() << "Error: Failed to create task file.\n";
	}
	else if (sub == "status") {
		if(cla.GetPositionalCount() < 5) { Cerr() << "Error: Requires <track> <phase> <task> <status>.\n"; return; }
		String track = AsString(cla.GetPositional(1));
		String phase = AsString(cla.GetPositional(2));
		String task = AsString(cla.GetPositional(3));
		String status_str = AsString(cla.GetPositional(4));
		TaskStatus status = StringToStatus(ToLower(status_str));
		if(status == STATUS_UNKNOWN) { Cerr() << "Error: Unknown status '" << status_str << "'.\n"; return; }
		PlanParser p;
		if(p.UpdateTaskStatus(docs_root, track, phase, task, status)) Cout() << "Updated status of '" << task << "' to " << status_str << "\n";
		else Cerr() << "Error: Failed to update task status.\n";
	}
	else if (sub == "decompose") {
		String freeform;
		if(cla.GetPositionalCount() > 1) freeform = AsString(cla.GetPositional(1));
		else {
			Cout() << "Enter your request: ";
			freeform = ReadStdIn();
		}
		if(freeform.IsEmpty()) return;

		CliMaestroEngine engine;
		engine.binary = "gemini";
		engine.model = "gemini-1.5-pro";
		engine.Arg("-o").Arg("stream-json");
		
		WorkGraphGenerator wgg(engine, true);
		try {
			WorkGraph wg = wgg.Generate(freeform, ValueMap());
			String out_dir = AppendFileName(docs_root, "docs/maestro/plans/workgraphs");
			RealizeDirectory(out_dir);
			String out_path = AppendFileName(out_dir, "plan_" + AsString(GetSysTime().Get()) + ".json");
			if(StoreAsJsonFile(wg, out_path, true)) {
				Cout() << "WorkGraph generated and saved to: " << out_path << "\n";
			}
		} catch(const Exc& e) {
			Cerr() << "Error: " << e << "\n";
		}
	}
	else if (sub == "enact") {
		if(cla.GetPositionalCount() < 2) {
			Cerr() << "Error: 'plan enact' requires a WorkGraph JSON file path.\n";
			return;
		}
		String path = AsString(cla.GetPositional(1));
		WorkGraph wg;
		if(!LoadFromJsonFile(wg, path)) {
			Cerr() << "Error: Failed to load WorkGraph from " << path << "\n";
			return;
		}
		
		String trk_dir = AppendFileName(plan_root, wg.track.name);
		RealizeDirectory(trk_dir);
		Cout() << "Enacting track: " << wg.track.name << "\n";
		
		for(const auto& phase : wg.phases) {
			String ph_dir = AppendFileName(trk_dir, phase.name);
			RealizeDirectory(ph_dir);
			Cout() << "  - Phase: " << phase.name << "\n";
			
			for(const auto& task : phase.tasks) {
				String task_file = AppendFileName(ph_dir, task.title + ".md");
				String content;
				content << "# Task: " << task.title << "\n"
				        << "# Status: TODO\n\n"
				        << "## Objective\n" << task.intent << "\n\n";
				if(task.definition_of_done.GetCount() > 0) {
					content << "## Definition of Done\n";
					for(const auto& dod : task.definition_of_done) {
						if(dod.kind == "command") content << "- [ ] Command: `" << dod.cmd << "`\n";
						else if(dod.kind == "file") content << "- [ ] File: `" << dod.path << "`\n";
					}
					content << "\n";
				}
				if(SaveFile(task_file, content)) Cout() << "    * Task created: " << task.title << "\n";
			}
		}
	}
	else if (sub == "run") {
		if(cla.GetPositionalCount() < 2) {
			Cerr() << "Error: 'plan run' requires a WorkGraph JSON file path.\n";
			return;
		}
		String path = AsString(cla.GetPositional(1));
		WorkGraph wg;
		if(!LoadFromJsonFile(wg, path)) {
			Cerr() << "Error: Failed to load WorkGraph from " << path << "\n";
			return;
		}
		
		bool execute = false;
		for(int i = 0; i < args.GetCount(); i++) if(args[i] == "--execute") execute = true;
		
		WorkGraphRunner runner(wg, !execute, true);
		RunSummary rs = runner.Run();
		
		Cout() << "\nRun Summary:\n"
		       << "  Tasks Completed: " << rs.tasks_completed << "\n"
		       << "  Tasks Failed:    " << rs.tasks_failed << "\n"
		       << "  Dry Run:         " << (rs.dry_run ? "yes" : "no") << "\n";
	}
	else if (sub == "score" || sub == "recommend") {
		if(cla.GetPositionalCount() < 2) {
			Cerr() << "Error: '" << sub << "' requires a WorkGraph JSON file path.\n";
			return;
		}
		String path = AsString(cla.GetPositional(1));
		WorkGraph wg;
		if(!LoadFromJsonFile(wg, path)) {
			Cerr() << "Error: Failed to load WorkGraph from " << path << "\n";
			return;
		}
		
		String profile = "default";
		if(cla.GetPositionalCount() > 2) profile = AsString(cla.GetPositional(2));
		
		RankedWorkGraph rwg = WorkGraphScorer::Rank(wg, profile);
		
		Cout() << "WorkGraph Scoring: " << wg.title << "\n"
		       << "Profile: " << profile << "\n"
		       << "Total tasks: " << rwg.summary["total_tasks"] << "\n\n";
		
		int limit = (sub == "recommend" ? 3 : 10);
		for(int i = 0; i < min(limit, rwg.ranked_tasks.GetCount()); i++) {
			const auto& t = rwg.ranked_tasks[i];
			Cout() << Format("%d. [%.1f] %s (%s)\n", i + 1, t.score, t.task_title, t.task_id);
		}
	}
	else { Cout() << "Unknown plan subcommand: " << sub << "\n"; ShowHelp(); }
}

}
