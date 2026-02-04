#include "WorkGraphRunner.h"

namespace Upp {

RunSummary WorkGraphRunner::Run() {
	RunSummary summary;
	summary.workgraph_id = wg.id;
	summary.dry_run = dry_run;
	
	VectorMap<String, Index<String>> deps = BuildDependencyGraph();
	
	bool progress = true;
	while(progress) {
		progress = false;
		Array<const WorkGraphTask*> runnable = GetRunnableTasks(deps);
		
		for(const auto* task : runnable) {
			if(ExecuteTask(*task)) {
				completed_tasks.Add(task->id);
				summary.tasks_completed++;
				progress = true;
			} else {
				failed_tasks.Add(task->id);
				summary.tasks_failed++;
				// Stop on failure for now
				return summary;
			}
		}
	}
	
	return summary;
}

VectorMap<String, Index<String>> WorkGraphRunner::BuildDependencyGraph() {
	VectorMap<String, Index<String>> deps;
	VectorMap<String, String> output_to_task;
	
	for(const auto& p : wg.phases) {
		for(const auto& t : p.tasks) {
			for(const auto& out : t.outputs)
				output_to_task.Add(out, t.id);
		}
	}
	
	for(const auto& p : wg.phases) {
		for(const auto& t : p.tasks) {
			Index<String>& d = deps.GetAdd(t.id);
			for(const auto& in : t.inputs) {
				if(output_to_task.Find(in) >= 0)
					d.Add(output_to_task.Get(in));
			}
			// Also add explicit depends_on
			for(const auto& dep_id : t.depends_on)
				d.Add(dep_id);
		}
	}
	
	return deps;
}

Array<const WorkGraphTask*> WorkGraphRunner::GetRunnableTasks(const VectorMap<String, Index<String>>& deps) {
	Array<const WorkGraphTask*> runnable;
	for(const auto& p : wg.phases) {
		for(const auto& t : p.tasks) {
			if(completed_tasks.Find(t.id) < 0 && failed_tasks.Find(t.id) < 0 && skipped_tasks.Find(t.id) < 0) {
				bool all_done = true;
				const Index<String>& d = deps.Get(t.id);
				for(const auto& dep_id : d) {
					if(completed_tasks.Find(dep_id) < 0) {
						all_done = false;
						break;
					}
				}
				if(all_done) runnable.Add(&t);
			}
		}
	}
	return runnable;
}

bool WorkGraphRunner::ExecuteTask(const WorkGraphTask& task) {
	if(verbose) Cout() << "[" << task.id << "] " << task.title << " - " << task.intent << "\n";
	
	if(dry_run) {
		for(const auto& dod : task.definition_of_done) {
			if(dod.kind == "command") Cout() << "  [DRY] $ " << dod.cmd << "\n";
			else if(dod.kind == "file") Cout() << "  [DRY] Check file: " << dod.path << "\n";
		}
		return true;
	}
	
	for(const auto& dod : task.definition_of_done) {
		if(dod.kind == "command") {
			if(!ExecuteCommand(dod.cmd, dod.expect)) return false;
		}
		else if(dod.kind == "file") {
			if(!CheckFile(dod.path, dod.expect)) return false;
		}
	}
	
	return true;
}

bool WorkGraphRunner::ExecuteCommand(const String& cmd, const String& expect) {
	if(verbose) Cout() << "  Executing: " << cmd << "\n";
	
	LocalProcess p;
	if(!p.Start(cmd)) return false;
	
	String out;
	while(p.IsRunning()) {
		String chunk;
		if(p.Read(chunk)) {
			out << chunk;
			if(verbose) Cout() << chunk;
		}
		Sleep(10);
	}
	
	int exit_code = p.GetExitCode();
	if(verbose) Cout() << "\n  Exit code: " << exit_code << "\n";
	
	if(expect.Find("exit 0") >= 0) return exit_code == 0;
	return exit_code == 0; // Default
}

bool WorkGraphRunner::CheckFile(const String& path, const String& expect) {
	if(verbose) Cout() << "  Checking file: " << path << "\n";
	bool exists = FileExists(path) || DirectoryExists(path);
	if(expect.Find("exists") >= 0) return exists;
	return exists;
}

} 
