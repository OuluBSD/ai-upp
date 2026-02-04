#include "Maestro.h"

namespace Upp {

void ConversionOrchestrator::Inventory(const String& source, const String& target)
{
	Cout() << "Generating inventory for source: " << source << "\n";
	MaestroInventory src_inv = InventoryGenerator::Generate(source);
	String src_path = ".maestro/convert/inventory/source_files.json";
	RealizeDirectory(GetFileDirectory(src_path));
	StoreAsJsonFile(src_inv, src_path, true);
	Cout() << "✓ Source inventory saved to " << src_path << "\n";

	Cout() << "Generating inventory for target: " << target << "\n";
	MaestroInventory tgt_inv = InventoryGenerator::Generate(target);
	String tgt_path = ".maestro/convert/inventory/target_files.json";
	RealizeDirectory(GetFileDirectory(tgt_path));
	StoreAsJsonFile(tgt_inv, tgt_path, true);
	Cout() << "✓ Target inventory saved to " << tgt_path << "\n";
}

void ConversionOrchestrator::Plan(const String& source, const String& target)
{
	Cout() << "Generating conversion plan from " << source << " to " << target << "\n";
	
	MaestroInventory src_inv, tgt_inv;
	if(!LoadFromJsonFile(src_inv, ".maestro/convert/inventory/source_files.json")) {
		Cerr() << "Error: Source inventory not found. Run inventory first.\n";
		return;
	}
	if(!LoadFromJsonFile(tgt_inv, ".maestro/convert/inventory/target_files.json")) {
		Cerr() << "Error: Target inventory not found. Run inventory first.\n";
		return;
	}
	
	ConversionMemory memory;
	memory.Load("."); // Assume current dir is project root
	
	WorkGraph wg = ConversionPlanner::GeneratePlan(src_inv, tgt_inv, memory);
	String plan_path = ".maestro/convert/plan/plan.json";
	RealizeDirectory(GetFileDirectory(plan_path));
	StoreAsJsonFile(wg, plan_path, true);
	
	Cout() << "✓ Conversion plan generated at " << plan_path << "\n";
}

void ConversionOrchestrator::Run(const String& source, const String& target, int limit)
{
	Cout() << "Executing conversion plan from " << source << " to " << target << "\n";
	
	WorkGraph wg;
	if(!LoadFromJsonFile(wg, ".maestro/convert/plan/plan.json")) {
		Cerr() << "Error: Conversion plan not found. Run plan first.\n";
		return;
	}
	
	ConversionMemory memory;
	memory.Load(".");
	
	CliMaestroEngine engine;
	engine.binary = "gemini";
	engine.model = "gemini-1.5-flash";
	engine.Arg("-y"); // Auto-confirm
	
	int completed = 0;
	int failed = 0;
	
	for(auto& phase : wg.phases) {
		Cout() << "\nPHASE: " << phase.name << "\n";
		Cout() << "------------------------------------------\n";
		
		for(auto& task : phase.tasks) {
			if(task.status == "done") {
				Cout() << "Skipping already completed task: " << task.title << "\n";
				continue;
			}
			
			if(limit > 0 && completed >= limit) break;
			
			Cout() << "Task: " << task.title << " (" << task.id << ")\n";
			Cout() << "  Intent: " << task.intent << "\n";
			
			String prompt;
			prompt << "You are a software conversion assistant.\n"
			       << "Goal: " << task.intent << "\n";
			
			if(task.inputs.GetCount() > 0) {
				prompt << "\nSource Files:\n";
				for(const auto& in : task.inputs) {
					prompt << "--- File: " << in << " ---\n";
					prompt << LoadFile(AppendFileName(source, in)) << "\n";
				}
			}
			
			prompt << "\nPlease provide the converted code or required files.\n";
			
			Cout() << "  Calling AI engine (" << engine.model << ")...\n";
			
			String response;
			bool success = false;
			bool done = false;
			
			engine.Send(prompt, [&](const MaestroEvent& ev) {
				if(ev.type == "message") {
					response << ev.text;
				} else if(ev.type == "done") {
					done = true;
					success = true;
				} else if(ev.type == "result" && ev.text.StartsWith("[API Error:")) {
					done = true;
					success = false;
				}
			});
			
			while(!done && engine.Do()) Sleep(10);
			
			if(success) {
				Cout() << "  ✓ AI response received (" << response.GetCount() << " bytes)\n";
				// In a real implementation, we would parse the response and save files.
				// For now, we'll just log it.
				String log_dir = ".maestro/convert/logs/" + task.id;
				RealizeDirectory(log_dir);
				SaveFile(AppendFileName(log_dir, "response.md"), response);
				
				// Run semantic check
				SemanticIntegrityChecker checker;
				SemanticResult sem = checker.RunCheck(task, source, target);
				Cout() << "  ✓ Semantic Check: " << sem.semantic_equivalence << " equivalence\n";
				if(sem.requires_human_review)
					Cout() << "  ⚠️  Requires human review due to potential loss or risk flags.\n";
				
				task.status = "done";
				completed++;
				Cout() << "  ✓ Task completed\n";
			} else {
				task.status = "failed";
				failed++;
				Cout() << "  ✗ Task failed\n";
			}
		}
		if(limit > 0 && completed >= limit) break;
	}
	
	// Save updated plan
	StoreAsJsonFile(wg, ".maestro/convert/plan/plan.json", true);
	
	Cout() << "\nRun Summary:\n"
	       << "  Tasks Completed: " << completed << "\n"
	       << "  Tasks Failed:    " << failed << "\n";
}

void ConversionOrchestrator::Validate(const String& source, const String& target)
{
	Cout() << "Validating conversion plan...\n";
	WorkGraph wg;
	if(!LoadFromJsonFile(wg, ".maestro/convert/plan/plan.json")) {
		Cerr() << "Error: Conversion plan not found.\n";
		return;
	}
	
	int errors = 0;
	Index<String> task_ids;
	for(const auto& phase : wg.phases) {
		for(const auto& task : phase.tasks) {
			if(task_ids.Find(task.id) >= 0) {
				Cerr() << "Error: Duplicate task ID: " << task.id << "\n";
				errors++;
			}
			task_ids.Add(task.id);
		}
	}
	
	if(errors == 0) Cout() << "✓ Plan is valid.\n";
	else Cout() << "✗ Plan has " << errors << " errors.\n";
}

void ConvertCommand::Execute(const Vector<String>& args)
{
	CommandLineArguments cla;
	cla.AddPositional("subcommand", UNKNOWN_V);
	cla.AddPositional("arg1", UNKNOWN_V);
	cla.AddPositional("arg2", UNKNOWN_V);
	cla.AddArg('l', "limit", true);
	cla.Parse(args);
	
	if (cla.GetPositionalCount() < 1) { ShowHelp(); return; }
	
	String sub = AsString(cla.GetPositional(0));
	String a1 = cla.GetPositionalCount() > 1 ? AsString(cla.GetPositional(1)) : ".";
	String a2 = cla.GetPositionalCount() > 2 ? AsString(cla.GetPositional(2)) : "out";
	
	if (sub == "inventory") ConversionOrchestrator::Inventory(a1, a2);
	else if (sub == "help" || sub == "h") { ShowHelp(); return; }
	else if (sub == "plan" || sub == "p") ConversionOrchestrator::Plan(a1, a2);
	else if (sub == "run" || sub == "r") {
		int limit = 0;
		if(cla.IsArg('l')) limit = StrInt(cla.GetArg('l'));
		ConversionOrchestrator::Run(a1, a2, limit);
	}
	else if (sub == "validate") ConversionOrchestrator::Validate(a1, a2);
	else if (sub == "playbook") {
		if(cla.GetPositionalCount() < 2) {
			Cout() << "usage: MaestroCLI convert playbook {list,show,use} [id]\n";
			return;
		}
		String pb_sub = AsString(cla.GetPositional(1));
		PlaybookManager pbm;
		if(pb_sub == "list" || pb_sub == "ls") {
			Array<Playbook> list = pbm.ListPlaybooks();
			Cout() << Format("% -25s % -50s % -10s\n", "ID", "TITLE", "VERSION");
			Cout() << String('-', 85) << "\n";
			for(const auto& pb : list)
				Cout() << Format("% -25s % -50s % -10s\n", pb.id, pb.title, pb.version);
		}
		else if(pb_sub == "show" || pb_sub == "sh") {
			if(cla.GetPositionalCount() < 3) { Cerr() << "Error: Requires playbook ID.\n"; return; }
			String id = AsString(cla.GetPositional(2));
			Playbook* pb = pbm.LoadPlaybook(id);
			if(pb) {
				Cout() << "Playbook: " << pb->title << " (" << pb->id << ")\n"
				       << "Version:  " << pb->version << "\n"
				       << "Intent:   " << pb->intent << "\n";
				delete pb;
			} else Cerr() << "Error: Playbook not found.\n";
		}
		else if(pb_sub == "use") {
			if(cla.GetPositionalCount() < 3) { Cerr() << "Error: Requires playbook ID.\n"; return; }
			String id = AsString(cla.GetPositional(2));
			if(pbm.BindPlaybook(id)) Cout() << "✓ Playbook '" << id << "' bound to current conversion.\n";
			else Cerr() << "Error: Failed to bind playbook.\n";
		}
		else Cout() << "Unknown playbook subcommand: " << pb_sub << "\n";
	}
	else if (sub == "add" || sub == "new" || sub == "n") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires pipeline name.\n"; return; }
		PipelineRuntime pr;
		ConversionPipeline p = pr.CreatePipeline(AsString(cla.GetPositional(1)), a1, a2);
		Cout() << "✓ Created conversion pipeline: " << p.id << "\n";
	}
	else if (sub == "status" || sub == "s") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires pipeline ID.\n"; return; }
		PipelineRuntime pr;
		ConversionPipeline p = pr.LoadPipeline(AsString(cla.GetPositional(1)));
		Cout() << "Pipeline ID: " << p.id << "\n" << "Name:        " << p.name << "\n" << "Status:      " << p.status << "\n";
	}
	else if (sub == "runs") {
		if(cla.GetPositionalCount() < 2) { Cout() << "usage: MaestroCLI convert runs {list,show,diff} [run_id]\n"; return; }
		String run_sub = AsString(cla.GetPositional(1));
		RegressionReplay rr;
		if(run_sub == "list" || run_sub == "ls") {
			Array<RunManifest> list = rr.ListRuns();
			Cout() << Format("% -36s % -12s % -20s\n", "RUN ID", "STATUS", "TIMESTAMP");
			Cout() << String('-', 70) << "\n";
			for(const auto& m : list)
				Cout() << Format("% -36s % -12s % -20s\n", m.run_id, m.status, Format(m.timestamp));
		}
		else if(run_sub == "show" || run_sub == "sh") {
			if(cla.GetPositionalCount() < 3) { Cerr() << "Error: Requires run ID.\n"; return; }
			RunManifest m = rr.LoadManifest(AsString(cla.GetPositional(2)));
			Cout() << "Run ID:    " << m.run_id << "\n" << "Status:    " << m.status << "\n" << "Timestamp: " << Format(m.timestamp) << "\n";
		}
	}
	else if (sub == "replay") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires run ID.\n"; return; }
		String run_id = AsString(cla.GetPositional(1));
		RegressionReplay rr;
		ConversionMemory memory; memory.Load(".");
		DriftReport report = rr.DetectDrift(run_id, a2, memory);
		Cout() << "Replaying run " << run_id << "...\n";
		Cout() << "✓ Drift Detection: " << (report.drift_detected ? "DRIFT DETECTED" : "No drift") << "\n";
	}
	else if (sub == "show" || sub == "sh" || sub == "reset" || sub == "rst" || sub == "batch" || sub == "b") {
		Cout() << "Subcommand '" << sub << "' is not yet fully implemented in C++ but is on the roadmap.\n";
	}
	else { Cout() << "Unknown convert subcommand: " << sub << "\n"; ShowHelp(); }
}

} // namespace Upp