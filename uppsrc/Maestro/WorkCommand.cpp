#include "Maestro.h"

namespace Upp {

void WorkCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI work [-h] [--ignore-gates]\n"
	       << "                       {any,track,phase,issue,task,discuss,analyze,fix,subwork,resume}\n"
	       << "                       ...\n"
	       << "positional arguments:\n"
	       << "    any                 Let AI select the best work item\n"
	       << "    track <id>          Work on a track\n"
	       << "    phase <id>          Work on a phase\n"
	       << "    issue <id>          Work on an issue\n"
	       << "    task <id>           Work on a task\n"
	       << "    analyze <target>    Analyze a target before work\n"
	       << "    fix <target>        Fix a target or issue\n";
}

void WorkCommand::Execute(const Vector<String>& args) {
	// Manual parsing for long args
	String sub;
	String arg1;
	String issue_id;
	bool simulate = false;
	
	int pos_count = 0;
	for(int i = 0; i < args.GetCount(); i++) {
		String arg = args[i];
		if(arg == "--simulate") simulate = true;
		else if(arg.StartsWith("--issue=")) issue_id = arg.Mid(8);
		else if(arg == "--issue" && i + 1 < args.GetCount()) {
			issue_id = args[++i];
		}
		else if(!arg.StartsWith("-")) {
			if(pos_count == 0) sub = arg;
			else if(pos_count == 1) arg1 = arg;
			pos_count++;
		}
	}
	
	if (pos_count == 0) { ShowHelp(); return; }
	
	WorkManager wm;
	
	if (sub == "any") {
		Array<WorkManager::WorkItem> items = wm.LoadAvailableWork();
		if (arg1 == "pick") {
			Array<WorkManager::WorkItem> top = wm.SelectTopWorkItems(items, 3);
			Cout() << "Top 3 recommendations:\n";
			for(int i = 0; i < top.GetCount(); i++)
				Cout() << Format("%d. [%s] %s\n", i + 1, top[i].type, top[i].name);
		} else {
			WorkManager::WorkItem best = wm.SelectBestWorkItem(items);
			if(best.id.IsEmpty()) Cout() << "No work items available.\n";
			else wm.StartWorkSession(best);
		}
	}
	else if (sub == "track" || sub == "phase" || sub == "issue" || sub == "task") {
		if(arg1.IsEmpty()) { Cerr() << "Error: Requires ID.\n"; return; }
		WorkManager::WorkItem item;
		item.id = arg1;
		item.type = sub;
		item.name = item.id;
		wm.StartWorkSession(item);
	}
	else if (sub == "analyze") {
		if(arg1.IsEmpty()) { Cerr() << "Error: Requires target.\n"; return; }
		wm.AnalyzeTarget(arg1, simulate);
	}
	else if (sub == "fix") {
		if(arg1.IsEmpty()) { Cerr() << "Error: Requires target.\n"; return; }
		wm.FixTarget(arg1, issue_id, simulate);
	}
	else {
		Cout() << "Subcommand '" << sub << "' is not yet fully implemented in C++.\n";
	}
}

}