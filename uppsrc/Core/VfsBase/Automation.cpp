#include "VfsBase.h"

NAMESPACE_UPP

AutomationElement& AutomationVisitor::AddElement(const char *text)
{
	AutomationElement& el = elements.Add();
	el.text = text;
	el.path = current_path;
	if(!el.text.IsEmpty()) {
		if(!el.path.IsEmpty()) el.path << "/";
		el.path << el.text;
	}
	return el;
}

Visitor& AutomationVisitor::AccessAction(const char *text, Event<> cb)
{
	if(found && write_mode) return *this;
	String t = text;
	if(t.IsEmpty()) {
		// This is tricky as we don't have access to Ctrl here.
		// But usually Bar::Item or similar caller passes text.
	}
	AutomationElement& el = AddElement(t);
	if(write_mode && (el.path == target_path || el.text == target_path)) {
		found = true;
		if(target_action && cb) cb();
	}
	return *this;
}

Visitor& AutomationVisitor::AccessOption(bool check, const char *text, Event<> cb)
{
	if(found && write_mode) return *this;
	AutomationElement& el = AddElement(text);
	el.checked = check;
	if(write_mode && (el.path == target_path || el.text == target_path)) {
		found = true;
		if(target_action && cb) cb();
	}
	return *this;
}

Visitor& AutomationVisitor::AccessMenu(const char *text, Event<Visitor&> proc)
{
	if(found && write_mode) return *this;
	AutomationElement& el = AddElement(text);
	el.is_menu = true;
	
	String old_path = current_path;
	current_path = el.path;
	proc(*this);
	current_path = old_path;
	
	return *this;
}

Visitor& AutomationVisitor::AccessLabel(const char *text)
{
	if(found && write_mode) return *this;
	AutomationElement& el = AddElement(text);
	if(write_mode && (el.path == target_path || el.text == target_path)) {
		found = true;
	}
	return *this;
}

Visitor& AutomationVisitor::AccessValue(const ::Upp::Value& v)
{
	if(elements.GetCount())
		elements.Top().value = v;
	return *this;
}

AutomationVisitor::AutomationVisitor()
{
}

END_UPP_NAMESPACE