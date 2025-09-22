#ifndef _ide_MCP_Protocol_h_
#define _ide_MCP_Protocol_h_


struct McpConfig {
	
};

struct McpRequest {
    String id;
    String method;
    Value  params; // JSON object
};

struct McpResponse {
    String id;
    Value  result; // or error
    bool   is_error = false;
};

inline bool ParseRequest(const String& json, McpRequest& out) {
    Value v = ParseJSON(json);
    if(!IsValueMap(v)) return false;
    ValueMap m = v;
    out.id = AsString(m.Get("id", Value()));
    out.method = AsString(m.Get("method", Value()));
    out.params = m.Get("params", Value());
    return true;
}

inline String MakeResult(const String& id, const Value& result) {
    ValueMap m;
    m.Add("jsonrpc", "2.0");
    m.Add("id", id);
    m.Add("result", result);
    return AsJSON(m);
}

inline String MakeError(const String& id, int code, const String& message) {
    ValueMap err;
    err.Add("code", code);
    err.Add("message", message);
    ValueMap m;
    m.Add("jsonrpc", "2.0");
    m.Add("id", id);
    m.Add("error", err);
    return AsJSON(m);
}

#endif
