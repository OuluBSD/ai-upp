#include "ScriptCommon.h"

NAMESPACE_UPP

String NormalizeScriptMcpTransport(const String& transport)
{
	String s = ToLower(transport);
	return s.IsEmpty() ? "stdio" : s;
}

Vector<String> GetScriptMcpMethodNames()
{
	Vector<String> methods;
	methods.Add("mcp.ping");
	methods.Add("mcp.capabilities");
	methods.Add("workspace.info");
	methods.Add("script.run");
	methods.Add("script.lint");
	methods.Add("plugin.list");
	methods.Add("plugin.test");
	return methods;
}

ValueMap GetScriptMcpCapabilities(const String& transport)
{
	ValueArray methods;
	for(const String& method : GetScriptMcpMethodNames())
		methods.Add(method);

	ValueMap caps;
	caps.Add("protocol", "jsonrpc-2.0");
	caps.Add("surface", "scriptcli-mcp");
	caps.Add("version", "v1");
	caps.Add("transport", NormalizeScriptMcpTransport(transport));
	caps.Add("supports_batch", false);
	caps.Add("methods", methods);
	return caps;
}

END_UPP_NAMESPACE
