#include "Maestro.h"

namespace Upp {

void WorkflowCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI workflow [-h]\n"
	       << "                           {list,ls,show,sh,create,new,edit,e,delete,rm,visualize,viz,archive,restore}\n"
	       << "                           ...\n"
	       << "Manage workflow diagrams and state machines for the maestro instance.\n"
	       << "positional arguments:\n"
	       << "  {list,ls,show,sh,create,new,edit,e,delete,rm,visualize,viz,archive,restore}\n"
	       << "                        Workflow subcommands\n"
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
	Cout() << "Command 'workflow' is not yet fully implemented in C++.\n";
}

}