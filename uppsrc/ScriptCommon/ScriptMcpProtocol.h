#ifndef _ScriptCommon_ScriptMcpProtocol_h_
#define _ScriptCommon_ScriptMcpProtocol_h_

enum ScriptMcpErrorCode {
	SCRIPTMCP_PARSE_ERROR = -32700,
	SCRIPTMCP_INVALID_REQUEST = -32600,
	SCRIPTMCP_METHOD_NOT_FOUND = -32601,
	SCRIPTMCP_INVALID_PARAMS = -32602,
	SCRIPTMCP_INTERNAL_ERROR = -32603,
};

String NormalizeScriptMcpTransport(const String& transport);

#endif
