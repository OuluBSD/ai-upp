#include "VfsShell.h"
#include <Core/Core.h>
#include <Core/VfsBase/VfsBase.h>  // For VFS mounting functionality
#include <ide/CommandLineHandler.h>
#include <iostream>

using namespace std;

using namespace Upp;

// Initialize MountManager with system filesystem mounted at root
void InitializeMountSystem() {
    MountManager& mm = MountManager::System();

    // Mount system filesystem at root "/"
    SystemFS* sysfs = new SystemFS();
    mm.Mount("/", sysfs, "sysfs");

    // This makes the system filesystem available at the root path
    // Additional VFS mounts can be added here for "/vfs/" path or other locations
}

namespace {

Vector<String> ToStringVector(const ValueArray& args, int start_index = 0)
{
	Vector<String> out;
	for(int i = start_index; i < args.GetCount(); i++)
		out.Add((String)args[i]);
	return out;
}

}

// Simple implementation of VfsShellHostBase for console application
class VfsShellHost : public VfsShellHostBase {
private:
	String output;
	int last_exit_code;
	
public:
	VfsShellHost() : last_exit_code(0) {} // Initialize to success
	
	bool Command(VfsShellConsole& shell, const ValueArray& args) override {
		if (args.GetCount() == 0) return false;

		String cmd = args[0];
		if (cmd == "help") {
			shell.CmdHelp(args);
			return true;
		}
		else if (cmd == "pwd") {
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
			// For true/false, we need to handle exit status properly
			if (cmd == "true") {
				this->SetExitCode(0);  // Success
			} else { // cmd == "false"
				this->SetExitCode(1);  // Failure
			}
			shell.CmdTrueFalse(args);
			return true;
		}
		else if (cmd == "echo") {
			shell.CmdEcho(args);
			return true;
		}
		else if (cmd == "idebuild") {
			shell.CmdIdeBuild(args);
			return true;
		}
		else if (cmd == "ideworkspace") {
			shell.CmdIdeWorkspace(args);
			return true;
		}
		else if (cmd == "idepkg") {
			shell.CmdIdePackage(args);
			return true;
		}
		else if (cmd == "ideinstall") {
			shell.CmdIdeInstall(args);
			return true;
		}
		else if (cmd == "ideuninstall") {
			shell.CmdIdeUninstall(args);
			return true;
		}
		else if (cmd == "theide") {
			Vector<String> ide_args = ToStringVector(args, 1);
			if(!HandleConsoleIdeArgs(ide_args))
				AddOutputLine(GetConsoleIdeExperimentalNotice());
			return true;
		}
		else if (cmd == "upp.builder.load") {
			shell.CmdUppBuilderLoad(args);
			return true;
		}
		else if (cmd == "upp.builder.add") {
			shell.CmdUppBuilderAdd(args);
			return true;
		}
		else if (cmd == "upp.builder.list") {
			shell.CmdUppBuilderList(args);
			return true;
		}
		else if (cmd == "upp.builder.active.set") {
			shell.CmdUppBuilderActiveSet(args);
			return true;
		}
		else if (cmd == "upp.builder.get") {
			shell.CmdUppBuilderGet(args);
			return true;
		}
		else if (cmd == "upp.builder.set") {
			shell.CmdUppBuilderSet(args);
			return true;
		}
		else if (cmd == "upp.builder.dump") {
			shell.CmdUppBuilderDump(args);
			return true;
		}
		else if (cmd == "upp.builder.active.dump") {
			shell.CmdUppBuilderActiveDump(args);
			return true;
		}
		else if (cmd == "upp.startup.load") {
			shell.CmdUppStartupLoad(args);
			return true;
		}
		else if (cmd == "upp.startup.list") {
			shell.CmdUppStartupList(args);
			return true;
		}
		else if (cmd == "upp.startup.open") {
			shell.CmdUppStartupOpen(args);
			return true;
		}
		else if (cmd == "upp.asm.load") {
			shell.CmdUppAsmLoad(args);
			return true;
		}
		else if (cmd == "upp.asm.create") {
			shell.CmdUppAsmCreate(args);
			return true;
		}
		else if (cmd == "upp.asm.list") {
			shell.CmdUppAsmList(args);
			return true;
		}
		else if (cmd == "upp.asm.scan") {
			shell.CmdUppAsmScan(args);
			return true;
		}
		else if (cmd == "upp.asm.load.host") {
			shell.CmdUppAsmLoadHost(args);
			return true;
		}
		else if (cmd == "upp.asm.refresh") {
			shell.CmdUppAsmRefresh(args);
			return true;
		}
		else if (cmd == "upp.wksp.open") {
			shell.CmdUppWkspOpen(args);
			return true;
		}
		else if (cmd == "upp.wksp.close") {
			shell.CmdUppWkspClose(args);
			return true;
		}
		else if (cmd == "upp.wksp.pkg.list") {
			shell.CmdUppWkspPkgList(args);
			return true;
		}
		else if (cmd == "upp.wksp.pkg.set") {
			shell.CmdUppWkspPkgSet(args);
			return true;
		}
		else if (cmd == "upp.wksp.file.list") {
			shell.CmdUppWkspFileList(args);
			return true;
		}
		else if (cmd == "upp.wksp.file.set") {
			shell.CmdUppWkspFileSet(args);
			return true;
		}
		else if (cmd == "upp.wksp.build") {
			shell.CmdUppWkspBuild(args);
			return true;
		}
		else if (cmd == "upp.gui") {
			shell.CmdUppGui(args);
			return true;
		}
		else if (cmd == "parse.file") {
			shell.CmdParseFile(args);
			return true;
		}
		else if (cmd == "parse.dump") {
			shell.CmdParseDump(args);
			return true;
		}
		else if (cmd == "parse.generate") {
			shell.CmdParseGenerate(args);
			return true;
		}

		return false; // Command not handled
	}

