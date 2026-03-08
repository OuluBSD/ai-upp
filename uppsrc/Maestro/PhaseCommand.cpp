#include "Maestro.h"

namespace Upp {

void PhaseCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI phase [-h] [--mode {editor,terminal}] [--dry-run] [-v]\n"
	       << "                        {list,ls,l,add,a,remove,rm,r,help,h,show,sh,edit,e,status,set-status,text,raw,set-text,setraw,discuss,d,set,st}\n"
	       << "                        ...\n"
	       << "positional arguments:\n"
	       << "  {list,ls,l,add,a,remove,rm,r,help,h,show,sh,edit,e,status,set-status,text,raw,set-text,setraw,discuss,d,set,st}\n"
	       << "                        Phase subcommands\n"
	       << "    list (ls, l)        List all phases (or phases in track)\n"
	       << "    add (a)             Add new phase\n"
	       << "    remove (rm, r)      Remove a phase\n"
	       << "    help (h)            Show help for phase commands\n"
	       << "    show (sh)           Show phase details\n"
	       << "    edit (e)            Edit phase in $EDITOR\n"
	       << "    status (set-status)\n"
	       << "                        Update phase status\n"
	       << "    text (raw)          Show raw phase JSON payload\n"
	       << "    set-text (setraw)   Replace phase block from stdin or a file\n"
	       << "    discuss (d)         Discuss phase with AI\n"
	       << "    set (st)            Set current phase context\n"
	       << "options:\n"
	       << "  -h, --help            show this help message and exit\n"
	       << "  --mode {editor,terminal}\n"
	       << "                        Discussion mode (editor or terminal)\n"
	       << "  --dry-run             Preview actions without executing them\n"
	       << "  -v, --verbose         Show detailed debug information including parsing failures\n";
}

void PhaseCommand::Execute(const Vector<String>& args) {
	Cout() << "Command 'phase' is not yet fully implemented in C++.\n";
}

} // namespace Upp
