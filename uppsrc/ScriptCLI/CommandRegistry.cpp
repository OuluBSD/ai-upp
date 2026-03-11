#include "ScriptCLI.h"

NAMESPACE_UPP

void PrintScriptCliUsage()
{
	Cout() << "ScriptCLI\n";
	Cout() << "Usage:\n";
	Cout() << "  ScriptCLI run <file.py>\n";
	Cout() << "  ScriptCLI lint <file.py>\n";
	Cout() << "  ScriptCLI plugin list\n";
	Cout() << "  ScriptCLI plugin test <plugin-id> [--case <name>]\n";
	Cout() << "  ScriptCLI mcp serve [--workspace <path>] [--transport stdio|tcp] [--port <n>]\n";
	Cout() << "\n";
	Cout() << "Practical default for AI tools:\n";
	Cout() << "  ScriptCLI mcp serve\n";
	Cout() << "  (uses stdio + current directory as workspace)\n";
}

int HandleScriptCliCommand(const Vector<String>& args)
{
	if(args.IsEmpty() || args[0] == "--help" || args[0] == "-h") {
		PrintScriptCliUsage();
		return SCRIPTCLI_OK;
	}

	const String& cmd = args[0];
	if(cmd == "run")
		return RunFileCommand(args);
	if(cmd == "lint")
		return LintFileCommand(args);
	if(cmd == "plugin")
		return PluginCommand(args);
	if(cmd == "mcp")
		return McpCommand(args);

	Cerr() << "Unknown command: " << cmd << "\n";
	PrintScriptCliUsage();
	return SCRIPTCLI_USAGE_ERROR;
}

END_UPP_NAMESPACE
