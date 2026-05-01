#ifndef _HtmlGui_LocalHttp_h_
#define _HtmlGui_LocalHttp_h_

#include <Core/Core.h>

NAMESPACE_UPP
#include <Core/Inet.h>
END_UPP_NAMESPACE

NAMESPACE_UPP

class LocalHttp {
public:
	static String Dispatch(const String& url, const String& method, const String& data);
	static void   RegisterHandler(const String& path, Event<const String&, const String&, String&> handler);
};

END_UPP_NAMESPACE

#endif