	String GetOutput() const override {
		return output;
	}

	void ClearOutput() override {
		output.Clear();
	}
	
	int GetLastExitCode() const override {
		return last_exit_code;
	}
	
	void SetExitCode(int code) override {
		last_exit_code = code;
	}

	void AddOutput(const String& s) override {
		output << s;
	}

	void AddOutputLine(const String& s) override {
		output << s << "\n";
	}
};

int main(int argc, char* argv[])
{
	using namespace Upp;

	// Initialize the mount system before creating VfsShell
	InitializeMountSystem();

	if(argc > 1 && String(argv[1]).IsEqual("theide")) {
		Vector<String> ide_args;
		for(int i = 2; i < argc; i++)
			ide_args.Add(String(argv[i]));
		if(!HandleConsoleIdeArgs(ide_args))
			Cout() << GetConsoleIdeExperimentalNotice() << "\n";
		return 0;
	}

	// Check for help option
	for (int i = 1; i < argc; i++) {
		String arg = String(argv[i]);
		if (arg == "-h" || arg == "--help") {
			Cout() << "VFS Shell - A virtual file system shell supporting csh/tcsh syntax\n";
			Cout() << "Usage: VfsShell [options] [command]\n";
			Cout() << "Options:\n";
			Cout() << "  -h, --help     Show this help message\n";
			Cout() << "  -l, --login    Start as a login shell (read init files)\n";
			Cout() << "Commands:\n";
			Cout() << "  Supports standard commands like ls, cd, pwd, etc.\n";
			Cout() << "  Supports csh syntax: pipes (a | b), logical AND (a && b), logical OR (a || b)\n";
			Cout() << "  Example: ls | grep \"txt\" && echo \"Found text files\" || echo \"No text files\"\n";
			return 0;
		}
	}

	VfsShellHost host;
	VfsShellConsole console(host);

	// Check for login shell flag to load initialization files
	bool is_login_shell = false;
	for (int i = 1; i < argc; i++) {
		String arg = String(argv[i]);
		if (arg == "-l" || arg == "--login") {
			is_login_shell = true;
			break;
		}
	}

	// Load initialization files if this is a login shell or if no command line args
	if (is_login_shell || argc == 1) {
		// Try to load ~/.vfshrc first, then ~/.cshrc as fallback
		String home_dir = GetEnv("HOME");
		if (!home_dir.IsEmpty()) {
			String vfshrc_path = AppendFileName(home_dir, ".vfshrc");
			if (FileExists(vfshrc_path)) {
				Cout() << "Loading initialization file: " << vfshrc_path << "\n";
				// Execute the initialization file
				console.ExecuteFile(vfshrc_path);
			} else {
				String cshrc_path = AppendFileName(home_dir, ".cshrc");
				if (FileExists(cshrc_path)) {
					Cout() << "Loading initialization file: " << cshrc_path << "\n";
					// Execute the initialization file
					console.ExecuteFile(cshrc_path);
				}
			}
		}
	}

	// For console application, just process commands from standard input or arguments
	if (argc > 1) {
		// Check if this is just running a single command and not the interactive shell
		bool is_single_command = true;
		for (int i = 1; i < argc; i++) {
			String arg = String(argv[i]);
			if (arg == "-l" || arg == "--login") {
				is_single_command = false;  // If login flag is present, might be interactive
			}
		}
		
		if (is_single_command) {
			// Process command line arguments as a command
			String full_command;
			for (int i = 1; i < argc; i++) {
				String arg = String(argv[i]);
				// Skip flags like -h, -l
				if (arg != "-h" && arg != "--help" && arg != "-l" && arg != "--login") {
					if (!full_command.IsEmpty()) full_command << " ";
					full_command << arg;
				}
			}
			
			if (!full_command.IsEmpty()) {
				// Store the command in the console's output field and execute
				console.output = full_command;
				console.Execute();
			}
		} else {
			// Interactive shell mode with potential init file loading
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

				// Set the input and execute
				console.output = input;
				console.Execute();
			}
		}
	} else {
		// Interactive shell mode with init file loading
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

			// Set the input and execute
			console.output = input;
			console.Execute();
		}
	}
	return 0;
}