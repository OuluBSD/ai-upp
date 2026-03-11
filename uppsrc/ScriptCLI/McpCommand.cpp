#include "ScriptCLI.h"
#include <iostream>

NAMESPACE_UPP

enum {
	JSONRPC_PARSE_ERROR = -32700,
	JSONRPC_INVALID_REQUEST = -32600,
	JSONRPC_METHOD_NOT_FOUND = -32601,
	JSONRPC_INVALID_PARAMS = -32602,
	JSONRPC_INTERNAL_ERROR = -32603,
};

static ValueMap MakeJsonRpcResult(const Value& id, const Value& result)
{
	ValueMap out;
	out.Add("jsonrpc", "2.0");
	out.Add("id", id);
	out.Add("result", result);
	return out;
}

static ValueMap MakeJsonRpcError(const Value& id, int code, const String& msg)
{
	ValueMap err;
	err.Add("code", code);
	err.Add("message", msg);

	ValueMap out;
	out.Add("jsonrpc", "2.0");
	out.Add("id", id);
	out.Add("error", err);
	return out;
}

static bool IsNotification(const ValueMap& req)
{
	return req.Find("id") < 0;
}

static String ResolveWorkspacePath(const String& workspace, const String& path)
{
	if(path.IsEmpty())
		return path;
	if(IsFullPath(path))
		return NormalizePath(path);
	return NormalizePath(AppendFileName(workspace, path));
}

static ValueMap HandleMcpMethod(ValueMap req, const String& workspace)
{
	Value id = req.Get("id", Value());
	String method = req.Get("method", "");
	Value params = req.Get("params", ValueMap());

	if(method == "mcp.ping") {
		ValueMap r;
		r.Add("text", "pong");
		r.Add("workspace", workspace);
		return MakeJsonRpcResult(id, r);
	}

	if(method == "mcp.capabilities") {
		ValueArray methods;
		methods.Add("mcp.ping");
		methods.Add("mcp.capabilities");
		methods.Add("workspace.info");
		methods.Add("script.run");
		methods.Add("script.lint");

		ValueMap caps;
		caps.Add("protocol", "jsonrpc-2.0");
		caps.Add("transport", "stdio");
		caps.Add("methods", methods);
		return MakeJsonRpcResult(id, caps);
	}

	if(method == "workspace.info") {
		ValueMap info;
		info.Add("workspace", workspace);
		info.Add("cwd", GetCurrentDirectory());
		return MakeJsonRpcResult(id, info);
	}

	if(method == "script.run") {
		if(!IsValueMap(params))
			return MakeJsonRpcError(id, JSONRPC_INVALID_PARAMS, "params must be object");
		ValueMap p = params;
		String path = ResolveWorkspacePath(workspace, p.Get("path", ""));
		if(path.IsEmpty())
			return MakeJsonRpcError(id, JSONRPC_INVALID_PARAMS, "path is required");
		if(!FileExists(path))
			return MakeJsonRpcError(id, JSONRPC_INVALID_PARAMS, "file not found: " + path);

		String code = LoadFile(path);
		PyVM vm;
		RunManager run(vm);
		bool had_error = false;
		String err_msg;
		run.WhenError = [&](const String& err) {
			had_error = true;
			err_msg = err;
		};
		run.Run(code, path);

		if(had_error)
			return MakeJsonRpcError(id, JSONRPC_INTERNAL_ERROR, err_msg);

		ValueMap out;
		out.Add("ok", true);
		out.Add("path", path);
		return MakeJsonRpcResult(id, out);
	}

	if(method == "script.lint") {
		if(!IsValueMap(params))
			return MakeJsonRpcError(id, JSONRPC_INVALID_PARAMS, "params must be object");
		ValueMap p = params;
		String path = ResolveWorkspacePath(workspace, p.Get("path", ""));
		if(path.IsEmpty())
			return MakeJsonRpcError(id, JSONRPC_INVALID_PARAMS, "path is required");
		if(!FileExists(path))
			return MakeJsonRpcError(id, JSONRPC_INVALID_PARAMS, "file not found: " + path);

		String code = LoadFile(path);
		Linter lint;
		Vector<Linter::Message> msgs = lint.Analyze(code, path);

		ValueArray issues;
		for(const auto& m : msgs) {
			ValueMap issue;
			issue.Add("line", m.line);
			issue.Add("column", m.column);
			issue.Add("severity", m.is_error ? "error" : "warning");
			issue.Add("text", m.text);
			issues.Add(issue);
		}

		ValueMap out;
		out.Add("path", path);
		out.Add("issues", issues);
		out.Add("ok", msgs.IsEmpty());
		return MakeJsonRpcResult(id, out);
	}

	return MakeJsonRpcError(id, JSONRPC_METHOD_NOT_FOUND, "Method not found: " + method);
}

static int RunMcpStdioServer(const String& workspace)
{
	std::string line;
	while(std::getline(std::cin, line)) {
		String req_json(line.c_str());
		Value parsed = ParseJSON(req_json);
		if(IsNull(parsed) || !IsValueMap(parsed)) {
			ValueMap err = MakeJsonRpcError(Null, JSONRPC_PARSE_ERROR, "Parse error");
			std::cout << AsJSON(err).ToStd() << "\n" << std::flush;
			continue;
		}

		ValueMap req = parsed;
		if(req.Get("jsonrpc", "") != "2.0" || req.Find("method") < 0) {
			ValueMap err = MakeJsonRpcError(req.Get("id", Null), JSONRPC_INVALID_REQUEST, "Invalid request");
			std::cout << AsJSON(err).ToStd() << "\n" << std::flush;
			continue;
		}

		if(IsNotification(req)) {
			continue;
		}

		ValueMap response = HandleMcpMethod(req, workspace);
		std::cout << AsJSON(response).ToStd() << "\n" << std::flush;
	}
	return SCRIPTCLI_OK;
}

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

	if(transport == "stdio")
		return RunMcpStdioServer(workspace);

	if(transport == "tcp") {
		Cerr() << "mcp serve: tcp transport not implemented yet (requested port " << port << ")\n";
		return SCRIPTCLI_INFRA_ERROR;
	}

	Cerr() << "mcp serve: invalid transport: " << transport << "\n";
	return SCRIPTCLI_USAGE_ERROR;
}

END_UPP_NAMESPACE
