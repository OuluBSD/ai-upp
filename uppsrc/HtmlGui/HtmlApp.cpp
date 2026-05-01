#include "HtmlApp.h"
#include <ByteVM/EcmaScript/DomBindings.h>

NAMESPACE_UPP

HtmlApp::HtmlApp() : doc(&html), bridge(js)
{
	Add(html.SizePos());
	html.SetDocument(&doc);
	
	bridge.SetWebSocket(ws);
	InitDomBindings(js);
}

void HtmlApp::Init()
{
}

void HtmlApp::Load(const String& url)
{
	// In native mode, we load local resources
	// In web mode, this would be a real URL
	html.SetHtml("<html><body><h1>Loading " + url + "...</h1></body></html>", url);
}

void HtmlApp::OnMessage(const String& msg)
{
	bridge.OnMessage(msg);
}

END_UPP_NAMESPACE
