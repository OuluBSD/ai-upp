#include "DomBridge.h"

NAMESPACE_UPP

DomBridge::DomBridge(JsVM& vm) : vm(vm)
{
}

void DomBridge::SetWebSocket(SimWebSocket& _ws)
{
	ws = &_ws;
	ws->WhenSend = [this](const String& msg) {
		this->OnMessage(msg);
	};
}

void DomBridge::OnMessage(const String& msg)
{
	// Here we would parse JSON from JS and call VM or update DOM
	// For now, just a placeholder
	RLOG("DomBridge received: " << msg);
}

void DomBridge::ApplyPatch(const String& patch)
{
	if(ws && ws->IsOpen()) {
		ws->WhenMessage(patch);
	}
}

END_UPP_NAMESPACE
