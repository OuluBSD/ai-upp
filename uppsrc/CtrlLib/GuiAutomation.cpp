#include "CtrlLib.h"
#include "GuiAutomation.h"

NAMESPACE_UPP

Bar::Item& AutomationBar::Item::Text(const char *text) {
	el.text = text;
	el.path = v.current_path;
	if(el.text.GetCount()) {
		if(el.path.GetCount()) el.path << "/";
		el.path << el.text;
	}

	if(v.write_mode && el.path == v.target_path) {
		v.found = true;
		if(v.target_action && callback)
			callback();
	}

	if(submenu_proc) {
		String old_path = v.current_path;
		v.current_path = el.path;
		AutomationBar ab(v);
		submenu_proc(ab);
		v.current_path = old_path;
	}
	return *this;
}

Bar::Item& AutomationBar::Item::Key(dword key) { return *this; }
Bar::Item& AutomationBar::Item::Image(const ::Upp::Image& img) { return *this; }
Bar::Item& AutomationBar::Item::Check(bool check) { el.checked = check; return *this; }
Bar::Item& AutomationBar::Item::Radio(bool check) { el.checked = check; return *this; }
Bar::Item& AutomationBar::Item::Enable(bool _enable) { el.enabled = _enable; return *this; }
Bar::Item& AutomationBar::Item::Bold(bool bold) { return *this; }
Bar::Item& AutomationBar::Item::Tip(const char *tip) { return *this; }
Bar::Item& AutomationBar::Item::Help(const char *help) { return *this; }
Bar::Item& AutomationBar::Item::Topic(const char *topic) { return *this; }
Bar::Item& AutomationBar::Item::Description(const char *desc) { return *this; }
Bar::Item& AutomationBar::Item::AccessValue(const ::Upp::Value& val) { el.value = val; return *this; }

Bar::Item& AutomationBar::AddItem(Event<> cb) {
	v.AccessAction("", cb);
	Item& it = automation_items.Add(new Item(v, v.elements.Top()));
	it.callback = cb;
	return it;
}

Bar::Item& AutomationBar::AddSubMenu(Event<Bar&> proc) {
	v.AccessAction("", []{});
	AutomationElement& el = v.elements.Top();
	el.is_menu = true;
	Item& it = automation_items.Add(new Item(v, el));
	it.submenu_proc = proc;
	return it;
}

void AutomationBar::AddCtrl(Ctrl *ctrl, int gapsize) { if(ctrl) ctrl->Access(v); }
void AutomationBar::AddCtrl(Ctrl *ctrl, Size sz) { if(ctrl) ctrl->Access(v); }
bool AutomationBar::IsEmpty() const { return false; }
void AutomationBar::Separator() {}

void GuiAutomationVisitor::Read(Ctrl& c)
{
	elements.Clear();
	current_path = "";
	write_mode = false;
	c.Access(*this);
}

::Upp::Value GuiAutomationVisitor::Read(Ctrl& c, const String& path)
{
	Read(c);
	for(const auto& el : elements)
		if(el.path == path)
			return el.value;
	return ::Upp::Value();
}

bool GuiAutomationVisitor::Write(Ctrl& c, const String& path, const ::Upp::Value& val, bool do_action)
{
	write_mode = true;
	target_path = path;
	target_value = val;
	target_action = do_action;
	found = false;
	current_path = "";
	c.Access(*this);
	return found;
}

END_UPP_NAMESPACE
