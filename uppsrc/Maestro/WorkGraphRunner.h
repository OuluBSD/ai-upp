#ifndef MAESTRO_WORKGRAPHRUNNER_H
#define MAESTRO_WORKGRAPHRUNNER_H

// NOTE: This header is normally included inside namespace Upp

#ifndef _Maestro_WorkGraphRunner_h_
#define _Maestro_WorkGraphRunner_h_

#include <Maestro/PlanModels.h>

namespace Upp {

struct RunSummary {
	String run_id;
	String workgraph_id;
	int    tasks_completed = 0;
	int    tasks_failed = 0;
	int    tasks_skipped = 0;
	bool   dry_run = true;
};

class WorkGraphRunner {
	WorkGraph& wg;
	bool dry_run = true;
	bool verbose = false;
	
	Index<String> completed_tasks;
	Index<String> failed_tasks;
	Index<String> skipped_tasks;

public:
	WorkGraphRunner(WorkGraph& wg, bool dry_run = true, bool verbose = false)
		: wg(wg), dry_run(dry_run), verbose(verbose) {}

	RunSummary Run();

private:
	bool ExecuteTask(const WorkGraphTask& task);
	bool ExecuteCommand(const String& cmd, const String& expect);
	bool CheckFile(const String& path, const String& expect);
	
	VectorMap<String, Index<String>> BuildDependencyGraph();
	Array<const WorkGraphTask*> GetRunnableTasks(const VectorMap<String, Index<String>>& deps);
};

}

#endif

#endif // MAESTRO_WORKGRAPHRUNNER_H
