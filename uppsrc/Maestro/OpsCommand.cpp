#include "Maestro.h"

namespace Upp {

void OpsCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI ops [-h] {doctor,run,list,ls,show,sh} ...\n"
	       << "positional arguments:\n"
	       << "    doctor              Run health checks and report gates/blockers\n"
	       << "    run                 Execute an ops plan (deterministic runbook)\n"
	       << "    list (ls)           List ops run records\n"
	       << "    show (sh)           Show ops run details\n";
}

static bool CheckCircular(const String& pkg, const VectorMap<String, Vector<String>>& graph, Index<String>& visited, Index<String>& stack, String& path) {
	if(stack.Find(pkg) >= 0) {
		path = "";
		for(int i = stack.Find(pkg); i < stack.GetCount(); i++)
			path << stack[i] << " -> ";
		path << pkg;
		return true;
	}
	if(visited.Find(pkg) >= 0) return false;
	
	visited.Add(pkg);
	stack.Add(pkg);
	
	int q = graph.Find(pkg);
	if(q >= 0) {
		for(const String& dep : graph[q]) {
			if(CheckCircular(dep, graph, visited, stack, path)) return true;
		}
	}
	
	stack.Remove(stack.GetCount() - 1);
	return false;
}

static static void Doctor(const Vector<String>& args) {
	Cout() << "[*] Running Maestro Project Doctor...\n";
	
	String root = FindPlanRoot();
	if(root.IsEmpty()) {
		Cerr() << "[!] ERROR: Not in a Maestro repository (could not find .maestro or docs/phases).\n";
		return;
	}
	Cout() << "[-] Project Root: " << root << "\n";
	
	// 1. Check required directories
	Vector<String> required = { ".maestro", "docs/maestro/runbooks", "docs/maestro/workflows", "docs/maestro/issues" };
	int missing_docs = 0;
	for(const String& d : required) {
		if(!DirectoryExists(AppendFileName(root, d))) {
			Cout() << "[!] Warning: Missing recommended directory: " << d << "\n";
			missing_docs++;
		}
	}
	if(missing_docs == 0) Cout() << "[+] All standard Maestro directories found.\n";
	
	// 2. Scan packages and check for circular dependencies
	Cout() << "[-] Scanning packages for dependency issues...\n";
	RepoScanner scanner;
	scanner.Scan(root);
	
	VectorMap<String, Vector<String>> graph;
	for(const auto& pkg : scanner.packages) {
		graph.Add(pkg.name, clone(pkg.dependencies));
	}
	
	Index<String> visited;
	Index<String> stack;
	String cycle_path;
	bool has_cycle = false;
	for(int i = 0; i < graph.GetCount(); i++) {
		if(CheckCircular(graph.GetKey(i), graph, visited, stack, cycle_path)) {
			Cerr() << "[!] ERROR: Circular dependency detected: " << cycle_path << "\n";
			has_cycle = true;
			break;
		}
	}
	if(!has_cycle) Cout() << "[+] No circular dependencies found among " << scanner.packages.GetCount() << " packages.\n";
	
	Cout() << "[*] Doctor's report complete.\n";
}

