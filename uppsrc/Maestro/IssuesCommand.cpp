#include "Maestro.h"

namespace Upp {

void IssuesCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI issues [-h]\n"
	       << "                         {list,ls,show,state,rollback,react,analyze,decide,fix,add,triage,link-task,resolve,ignore,hier,convention,build,runtime,features,product,look,ux}\n"
	       << "                         ...\n"
	       << "positional arguments:\n"
	       << "  {list,ls,show,state,rollback,react,analyze,decide,fix,add,triage,link-task,resolve,ignore,hier,convention,build,runtime,features,product,look,ux}\n"
	       << "                        Issues subcommands\n"
	       << "    list (ls)           List issues\n"
	       << "    show                Show issue details\n"
	       << "    state               Update issue state\n"
	       << "    rollback            Rollback issue state\n"
	       << "    react               React to an issue and match solutions\n"
	       << "    analyze             Analyze an issue\n"
	       << "    decide              Decide what to do with an issue\n"
	       << "    fix                 Start or complete fix phase\n"
	       << "    add                 Add issue from log scan or manually\n"
	       << "    triage              Triage issues (assign severity, propose tasks)\n"
	       << "    link-task           Link issue to task\n"
	       << "    resolve             Mark issue as resolved\n"
	       << "    ignore              Ignore issue (won't block work)\n"
	       << "    hier                List hier issues\n"
	       << "    convention          List convention issues\n"
	       << "    build               List build issues\n"
	       << "    runtime             List runtime issues\n"
	       << "    features            List features issues\n"
	       << "    product             List product issues\n"
	       << "    look                List look issues\n"
	       << "    ux                  List ux issues\n"
	       << "options:\n"
	       << "  -h, --help            show this help message and exit\n";
}

void IssuesCommand::Execute(const Vector<String>& args) {
	Cout() << "Command 'issues' is not yet fully implemented in C++.\n";
}

} // namespace Upp
