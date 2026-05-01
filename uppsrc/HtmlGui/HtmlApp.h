#ifndef _HtmlGui_HtmlApp_h_
#define _HtmlGui_HtmlApp_h_

#include "HtmlDocument.h"
#include "DomBridge.h"
#include <ByteVM/EcmaScript/JsVM.h>

NAMESPACE_UPP

class HtmlApp : public TopWindow {
protected:
	Layout::HtmlCtrl html;
	HtmlDocument     doc;
	JsVM             js;
	DomBridge        bridge;
	SimWebSocket     ws;

public:
	typedef HtmlApp CLASSNAME;
	HtmlApp();
	
	virtual void  Init();
	void          Load(const String& url);
	
	void          OnMessage(const String& msg);
};

END_UPP_NAMESPACE

#endif
