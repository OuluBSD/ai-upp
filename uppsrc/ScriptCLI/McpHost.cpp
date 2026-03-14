#include "ScriptCLI.h"
#include <iostream>

NAMESPACE_UPP

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

int McpHost::Run(const McpHostConfig& config)
{
	String transport = NormalizeScriptMcpTransport(config.transport);
	if(transport == "stdio")
		return RunStdio(config);

	if(transport == "tcp") {
		Cerr() << "mcp serve: tcp transport not implemented yet (requested port " << config.port << ")\n";
		return SCRIPTCLI_INFRA_ERROR;
	}

	Cerr() << "mcp serve: invalid transport: " << transport << "\n";
	return SCRIPTCLI_USAGE_ERROR;
}

int McpHost::RunStdio(const McpHostConfig& config)
{
	ScriptServices services(config.workspace);
	ScriptMcpContext ctx;
	ctx.workspace = services.GetWorkspace();
	ctx.transport = "stdio";
	ctx.services = &services;

	std::string line;
	while(std::getline(std::cin, line)) {
		String req_json(line.c_str());
		Value parsed = ParseJSON(req_json);
		if(IsNull(parsed) || !IsValueMap(parsed)) {
			ValueMap err = MakeJsonRpcError(Null, SCRIPTMCP_PARSE_ERROR, "Parse error");
			std::cout << AsJSON(err).ToStd() << "\n" << std::flush;
			continue;
		}

		ValueMap req = parsed;
		if(req.Get("jsonrpc", "") != "2.0" || req.Find("method") < 0) {
			ValueMap err = MakeJsonRpcError(req.Get("id", Null), SCRIPTMCP_INVALID_REQUEST, "Invalid request");
			std::cout << AsJSON(err).ToStd() << "\n" << std::flush;
			continue;
		}

		if(IsNotification(req))
			continue;

		Value result;
		int error_code = 0;
		String error_message;
		String method = req.Get("method", "");
		bool handled = HandleScriptMcpMethod(method, req.Get("params", Value()), ctx, result, error_code, error_message);

		ValueMap response;
		if(!handled)
			response = MakeJsonRpcError(req.Get("id", Null), SCRIPTMCP_METHOD_NOT_FOUND, "Method not found: " + method);
		else if(error_code)
			response = MakeJsonRpcError(req.Get("id", Null), error_code, error_message);
		else
			response = MakeJsonRpcResult(req.Get("id", Null), result);

		std::cout << AsJSON(response).ToStd() << "\n" << std::flush;
	}
	return SCRIPTCLI_OK;
}

END_UPP_NAMESPACE
