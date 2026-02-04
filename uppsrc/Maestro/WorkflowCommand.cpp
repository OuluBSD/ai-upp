#include "Maestro.h"

namespace Upp {

void WorkflowCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI workflow [-h]\n"
	       << "                           {list,ls,show,sh,create,new,edit,e,delete,rm,visualize,viz,archive,restore}\n"
	       << "                           ...\n"
	       << "Manage workflow diagrams and state machines for the maestro instance.\n"
	       << "positional arguments:\n"
	       << "    list (ls)           List all workflows\n"
	       << "    show (sh)           Show a specific workflow\n"
	       << "    create (new)        Create a new workflow\n"
	       << "    edit (e)            Edit a workflow\n"
	       << "    delete (rm)         Delete a workflow\n"
	       << "    visualize (viz)     Visualize a workflow as UML diagram\n"
	       << "    archive             Archive a workflow file\n"
	       << "    restore             Restore an archived workflow\n"
	       << "options:\n"
	       << "  -h, --help            show this help message and exit\n";
}

void WorkflowCommand::Execute(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.AddPositional("subcommand", UNKNOWN_V);
	cla.AddPositional("arg1", UNKNOWN_V);
	cla.AddPositional("arg2", UNKNOWN_V);
	cla.Parse(args);
	
	if (cla.GetPositionalCount() == 0) { ShowHelp(); return; }
	
	String sub = AsString(cla.GetPositional(0));
	WorkflowManager wm;
	
	if (sub == "list" || sub == "ls") {
		Vector<String> list = wm.ListWorkflows();
		Cout() << "Found " << list.GetCount() << " workflow(s):\n";
		for(const auto& w : list) Cout() << "  " << w << "\n";
	}
	else if (sub == "show" || sub == "sh") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires workflow name.\n"; return; }
		Cout() << wm.LoadWorkflow(AsString(cla.GetPositional(1))) << "\n";
	}
	else if (sub == "create" || sub == "new") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires workflow name.\n"; return; }
		String name = AsString(cla.GetPositional(1));
		String content = "@startuml\ntitle " + name + "\nstart\nstop\n@enduml";
		if(wm.SaveWorkflow(name, content)) Cout() << "✓ Created workflow: " << name << "\n";
		else Cerr() << "Error: Failed to create workflow.\n";
	}
	else if (sub == "edit" || sub == "e") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires workflow name.\n"; return; }
		// Stub: In real implementation, this would open an editor
		Cout() << "Opening editor for " << AsString(cla.GetPositional(1)) << " (stub)...\n";
	}
	else if (sub == "delete" || sub == "rm") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires workflow name.\n"; return; }
		if(wm.DeleteWorkflow(AsString(cla.GetPositional(1)))) Cout() << "✓ Deleted workflow.\n";
		else Cerr() << "Error: Failed to delete workflow.\n";
	}
	else if (sub == "visualize" || sub == "viz") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires workflow name.\n"; return; }
		Cout() << wm.Visualize(AsString(cla.GetPositional(1))) << "\n";
	}
	else {
		Cout() << "Subcommand '" << sub << "' is not yet fully implemented in C++ but is on the roadmap.\n";
	}
}

}
