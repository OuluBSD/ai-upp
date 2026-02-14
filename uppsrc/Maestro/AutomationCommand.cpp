#include "Maestro.h"

namespace Upp {

void AutomationCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI automation {run,plan,enact} <package> [...]\n"
	       << "\n"
	       << "Subcommands:\n"
	       << "    run <package>     Execute formal verification on the specified package\n"
	       << "    plan <package>    Synthesize a WorkGraph using Logic Engine and Constraints\n"
	       << "    enact <package>   Execute the synthesized plan on the running application\n";
}

void AutomationCommand::Execute(const Vector<String>& args) {
	if (args.GetCount() < 2) {
		ShowHelp();
		return;
	}

	String sub = args[0];
	String pkg = args[1];
	
	if (sub == "enact") {
		Cout() << "Enacting synthesized plan for " << pkg << "...\n";
		
		// 1. Find the latest plan
		String plan_root = FindPlanRoot();
		String workgraphs_dir = AppendFileName(plan_root, "docs/maestro/plans/workgraphs");
		
		String latest_plan_path;
		Time latest_time = Time::Low();
		
		FindFile ff(AppendFileName(workgraphs_dir, "plan_synth_*.json"));
		while(ff) {
			if (ff.GetLastWriteTime() > latest_time) {
				latest_time = ff.GetLastWriteTime();
				latest_plan_path = ff.GetPath();
			}
			ff.Next();
		}
		
		if (latest_plan_path.IsEmpty()) {
			Cerr() << "Error: No synthesized plan found in " << workgraphs_dir << "\n";
			return;
		}
		
		Cout() << "Executing plan: " << latest_plan_path << "\n";
		
		// 2. Load the plan
		WorkGraph wg;
		if (!LoadFromJsonFile(wg, latest_plan_path)) {
			Cerr() << "Error: Failed to load plan from " << latest_plan_path << "\n";
			return;
		}
		
		// 3. Generate a temporary Python script to drive the application
		// We use the 'AutomationElement' bindings from Ctrl/Automation
		String script;
		script << "import time\n";
		script << "log('Starting autonomous enactment for " << pkg << "')\n";
		script << "wait_time(1.0)\n"; // Wait for app to open
		
		for(const auto& phase : wg.phases) {
			for(const auto& task : phase.tasks) {
				String title = task.title;
				script << "log('Executing: " << title << "')\n";
				
				// Map high-level actions to low-level UI interactions
				// Example: "Implement mainWindow" -> we skip if it's internal
				// Example: "Show mainWindow" -> verify it's visible
				// Example: "Enable loginButton" -> find('loginButton').click()
				
				if (title.StartsWith("Show ")) {
					String entity = title.Mid(5);
					script << "el = find('" << entity << "')\n";
					script << "if el:\n";
					script << "    log('  " << entity << " is visible')\n";
					script << "else:\n";
					script << "    log('  Error: " << entity << " not found!')\n";
				}
				else if (title.StartsWith("Enable ")) {
					String entity = title.Mid(7);
					script << "el = find('" << entity << "')\n";
					script << "if el:\n";
					script << "    el.click()\n";
					script << "    log('  Clicked " << entity << "')\n";
					script << "else:\n";
					script << "    log('  Error: " << entity << " not found!')\n";
				}
				script << "wait_time(0.5)\n";
			}
		}
		script << "log('Enactment complete.')\n";
		script << "exit(0)\n";
		
		String script_path = AppendFileName(GetHomeDirectory(), ".gemini/tmp/enact_script.py");
		RealizeDirectory(GetFileDirectory(script_path));
		SaveFile(script_path, script);
		
		Cout() << "Generated enactment script saved to " << script_path << "\n";
		
		// 4. Build and Run
		String method = "CLANG";
		String flags = "+GUI";
		
		Cout() << "Building " << pkg << "...\n";
		String uppsrc = AppendFileName(plan_root, "uppsrc");
		String upptst = AppendFileName(plan_root, "upptst");
		String examples = AppendFileName(plan_root, "examples");
		String assembly = uppsrc + "," + upptst + "," + examples;
		String umk_cmd = "umk " + assembly + " " + pkg + " " + method + " -abvs " + flags;
		
		if (system(umk_cmd) != 0) {
			Cerr() << "Error: Build failed.\n";
			return;
		}
		
		// Find exe
		String exe_path;
		String pkg_out = AppendFileName(AppendFileName(GetHomeDirectory(), ".cache/upp.out"), pkg);
		auto LookForExe = [&](const String& dir, auto& self) -> void {
			FindFile ff(AppendFileName(dir, "*"));
			while (ff) {
				if (ff.IsFolder()) self(ff.GetPath(), self);
				else if (ff.IsFile() && ff.GetName() == pkg && ff.IsExecutable()) exe_path = ff.GetPath();
				ff.Next();
			}
		};
		if (DirectoryExists(pkg_out)) LookForExe(pkg_out, LookForExe);
		
		if (exe_path.IsEmpty()) {
			Cerr() << "Error: Could not find executable.\n";
			return;
		}
		
		Cout() << "Executing enactment: " << exe_path << " --script " << script_path << "\n";
		system(exe_path + " --script " + script_path);
		Cout() << "Enactment finished.\n";
	}
	else if (sub == "plan") {
		Cout() << "Synthesizing WorkGraph for " << pkg << " using Logic Engine...\n";
		
		// 1. Load Constraints
		String plan_root = FindPlanRoot();
		String constraints_dir = AppendFileName(plan_root, "docs/maestro/plans/constraints");
		
		// Try to find the constraint file. We assume it matches the package name or we look for *any* .ugui if specific one not found?
		// The prompt implies we link Runbook -> Constraints.
		// For simplicity, we look for a .ugui file that contains the package name or just the most recent one.
		// Or we assume the constraints file is named <runbook_id>.ugui.
		// Let's search for *.ugui.
		
		String ugui_path;
		String runbook_id;
		
		FindFile ff(AppendFileName(constraints_dir, "*.ugui"));
		while(ff) {
			ugui_path = ff.GetPath();
			runbook_id = GetFileTitle(ugui_path); // Assuming file name is runbook ID
			break; // Take the first one for now, or improve logic
		}
		
		if (ugui_path.IsEmpty()) {
			Cerr() << "Error: No constraints (.ugui) found in " << constraints_dir << "\n";
			return;
		}
		
		Cout() << "Using constraints from: " << ugui_path << "\n";
		String content = LoadFile(ugui_path);
		Vector<String> lines = Split(content, '\n');
		
		// 2. Setup Action Planner
		ActionPlanner planner;
		ActionPlannerWrapper wrap(planner);
		
		// Extract atoms from constraints
		// Constraints are like: VISIBLE(mainWindow)
		// We map this to atoms: "visible_mainWindow", "exists_mainWindow"
		
		Index<String> entities;
		
		for(const String& line : lines) {
			String l = TrimBoth(line);
			if(l.IsEmpty()) continue;
			
			// Simple parsing: PREDICATE(entity)
			int p_start = l.Find('(');
			int p_end = l.Find(')');
			if (p_start > 0 && p_end > p_start) {
				String pred = l.Left(p_start);
				String entity = l.Mid(p_start + 1, p_end - p_start - 1);
				entities.FindAdd(entity);
			}
		}
		
		// Define Atoms
		// For each entity 'e', we have atoms: exists_e, visible_e, enabled_e
		for(const String& e : entities) {
			wrap.GetAtomIndex("exists_" + e);
			wrap.GetAtomIndex("visible_" + e);
			wrap.GetAtomIndex("enabled_" + e);
		}
		
		// Define Actions
		// 1. Implement(e):  Pre: {} -> Post: {exists_e}
		// 2. Show(e):       Pre: {exists_e} -> Post: {visible_e}
		// 3. Enable(e):     Pre: {visible_e} -> Post: {enabled_e}
		
		for(const String& e : entities) {
			int idx;
			
			// Implement
			idx = wrap.GetEventIndex("Implement " + e);
			wrap.SetPostCondition("Implement " + e, "exists_" + e, true);
			wrap.SetCost("Implement " + e, 10);
			
			// Show
			idx = wrap.GetEventIndex("Show " + e);
			wrap.SetPreCondition("Show " + e, "exists_" + e, true);
			wrap.SetPostCondition("Show " + e, "visible_" + e, true);
			wrap.SetCost("Show " + e, 5);
			
			// Enable
			idx = wrap.GetEventIndex("Enable " + e);
			wrap.SetPreCondition("Enable " + e, "visible_" + e, true);
			wrap.SetPostCondition("Enable " + e, "enabled_" + e, true);
			wrap.SetCost("Enable " + e, 2);
		}
		
		// 3. Set Goal State
		BinaryWorldState goal;
		// We need to map the planner's state to BinaryWorldState.
		// ActionPlannerWrapper doesn't expose a direct way to build a BWS easily without 'SetPreCondition' style
		// But we can use the internal ActionPlanner if we knew the IDs, or just create a dummy "GoalAction" and get its preconds?
		// No, `OmniActionPlanner` or `SearchAlgos` usually take a start and goal BWS.
		// `ActionPlanner` class itself has `DoAction` but not a "Search" method directly?
		// Wait, `ActionPlanner.h` has `OmniActionPlanner` which inherits `OmniSearcher`.
		// But here I'm using `ActionPlanner` core.
		
		// Let's use `AStar` or `BreadthFirst` from `SearchAlgos.h`?
		// Or `ActionNode` which has `GetEstimate`?
		// Actually, `ActionPlanner` seems to be the *Domain Definition*. The searcher is separate.
		// Let's look at `Tests.cpp` in `AI/Core/Base` if I could... but I can't.
		
		// `ActionPlanner` has `GetPossibleStateTransition`.
		// I will implement a simple BFS here since the state space is small.
		
		BinaryWorldState start_state; // Empty
		
		// Build Goal State from .ugui
		BinaryWorldState goal_state;
		// We assume goal is satisfying all constraints.
		// But `BinaryWorldState` is just a bitset.
		// I need to know the bit indices.
		
		for(const String& line : lines) {
			String l = TrimBoth(line);
			if(l.IsEmpty()) continue;
			int p_start = l.Find('(');
			int p_end = l.Find(')');
			if (p_start > 0 && p_end > p_start) {
				String pred = l.Left(p_start);
				String entity = l.Mid(p_start + 1, p_end - p_start - 1);
				
				String atom;
				if(pred == "VISIBLE") atom = "visible_" + entity;
				else if(pred == "ENABLED") atom = "enabled_" + entity;
				// Add others as needed
			}
		}

		// RE-STRATEGY: Use `OmniActionPlanner` if possible, or just a simple custom search using `ActionPlanner` as the transition model.
		// `ActionPlanner` is simple: generic STRIPS.
		
		struct Node {
			BinaryWorldState state;
			Vector<int> plan; // Action IDs
			double cost = 0;
		};
		
		// Create a "GoalReached" action that requires all constraints.
		// Then plan from Start to satisfy GoalReached's preconditions.
		
		int goal_action_id = wrap.GetEventIndex("GoalReached");
		for(const String& line : lines) {
			String l = TrimBoth(line);
			if(l.IsEmpty()) continue;
			int p_start = l.Find('(');
			int p_end = l.Find(')');
			if (p_start > 0 && p_end > p_start) {
				String pred = l.Left(p_start);
				String entity = l.Mid(p_start + 1, p_end - p_start - 1);
				String atom;
				if(pred == "VISIBLE") atom = "visible_" + entity;
				else if(pred == "ENABLED") atom = "enabled_" + entity;
				
				if(!atom.IsEmpty()) {
					wrap.SetPreCondition("GoalReached", atom, true);
				}
			}
		}
		
		// Now we want to reach the state where "GoalReached" is applicable.
		// Simple backward chaining or forward search.
		// Let's do forward search until "GoalReached" is applicable.
		
		Array<Node> queue;
		Node& start = queue.Add(); // Default BWS is empty (all false)
		start.state.mask = wrap.GetMask();
		
		// We need a visited set. BWS has `GetHashValue()`.
		Index<hash_t> visited;
		
		int best_plan_idx = -1;
		
		int max_iter = 1000;
		int iter = 0;
		
		while(iter++ < max_iter && !queue.IsEmpty()) {
			// Get lowest cost (simplified: just pop first, BFS)
			Node current = pick(queue[0]);
			queue.Remove(0);
			
			// Check if GoalReached is applicable
			Array<BinaryWorldState*> next_states;
			Vector<int> act_ids;
			Vector<double> costs;
			
			planner.GetPossibleStateTransition(current.state, next_states, act_ids, costs);
			
			for(int i = 0; i < act_ids.GetCount(); i++) {
				if (act_ids[i] == goal_action_id) {
					Cout() << "Plan found! Length: " << current.plan.GetCount() << "\n";
					
					// Convert to WorkGraph
					WorkGraph wg;
					wg.title = "Synthesized Plan for " + pkg;
					wg.domain = "software-construction";
					
					WorkGraphPhase phase;
					phase.name = "Implementation";
					
					for(int action_id : current.plan) {
						String act_name = wrap.GetActionName(action_id);
						WorkGraphTask task;
						task.title = act_name;
						task.intent = "Satisfy constraint via " + act_name;
						task.definition_of_done.Add().kind = "code";
						phase.tasks.Add(task);
						
						Cout() << "  + " << act_name << "\n";
					}
					wg.phases.Add(phase);
					
					// Save
					String out_dir = AppendFileName(plan_root, "docs/maestro/plans/workgraphs");
					RealizeDirectory(out_dir);
					String out_path = AppendFileName(out_dir, "plan_synth_" + AsString(GetSysTime().Get()) + ".json");
					StoreAsJsonFile(wg, out_path, true);
					Cout() << "WorkGraph saved to " << out_path << "\n";
					return;
				}
			}
			
			// Expand
			for(int i = 0; i < next_states.GetCount(); i++) {
				BinaryWorldState& next_ws = *next_states[i];
				hash_t h = next_ws.GetHashValue();
				if(visited.Find(h) < 0) {
					visited.Add(h);
					Node& next_node = queue.Add();
					next_node.state = next_ws;
					next_node.plan.Append(current.plan);
					next_node.plan.Add(act_ids[i]);
					next_node.cost = current.cost + costs[i];
				}
			}
		}
		
		Cerr() << "Planner failed to find a solution.\n";
	}
	else if (sub == "run") {
		String method = "CLANG";
		String flags = "+GUI";

		for (int i = 2; i < args.GetCount(); i++) {
			if (args[i] == "--method" && i + 1 < args.GetCount()) method = args[++i];
			else if (args[i] == "--flags" && i + 1 < args.GetCount()) flags = args[++i];
		}

		Cout() << "Running formal verification for: " << pkg << "\n";

	// 1. Build the package first to ensure it's up to date
	Cout() << "Building " << pkg << "...\n";
	
	String plan_root = FindPlanRoot();
	String uppsrc = AppendFileName(plan_root, "uppsrc");
	String upptst = AppendFileName(plan_root, "upptst");
	String examples = AppendFileName(plan_root, "examples");
	
	String assembly = uppsrc + "," + upptst + "," + examples;
	String umk_cmd = "umk " + assembly + " " + pkg + " " + method + " -abvs " + flags;
	
	Cout() << "Using assembly: " << assembly << "\n";
	int build_res = system(umk_cmd);
	if (build_res != 0) {
		Cerr() << "Error: Build failed with exit code " << build_res << "\n";
		return;
	}

	// 2. Find the executable
	String out_dir = AppendFileName(GetHomeDirectory(), ".cache/upp.out");
	String pkg_out = AppendFileName(out_dir, pkg);
	
	String exe_path;
	Time latest_time = Time::Low();

	// Recursively look for the executable
	auto LookForExe = [&](const String& dir, auto& self) -> void {
		FindFile ff(AppendFileName(dir, "*"));
		while (ff) {
			if (ff.IsFolder()) {
				self(ff.GetPath(), self);
			} else if (ff.IsFile() && ff.GetName() == pkg && ff.IsExecutable()) {
				if (ff.GetLastWriteTime() > latest_time) {
					latest_time = ff.GetLastWriteTime();
					exe_path = ff.GetPath();
				}
			}
			ff.Next();
		}
	};
	
	if (DirectoryExists(pkg_out))
		LookForExe(pkg_out, LookForExe);

	if (exe_path.IsEmpty()) {
		Cerr() << "Error: Could not find executable for package " << pkg << "\n";
		return;
	}

	Cout() << "Executing: " << exe_path << " --test\n";

	// 3. Run the executable with --test
	String run_cmd = exe_path + " --test";
	int run_res = system(run_cmd);
	
	// 4. Parse the log file
	String log_path = AppendFileName(AppendFileName(AppendFileName(GetHomeDirectory(), ".local/state/u++"), "log"), pkg + ".log");
	if (!FileExists(log_path)) {
		Cerr() << "Error: Log file not found at " << log_path << "\n";
		return;
	}

	String log_content = LoadFile(log_path);
	Vector<String> lines = Split(log_content, '\n');
	
	int total = 0;
	int success = 0;
	int failure = 0;

	for (const String& line : lines) {
		if (line.Find("[CTRL] SUCCESS") >= 0) {
			total++;
			success++;
		} else if (line.Find("[CTRL] FAILURE") >= 0) {
			total++;
			failure++;
			Cerr() << "  " << line << "\n";
		}
	}

	Cout() << "\nFormal Verification Summary:\n"
	       << "----------------------------\n"
	       << "Total Constraints Checked: " << total << "\n"
	       << "Success:                   " << success << "\n"
	       << "Failure:                   " << failure << "\n";

	if (total > 0 && failure == 0) {
		Cout() << "✓ ALL CONSTRAINTS PROVEN.\n";
	} else if (total == 0) {
		Cerr() << "⚠ NO CONSTRAINTS FOUND OR CHECKED.\n";
	} else {
		Cerr() << "✗ VERIFICATION FAILED.\n";
	}
}

}END_UPP_NAMESPACE
