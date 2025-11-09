#include "VfsShell.h"
#include <CtrlLib/CtrlLib.h>

using namespace Upp;

// Simple implementation of VfsShellHostBase for testing
class VfsShellHost : public VfsShellHostBase {
public:
	bool Command(VfsShellConsole& shell, const ValueArray& args) override {
		if (args.GetCount() == 0) return false;
		
		String cmd = args[0];
		if (cmd == "pwd") {
			shell.CmdPwd();
			return true;
		}
		else if (cmd == "cd") {
			shell.CmdCd(args);
			return true;
		}
		else if (cmd == "ls") {
			shell.CmdLs(args);
			return true;
		}
		else if (cmd == "tree") {
			shell.CmdTree(args);
			return true;
		}
		else if (cmd == "mkdir") {
			shell.CmdMkdir(args);
			return true;
		}
		else if (cmd == "touch") {
			shell.CmdTouch(args);
			return true;
		}
		else if (cmd == "rm") {
			shell.CmdRm(args);
			return true;
		}
		else if (cmd == "mv") {
			shell.CmdMv(args);
			return true;
		}
		else if (cmd == "link") {
			shell.CmdLink(args);
			return true;
		}
		else if (cmd == "export") {
			shell.CmdExport(args);
			return true;
		}
		else if (cmd == "cat") {
			shell.CmdCat(args);
			return true;
		}
		else if (cmd == "grep") {
			shell.CmdGrep(args);
			return true;
		}
		else if (cmd == "rg") {
			shell.CmdRg(args);
			return true;
		}
		else if (cmd == "head") {
			shell.CmdHead(args);
			return true;
		}
		else if (cmd == "tail") {
			shell.CmdTail(args);
			return true;
		}
		else if (cmd == "uniq") {
			shell.CmdUniq(args);
			return true;
		}
		else if (cmd == "count") {
			shell.CmdCount(args);
			return true;
		}
		else if (cmd == "history") {
			shell.CmdHistory(args);
			return true;
		}
		else if (cmd == "random") {
			shell.CmdRandom(args);
			return true;
		}
		else if (cmd == "true" || cmd == "false") {
			// For true/false, we just need to handle exit status, but for now just process as command
			shell.CmdTrueFalse(args);
			return true;
		}
		else if (cmd == "echo") {
			shell.CmdEcho(args);
			return true;
		}
		
		return false; // Command not handled
	}
	
	String GetOutput() const override {
		return output;
	}
	
private:
	String output;
};

GUI_APP_MAIN
{
	VfsShellHost host;
	VfsShellConsole console(host);
	
	// Create a simple top window to host the console
	TopWindow win;
	win.Add(console.SizePos());
	win.Run();
}