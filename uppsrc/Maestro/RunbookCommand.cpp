#include "Maestro.h"

namespace Upp {

void RunbookCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI runbook [-h]\n"
	       << "                          {list,ls,show,sh,add,new,rm,remove,delete,step-add,sa,resolve,res}\n"
	       << "                          ...\n"
	       << "Manage runbook entries as first-class project assets.\n"
	       << "positional arguments:\n"
	       << "    list (ls)           List all runbooks\n"
	       << "    show (sh)           Show a specific runbook\n"
	       << "    add (new)           Create a new runbook\n"
	       << "    rm (remove, delete) Delete a runbook\n"
	       << "    step-add (sa)       Add a step to a runbook\n"
	       << "    resolve (res)       Resolve freeform text to structured runbook JSON\n";
}

void RunbookCommand::Execute(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.AddPositional("subcommand", UNKNOWN_V);
	cla.AddPositional("arg1", UNKNOWN_V);
	cla.AddPositional("arg2", UNKNOWN_V);
	cla.Parse(args);
	
	if (cla.GetPositionalCount() == 0) { ShowHelp(); return; }
	
	String sub = AsString(cla.GetPositional(0));
	RunbookManager rbm;
	
	if (sub == "list" || sub == "ls") {
		Array<Runbook> list = rbm.ListRunbooks();
		Cout() << "Found " << list.GetCount() << " runbook(s):\n";
		for(const auto& rb : list)
			Cout() << Format("  %-30s %s\n", rb.id, rb.title);
	}
	else if (sub == "show" || sub == "sh") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires runbook ID.\n"; return; }
		Runbook rb = rbm.LoadRunbook(AsString(cla.GetPositional(1)));
		Cout() << "Runbook: " << rb.title << "\nID:      " << rb.id << "\nGoal:    " << rb.goal << "\n";
		Cout() << "\nSteps (" << rb.steps.GetCount() << "):\n";
		for(const auto& s : rb.steps)
			Cout() << Format("  %d. [%s] %s (Cmd: %s)\n", s.n, s.actor, s.action, s.command);
	}
	else if (sub == "add" || sub == "new") {
		String title = cla.GetPositionalCount() > 1 ? AsString(cla.GetPositional(1)) : "New Runbook";
		Runbook rb;
		rb.title = title;
		rb.id = "rb-" + FormatIntHex(Random(), 8);
		if(rbm.SaveRunbook(rb)) Cout() << "✓ Created runbook: " << rb.id << "\n";
		else Cerr() << "Error: Failed to save runbook.\n";
	}
	else if (sub == "rm" || sub == "remove" || sub == "delete") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires runbook ID.\n"; return; }
		if(rbm.DeleteRunbook(AsString(cla.GetPositional(1)))) Cout() << "✓ Deleted runbook.\n";
		else Cerr() << "Error: Failed to delete runbook.\n";
	}
	else if (sub == "resolve" || sub == "res") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires text input.\n"; return; }
		Cout() << "Resolving runbook via AI...\n";
		Runbook rb = rbm.Resolve(AsString(cla.GetPositional(1)));
		if(rbm.SaveRunbook(rb)) Cout() << "✓ Created/Updated runbook: " << rb.id << "\n";
		else Cerr() << "Error: Failed to resolve runbook.\n";
	}
	else {
		Cout() << "Subcommand '" << sub << "' is not yet fully implemented in C++ but is on the roadmap.\n";
	}
}

} // namespace Upp