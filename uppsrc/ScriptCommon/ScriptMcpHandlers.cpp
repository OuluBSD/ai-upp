#include "ScriptCommon.h"

NAMESPACE_UPP

static ValueMap ToValueMap(const ScriptRunResult& r)
{
	ValueMap out;
	out.Add("ok", r.ok);
	out.Add("path", r.path);
	out.Add("kind", r.kind);
	if(!r.error.IsEmpty())
		out.Add("error", r.error);
	return out;
}

static ValueMap ToValueMap(const ScriptLintResult& r)
{
	ValueMap out;
	ValueArray issues;
	for(const auto& issue : r.issues) {
		ValueMap v;
		v.Add("line", issue.line);
		v.Add("column", issue.column);
		v.Add("severity", issue.severity);
		v.Add("text", issue.text);
		issues.Add(v);
	}
	out.Add("ok", r.ok);
	out.Add("path", r.path);
	out.Add("issues", issues);
	return out;
}

static ValueMap ToValueMap(const ScriptPluginTestResult& r)
{
	ValueMap out;
	ValueArray results;
	for(const auto& item : r.results) {
		ValueMap v;
		v.Add("name", item.name);
		v.Add("ok", item.ok);
		if(!item.message.IsEmpty())
			v.Add("message", item.message);
		results.Add(v);
	}
	out.Add("plugin_id", r.plugin_id);
	out.Add("ok", r.ok);
	out.Add("passed", r.passed);
	out.Add("failed", r.failed);
	out.Add("results", results);
	if(!r.error.IsEmpty())
		out.Add("error", r.error);
	return out;
}

static bool RequireObjectParams(const Value& params, ValueMap& out, int& error_code, String& error_message)
{
	if(IsNull(params)) {
		out = ValueMap();
		return true;
	}
	if(!IsValueMap(params)) {
		error_code = SCRIPTMCP_INVALID_PARAMS;
		error_message = "params must be object";
		return false;
	}
	out = params;
	return true;
}

bool HandleScriptMcpMethod(const String& method, const Value& params, const ScriptMcpContext& ctx,
                           Value& result, int& error_code, String& error_message)
{
	error_code = 0;
	error_message.Clear();

	ScriptServices fallback(ctx.workspace);
	const ScriptServices* services = ctx.services ? ctx.services : &fallback;

	if(method == "mcp.ping") {
		ValueMap out;
		out.Add("text", "pong");
		out.Add("workspace", services->GetWorkspace());
		result = out;
		return true;
	}

	if(method == "mcp.capabilities") {
		result = GetScriptMcpCapabilities(ctx.transport);
		return true;
	}

	if(method == "workspace.info") {
		ValueMap out;
		out.Add("workspace", services->GetWorkspace());
		out.Add("cwd", GetCurrentDirectory());
		result = out;
		return true;
	}

	if(method == "script.run") {
		ValueMap p;
		if(!RequireObjectParams(params, p, error_code, error_message))
			return true;

		String path = p.Get("path", "");
		if(path.IsEmpty()) {
			error_code = SCRIPTMCP_INVALID_PARAMS;
			error_message = "path is required";
			return true;
		}

		ScriptRunRequest req;
		req.path = path;
		ScriptRunResult run = services->RunFile(req);
		if(!run.ok) {
			error_code = run.runtime_error ? SCRIPTMCP_INTERNAL_ERROR : SCRIPTMCP_INVALID_PARAMS;
			error_message = run.error;
			return true;
		}

		result = ToValueMap(run);
		return true;
	}

	if(method == "script.lint") {
		ValueMap p;
		if(!RequireObjectParams(params, p, error_code, error_message))
			return true;

		String path = p.Get("path", "");
		if(path.IsEmpty()) {
			error_code = SCRIPTMCP_INVALID_PARAMS;
			error_message = "path is required";
			return true;
		}

		ScriptLintResult lint = services->LintFile(path);
		if(lint.path.IsEmpty()) {
			error_code = SCRIPTMCP_INVALID_PARAMS;
			error_message = "path is required";
			return true;
		}
		if(!FileExists(lint.path)) {
			error_code = SCRIPTMCP_INVALID_PARAMS;
			error_message = "file not found: " + lint.path;
			return true;
		}

		result = ToValueMap(lint);
		return true;
	}

	if(method == "plugin.list") {
		ValueArray plugins;
		for(const auto& info : services->ListPlugins()) {
			ValueMap v;
			v.Add("id", info.id);
			v.Add("name", info.name);
			v.Add("description", info.description);
			plugins.Add(v);
		}
		ValueMap out;
		out.Add("plugins", plugins);
		result = out;
		return true;
	}

	if(method == "plugin.test") {
		ValueMap p;
		if(!RequireObjectParams(params, p, error_code, error_message))
			return true;

		String plugin_id = p.Get("plugin_id", p.Get("id", ""));
		if(plugin_id.IsEmpty()) {
			error_code = SCRIPTMCP_INVALID_PARAMS;
			error_message = "plugin_id is required";
			return true;
		}

		String case_name = p.Get("case", "");
		ScriptPluginTestResult test = services->TestPlugin(plugin_id, case_name);
		if(!test.error.IsEmpty()) {
			error_code = test.error.StartsWith("plugin not found:") ? SCRIPTMCP_INVALID_PARAMS : SCRIPTMCP_INTERNAL_ERROR;
			error_message = test.error;
			return true;
		}

		result = ToValueMap(test);
		return true;
	}

	return false;
}

END_UPP_NAMESPACE
