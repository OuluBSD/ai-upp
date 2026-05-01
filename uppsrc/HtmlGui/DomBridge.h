#ifndef _HtmlGui_DomBridge_h_
#define _HtmlGui_DomBridge_h_

#include "SimWebSocket.h"
#include <ByteVM/EcmaScript/JsVM.h>

NAMESPACE_UPP

class DomBridge {
	JsVM& vm;
	Ptr<SimWebSocket> ws;

public:
	DomBridge(JsVM& vm);
	void SetWebSocket(SimWebSocket& _ws);
	
	void OnMessage(const String& msg);
	void ApplyPatch(const String& patch);
};

END_UPP_NAMESPACE

#endif
