#ifndef _HtmlGui_ElWidget_h_
#define _HtmlGui_ElWidget_h_

#include <LayoutCtrl/LayoutCtrl.h>

NAMESPACE_UPP

class ElWidget : public Layout::HtmlTag {
protected:
	Ptr<Ctrl> widget;
	String    type;

public:
	ElWidget();
	virtual ~ElWidget();

	virtual void Paint(Draw& hdc, int x, int y, const Layout::Position& clip) override;
	virtual void GetContentSize(Size& sz, int max_width) override;
	virtual void ParseAttributes() override;

	virtual Ctrl* CreateWidget(const String& type);
};

END_UPP_NAMESPACE

#endif
