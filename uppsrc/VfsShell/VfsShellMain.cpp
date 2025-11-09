#include "VfsShell.h"
#include <Core/Core.h>
#include <iostream>

using namespace std;

using namespace Upp;

// Simple implementation of VfsShellHostBase for console application
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

	void ClearOutput() {
		output.Clear();
	}
	
private:
	String output;
};

int main(int argc, char* argv[])
{
	using namespace Upp;
	VfsShellHost host;
	VfsShellConsole console(host);
	
	// For console application, just process commands from standard input or arguments
	if (argc > 1) {
		// Process command line arguments as a command
		ValueArray args;
		for (int i = 1; i < argc; i++) {
			args.Add(String(argv[i]));
		}
		
		if (host.Command(console, args)) {
			String output = host.GetOutput();
			if (!output.IsEmpty()) {
				Cout() << output << "\n";
			}
		} else {
			Cout() << "Unknown command: " << String(argv[1]) << "\n";
		}
	} else {
		// Interactive shell mode
		Cout() << "VFS Shell - Type 'help' for available commands or 'quit' to exit\n";
		Cout() << "Current directory: " << (String)console.GetCurrentDirectory() << "\n";
		
		for (;;) {
			Cout() << console.GetCurrentDirectory() << " $ ";
			fflush(stdout);
			std::string temp_input;
			getline(std::cin, temp_input);
			String input = String(temp_input.c_str());  // Cast to std::string to use with getline
			if (input == "quit" || input == "exit") {
				break;
			}
			
			// Parse the input and execute the command
			Vector<String> parts = Split(input, " ", true, true);
			if (parts.GetCount() > 0) {
				ValueArray args;
				for (const String& part : parts) {
					args.Add(part);
				}
				
				host.ClearOutput(); // Clear previous output
				if (host.Command(console, args)) {
					String output = host.GetOutput();
					if (!output.IsEmpty()) {
						Cout() << output << "\n";
					}
				} else {
					Cout() << "Unknown command: " << parts[0] << "\n";
				}
			}
		}
	}
	return 0;
}