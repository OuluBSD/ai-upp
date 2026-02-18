#include "ConstraintVisitor.h"
#include <CtrlLib/CtrlLib.h>

namespace Upp {

static String Sanitize(const char *text)
{
	String r;
	if(!text) return "null";
	bool first = true;
	while(*text) {
		if(IsAlNum(*text)) {
			r.Cat(first ? ToLower(*text) : *text);
			first = false;
		}
		else if(IsSpace(*text) || *text == '_') {
			// Skip space, but keep next letter as Uppercase for camelCase
			text++;
			if(*text && IsAlNum(*text))
				r.Cat(ToUpper(*text));
			else
				continue;
		}
		text++;
	}
	return r.IsEmpty() ? "item" : r;
}

static String Sanitize(const String& s)
{
	return Sanitize(~s);
}

ConstraintVisitor::ConstraintVisitor()
{
	mode = MODE_CONSTRAINT;
	storing = true;
}

Visitor& ConstraintVisitor::AccessLabel(const char *text)
{
	String name = ctrl->GetLayoutId();
	if(name.IsEmpty() || (text && *text)) name = Sanitize(text);
	current_ctrl = name;
	facts.FindAdd(Format("LABEL(%s)", name));
	if(ctrl->IsVisible()) facts.FindAdd(Format("VISIBLE(%s)", name));
	if(ctrl->IsEnabled()) facts.FindAdd(Format("ENABLED(%s)", name));
	return *this;
}

Visitor& ConstraintVisitor::AccessAction(const char *text, Event<> cb)
{
	String name = Sanitize(text);
	facts.FindAdd(Format("BUTTON(%s)", name));
	if(ctrl->IsVisible()) facts.FindAdd(Format("VISIBLE(%s)", name));
	if(ctrl->IsEnabled()) facts.FindAdd(Format("ENABLED(%s)", name));
	return *this;
}

Visitor& ConstraintVisitor::AccessOption(bool check, const char *text, Event<> cb)
{
	String name = Sanitize(text);
	facts.FindAdd(Format("OPTION(%s)", name));
	if(check) facts.FindAdd(Format("CHECKED(%s)", name));
	if(ctrl->IsVisible()) facts.FindAdd(Format("VISIBLE(%s)", name));
	if(ctrl->IsEnabled()) facts.FindAdd(Format("ENABLED(%s)", name));
	return *this;
}

Visitor& ConstraintVisitor::AccessValue(const Value& v)
{
	if(!current_ctrl.IsEmpty()) {
		if(!v.IsNull()) {
			facts.FindAdd(Format("HasValue(%s)", current_ctrl));
			String s = v.ToString();
			if(s.GetCount() > 0 && s[0] == '[') {
				// Heuristic: starts with [ is often QTF
				facts.FindAdd(Format("IS_QTF(%s)", current_ctrl));
			}
		}
		else
			facts.FindAdd(Format("IsNull(%s)", current_ctrl));
	}
	return *this;
}

void ConstraintVisitor::CollectFacts(Ctrl& c)
{
	ctrl = &c;
	c.Access(*this);

	Button* btn = dynamic_cast<Button*>(&c);
	if(btn && btn->GetRibbonMode() != Button::RIBBON_NONE) {
		String n = Sanitize(btn->GetLayoutId());
		if(n.IsEmpty())
			n = Sanitize(btn->GetLabel());
		if(n.IsEmpty())
			n = "item";

		facts.FindAdd(Format("RIBBON_BUTTON(%s)", n));
		if(btn->GetSize().cx > 0 && btn->GetSize().cy > 0)
			facts.FindAdd(Format("RIBBON_SIZE_NONZERO(%s)", n));
		if(btn->WasRibbonTextDrawn())
			facts.FindAdd(Format("RIBBON_TEXT_DRAWN(%s)", n));
		Size tsz = btn->GetRibbonTextDrawSize();
		if(tsz.cx > 0 && tsz.cy > 0)
			facts.FindAdd(Format("RIBBON_TEXT_SIZE_NONZERO(%s)", n));
		if(btn->WasRibbonImageDrawn())
			facts.FindAdd(Format("RIBBON_IMAGE_DRAWN(%s)", n));
		Size isz = btn->GetRibbonImageDrawSize();
		if(isz.cx > 0 && isz.cy > 0)
			facts.FindAdd(Format("RIBBON_IMAGE_SIZE_NONZERO(%s)", n));
	}

	for(Ctrl *child = c.GetFirstChild(); child; child = child->GetNext()) {
		CollectFacts(*child);
	}
}

}
