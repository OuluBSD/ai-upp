#include "Maestro.h"

namespace Upp {

void RunbookCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI runbook [-h]\n"
	       << "                          {list,ls,show,sh,add,new,edit,e,rm,remove,delete,step-add,sa,step-edit,se,step-rm,sr,step-renumber,srn,export,exp,render,rnd,discuss,d,resolve,res,archive,restore}\n"
	       << "                          ...\n"
	       << "Manage runbook entries as first-class project assets. Runbooks provide a narrative-first\n"
	       << "modeling layer before formalization.\n"
	       << "positional arguments:\n"
	       << "  {list,ls,show,sh,add,new,edit,e,rm,remove,delete,step-add,sa,step-edit,se,step-rm,sr,step-renumber,srn,export,exp,render,rnd,discuss,d,resolve,res,archive,restore}\n"
	       << "                        Runbook subcommands\n"
	       << "    list (ls)           List all runbooks\n"
	       << "    show (sh)           Show a specific runbook\n"
	       << "    add (new)           Create a new runbook\n"
	       << "    edit (e)            Edit a runbook\n"
	       << "    rm (remove, delete)\n"
	       << "                        Delete a runbook\n"
	       << "    step-add (sa)       Add a step to a runbook\n"
	       << "    step-edit (se)      Edit a step in a runbook\n"
	       << "    step-rm (sr)        Remove a step from a runbook\n"
	       << "    step-renumber (srn)\n"
	       << "                        Renumber steps in a runbook\n"
	       << "    export (exp)        Export a runbook\n"
	       << "    render (rnd)        Render a runbook PUML to SVG\n"
	       << "    discuss (d)         Discuss runbook with AI (placeholder)\n"
	       << "    resolve (res)       Resolve freeform text to structured runbook JSON\n"
	       << "    archive             Archive a runbook (markdown or JSON)\n"
	       << "    restore             Restore an archived runbook\n"
	       << "options:\n"
	       << "  -h, --help            show this help message and exit\n";
}

void RunbookCommand::Execute(const Vector<String>& args) {
	Cout() << "Command 'runbook' is not yet fully implemented in C++.\n";
}

} // namespace Upp
