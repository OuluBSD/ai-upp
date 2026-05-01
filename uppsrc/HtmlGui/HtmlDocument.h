#ifndef _HtmlGui_HtmlDocument_h_
#define _HtmlGui_HtmlDocument_h_

#include "ElWidget.h"

NAMESPACE_UPP

class HtmlDocument : public Layout::Document {
public:
	HtmlDocument(Layout::HtmlCtrl* ctrl) : Layout::Document(ctrl) {}

	virtual Layout::Element* CreateElement(Layout::HtmlNode& n) override;
};

END_UPP_NAMESPACE

#endif
