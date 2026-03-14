#include "ScriptCLI.h"

NAMESPACE_UPP

static void PrintMcpServeHelp()
{
	Cout() << "ScriptCLI MCP\n";
	Cout() << "Usage:\n";
	Cout() << "  ScriptCLI mcp serve [--workspace <path>] [--transport stdio|tcp] [--port <n>]\n";
	Cout() << "\n";
	Cout() << "Defaults:\n";
	Cout() << "  --transport stdio\n";
	Cout() << "  --workspace current directory\n";
	Cout() << "\n";
	Cout() << "Practical AI integration (no port needed):\n";
	Cout() << "  command: ScriptCLI mcp serve\n";
	Cout() << "  cwd: project root to use as workspace\n";
	Cout() << "\n";
	Cout() << "Example mcp.json entry:\n";
	Cout() << "{\n";
	Cout() << "  \"servers\": {\n";
	Cout() << "    \"scriptcli\": {\n";
	Cout() << "      \"command\": \"ScriptCLI\",\n";
	Cout() << "      \"args\": [\"mcp\", \"serve\"],\n";
	Cout() << "      \"cwd\": \"/path/to/project\"\n";
	Cout() << "    }\n";
	Cout() << "  }\n";
	Cout() << "}\n";
}

int McpCommand(const Vector<String>& args)
{
	if(args.GetCount() < 2 || args[1] == "--help" || args[1] == "-h") {
		PrintMcpServeHelp();
		return SCRIPTCLI_OK;
	}

	if(args[1] != "serve") {
		Cerr() << "mcp: expected subcommand 'serve'\n";
		return SCRIPTCLI_USAGE_ERROR;
	}

	String transport = "stdio";
	String workspace = GetCurrentDirectory();
	int port = 7326;

	for(int i = 2; i < args.GetCount(); i++) {
		const String& a = args[i];
		if(a == "--workspace" && i + 1 < args.GetCount())
			workspace = args[++i];
		else if(a.StartsWith("--workspace="))
			workspace = a.Mid(12);
		else if(a == "--transport" && i + 1 < args.GetCount())
			transport = ToLower(args[++i]);
		else if(a.StartsWith("--transport="))
			transport = ToLower(a.Mid(12));
		else if(a == "--port" && i + 1 < args.GetCount())
			port = StrInt(args[++i]);
		else if(a.StartsWith("--port="))
			port = StrInt(a.Mid(7));
		else if(a == "--help" || a == "-h") {
			PrintMcpServeHelp();
			return SCRIPTCLI_OK;
		}
		else {
			Cerr() << "mcp serve: unknown argument: " << a << "\n";
			return SCRIPTCLI_USAGE_ERROR;
		}
	}

	workspace = NormalizePath(workspace);
	if(!DirectoryExists(workspace)) {
		Cerr() << "mcp serve: workspace does not exist: " << workspace << "\n";
		return SCRIPTCLI_INFRA_ERROR;
	}

	McpHostConfig config;
	config.workspace = workspace;
	config.transport = transport;
	config.port = port;

	McpHost host;
	return host.Run(config);
}

END_UPP_NAMESPACE