static void RunOps(const Vector<String>& args) {
	if(args.IsEmpty()) {
		Cout() << "usage: MaestroCLI ops run <runbook_id>\n";
		return;
	}
	
	String root = FindPlanRoot();
	if(root.IsEmpty()) {
		Cerr() << "Error: Not in a Maestro repository.\n";
		return;
	}
	
	String rb_id = args[0];
	RunbookManager rbm(root);
	Runbook rb = rbm.LoadRunbook(rb_id);
	
	if(rb.id.IsEmpty()) {
		Cerr() << "Error: Runbook '" << rb_id << "' not found.\n";
		return;
	}
	
	Cout() << "[*] Executing Runbook: " << rb.title << " (" << rb.id << ")\n";
	
	String run_id = "run-" + FormatIntHex(Random(), 8);
	String ops_dir = AppendFileName(root, ".maestro/ops/runs");
	RealizeDirectory(ops_dir);
	
	ValueMap run_meta;
	run_meta("id") = run_id;
	run_meta("runbook_id") = rb.id;
	run_meta("title") = rb.title;
	run_meta("timestamp") = GetSysTime();
	run_meta("status") = "running";
	
	ValueArray step_results;
	bool success = true;
	for(const auto& step : rb.steps) {
		Cout() << "Step " << step.n << " [" << step.actor << "]: " << step.action << "\n";
		ValueMap sr;
		sr("n") = step.n;
		sr("action") = step.action;
		
		if(!step.command.IsEmpty()) {
			Cout() << "  Running: " << step.command << "\n";
			int res = system(step.command);
			sr("exit_code") = res;
			if(res != 0) {
				Cerr() << "  [!] Failed with exit code: " << res << "\n";
				success = false;
				sr("status") = "failed";
				step_results.Add(sr);
				break;
			}
			sr("status") = "success";
		} else {
			sr("status") = "skipped (no command)";
		}
		step_results.Add(sr);
	}
	
	run_meta("status") = success ? "success" : "failed";
	run_meta("steps") = step_results;
	
	StoreAsJsonFile(run_meta, AppendFileName(ops_dir, run_id + ".json"), true);
	
	if(success) Cout() << "[+] Runbook execution complete.\n";
	else Cerr() << "[!] Runbook execution failed.\n";
	Cout() << "[*] Run record saved: " << run_id << "\n";
}

static void OpsList(const Vector<String>& args) {
	String root = FindPlanRoot();
	String ops_dir = AppendFileName(root, ".maestro/ops/runs");
	
	if(!DirectoryExists(ops_dir)) {
		Cout() << "No operations run history found.\n";
		return;
	}
	
	Cout() << "Recent Operations Runs:\n";
	Cout() << "ID         | Timestamp           | Runbook    | Status\n";
	Cout() << "-----------|---------------------|------------|---------\n";
	
	FindFile ff;
	if(ff.Search(AppendFileName(ops_dir, "*.json"))) {
		do {
			Value v = ParseJSON(LoadFile(ff.GetPath()));
			if(!v.IsError() && v.Is<ValueMap>()) {
				ValueMap m = v;
				Cout() << Format("%-10s | %-19s | %-10s | %s\n", 
				                 AsString(m["id"]), 
				                 AsString(m["timestamp"]), 
				                 AsString(m["runbook_id"]), 
				                 AsString(m["status"]));
			}
		} while(ff.Next());
	}
}

static void OpsShow(const Vector<String>& args) {
	if(args.IsEmpty()) {
		Cout() << "usage: MaestroCLI ops show <run_id>\n";
		return;
	}
	
	String root = FindPlanRoot();
	String run_path = AppendFileName(root, ".maestro/ops/runs/" + args[0] + ".json");
	
	if(!FileExists(run_path)) {
		Cerr() << "Error: Run record '" << args[0] << "' not found.\n";
		return;
	}
	
	ValueMap m = ParseJSON(LoadFile(run_path));
	Cout() << "Operations Run Detail: " << m["id"] << "\n";
	Cout() << "Runbook:   " << m["title"] << " (" << m["runbook_id"] << ")\n";
	Cout() << "Timestamp: " << m["timestamp"] << "\n";
	Cout() << "Status:    " << m["status"] << "\n\n";
	
	Cout() << "Steps:\n";
	ValueArray steps = m["steps"];
	for(int i = 0; i < steps.GetCount(); i++) {
		ValueMap s = steps[i];
		Cout() << " " << s["n"] << ". " << s["action"] << " -> " << s["status"];
		if(!s["exit_code"].IsVoid()) Cout() << " (exit: " << s["exit_code"] << ")";
		Cout() << "\n";
	}
}

void OpsCommand::Execute(const Vector<String>& args) {
	if(args.IsEmpty()) { ShowHelp(); return; }
	
	String sub = args[0];
	Vector<String> sub_args;
	for(int i = 1; i < args.GetCount(); i++) sub_args.Add(args[i]);
	
	if(sub == "doctor") Doctor(sub_args);
	else if(sub == "run") RunOps(sub_args);
	else if(sub == "list" || sub == "ls") OpsList(sub_args);
	else if(sub == "show" || sub == "sh") OpsShow(sub_args);
	else {
		Cerr() << "Unknown ops subcommand: " << sub << "\n";
		ShowHelp();
	}
}

}