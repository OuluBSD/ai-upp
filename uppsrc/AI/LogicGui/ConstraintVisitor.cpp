#include "ConstraintVisitor.h"

namespace Upp {

static String Sanitize(const char *s)
{
	String r;
	if(!s) return r;
	while(*s) {
		if(IsAlNum(*s))
			r.Cat(ToLower(*s));
		else if(IsSpace(*s) || *s == '_')
			r.Cat('_');
		s++;
	}
	return r;
}

ConstraintVisitor::ConstraintVisitor()
{
	mode = MODE_CONSTRAINT;
	storing = true;
}

Visitor& ConstraintVisitor::AccessLabel(const char *text)
{
	String name = Sanitize(text);
	current_ctrl = name;
	facts.FindAdd(Format("Label(%s)", name));
	if(ctrl->IsVisible()) facts.FindAdd(Format("Visible(%s)", name));
	if(ctrl->IsEnabled()) facts.FindAdd(Format("Enabled(%s)", name));
	return *this;
}

Visitor& ConstraintVisitor::AccessAction(const char *text, Event<> cb)
{
	String name = Sanitize(text);
	facts.FindAdd(Format("Button(%s)", name));
	if(ctrl->IsVisible()) facts.FindAdd(Format("Visible(%s)", name));
	if(ctrl->IsEnabled()) facts.FindAdd(Format("Enabled(%s)", name));
	return *this;
}

Visitor& ConstraintVisitor::AccessOption(bool check, const char *text, Event<> cb)
{
	String name = Sanitize(text);
	facts.FindAdd(Format("Option(%s)", name));
	if(check) facts.FindAdd(Format("Checked(%s)", name));
	if(ctrl->IsVisible()) facts.FindAdd(Format("Visible(%s)", name));
	if(ctrl->IsEnabled()) facts.FindAdd(Format("Enabled(%s)", name));
	return *this;
}

Visitor& ConstraintVisitor::AccessValue(const Value& v)
{
	if(!current_ctrl.IsEmpty()) {
		if(!v.IsNull())
			facts.FindAdd(Format("HasValue(%s)", current_ctrl));
		else
			facts.FindAdd(Format("IsNull(%s)", current_ctrl));
	}
	return *this;
}

void ConstraintVisitor::CollectFacts(Ctrl& c)
{
	ctrl = &c;
	c.Access(*this);
	for(Ctrl *child = c.GetFirstChild(); child; child = child->GetNext()) {
		CollectFacts(*child);
	}
}

}