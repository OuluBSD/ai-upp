#include "ElWidget.h"

NAMESPACE_UPP

ElWidget::ElWidget()
{
}

ElWidget::~ElWidget()
{
	if(widget) widget->Remove();
}

void ElWidget::ParseAttributes()
{
	Layout::HtmlTag::ParseAttributes();
	type = GetAttr("type");
	if(type.GetCount() && !widget) {
		widget = CreateWidget(type);
		if(widget) {
			// In native mode, we add it to the HtmlCtrl
			// GetDocument()->GetHtmlCtrl().Add(*widget);
		}
	}
}

void ElWidget::Paint(Draw& hdc, int x, int y, const Layout::Position& clip)
{
	Layout::HtmlTag::DrawBackground(hdc, x, y, clip);
	if(widget) {
		Rect r(x + pos.x, y + pos.y, x + pos.x + pos.width, y + pos.y + pos.height);
		widget->SetRect(r);
		if(!widget->GetParent()) {
			// This is a bit hacky, needs proper wiring to HtmlCtrl
		}
	}
}

void ElWidget::GetContentSize(Size& sz, int max_width)
{
	if(widget) {
		sz = widget->GetStdSize();
	} else {
		sz = Size(100, 20); // Default placeholder size
	}
}

Ctrl* ElWidget::CreateWidget(const String& type)
{
	if(type == "button") return new Button();
	if(type == "edit") return new EditField();
	return nullptr;
}

END_UPP_NAMESPACE
