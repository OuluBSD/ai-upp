#include "Maestro.h"

namespace Upp {

void TaskCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI task [-h] [--mode {editor,terminal}] [--dry-run] [-v]\n"
	       << "                       {list,ls,l,show,sh,add,a,remove,rm,r,status,set-status,text,raw,set-text,setraw,set-details,details,help,h,discuss,d}\n"
	       << "                       ...\n"
	       << "positional arguments:\n"
	       << "  {list,ls,l,show,sh,add,a,remove,rm,r,status,set-status,text,raw,set-text,setraw,set-details,details,help,h,discuss,d}\n"
	       << "                        Task subcommands\n"
	       << "    list (ls, l)        List all tasks (or tasks in phase)\n"
	       << "    show (sh)           Show task details\n"
	       << "    add (a)             Add new task\n"
	       << "    remove (rm, r)      Remove a task\n"
	       << "    status (set-status)\n"
	       << "                        Update task status\n"
	       << "    text (raw)          Show raw task block from phase file\n"
	       << "    set-text (setraw)   Replace task block from stdin or a file\n"
	       << "    set-details (details)\n"
	       << "                        Set a detail (key-value pair) for a task\n"
	       << "    help (h)            Show help for task commands\n"
	       << "    discuss (d)         Discuss task with AI\n"
	       << "options:\n"
	       << "  -h, --help            show this help message and exit\n"
	       << "  --mode {editor,terminal}\n"
	       << "                        Discussion mode (editor or terminal)\n"
	       << "  --dry-run             Preview actions without executing them\n"
	       << "  -v, --verbose         Show verbose errors and parsing details\n";
}

void TaskCommand::Execute(const Vector<String>& args) {
	Cout() << "Command 'task' is not yet fully implemented in C++.\n";
}

} // namespace Upp
