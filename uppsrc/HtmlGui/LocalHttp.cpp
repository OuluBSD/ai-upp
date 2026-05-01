#include "LocalHttp.h"

NAMESPACE_UPP

static VectorMap<String, Event<const String&, const String&, String&>>& Handlers()
{
	static VectorMap<String, Event<const String&, const String&, String&>> h;
	return h;
}

void LocalHttp::RegisterHandler(const String& path, Event<const String&, const String&, String&> handler)
{
	Handlers().GetAdd(path) = handler;
}

String LocalHttp::Dispatch(const String& url, const String& method, const String& data)
{
	// Simple path-based dispatch
	for(int i = 0; i < Handlers().GetCount(); i++) {
		if(url.StartsWith(Handlers().GetKey(i))) {
			String response;
			Handlers()[i](url, data, response);
			return response;
		}
	}
	return "404 Not Found";
}

END_UPP_NAMESPACE
