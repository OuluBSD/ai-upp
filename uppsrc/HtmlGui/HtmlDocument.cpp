#include "HtmlDocument.h"

NAMESPACE_UPP

Layout::Element* HtmlDocument::CreateElement(Layout::HtmlNode& n)
{
	if(n.GetText() == WString("widget")) {
		return new ElWidget();
	}
	return Layout::Document::CreateElement(n);
}

END_UPP_NAMESPACE
