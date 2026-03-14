#ifndef _ScriptCommon_ScriptMcpHandlers_h_
#define _ScriptCommon_ScriptMcpHandlers_h_

struct ScriptMcpContext {
	String          workspace;
	String          transport;
	ScriptServices* services = nullptr;
};

bool HandleScriptMcpMethod(const String& method, const Value& params, const ScriptMcpContext& ctx,
                           Value& result, int& error_code, String& error_message);

#endif
