#ifndef _CtrlLib_GuiAutomation_h_
#define _CtrlLib_GuiAutomation_h_

#include <CtrlLib/Bar.h>
#include <Core/VfsBase/Automation.h>

class Ctrl;

class AutomationBar : public Bar {
public:
	struct Item : public Bar::Item {
		AutomationVisitor& v;
		AutomationElement& el;
		Event<Bar&> submenu_proc;
		Event<>     callback;

		Bar::Item& Text(const char *text) override;
		Bar::Item& Key(dword key) override;
		Bar::Item& Image(const ::Upp::Image& img) override;
		Bar::Item& Check(bool check) override;
		Bar::Item& Radio(bool check) override;
		Bar::Item& Enable(bool _enable) override;
		Bar::Item& Bold(bool bold) override;
		Bar::Item& Tip(const char *tip) override;
		Bar::Item& Help(const char *help) override;
		Bar::Item& Topic(const char *topic) override;
		Bar::Item& Description(const char *desc) override;
		Bar::Item& AccessValue(const ::Upp::Value& v) override;

		Item(AutomationVisitor& v, AutomationElement& el) : v(v), el(el) {}
	};

	AutomationVisitor& v;
	Array<Item> automation_items;

	Bar::Item& AddItem(Event<> cb) override;
	Bar::Item& AddSubMenu(Event<Bar&> proc) override;
	void AddCtrl(Ctrl *ctrl, int gapsize) override;
	void AddCtrl(Ctrl *ctrl, Size sz) override;
	bool IsMenuBar() const override { return true; }
	bool IsEmpty() const override;
	void Separator() override;

	// Overrides to prevent slicing and ensure our Item is used
	Bar::Item& Add(bool en, const char *text, const ::Upp::Image& img, const Callback& cb) override { return AddItem(cb).Text(text).Image(img).Enable(en); }
	Bar::Item& Add(const char *text, const ::Upp::Image& img, const Callback& cb) override { return AddItem(cb).Text(text).Image(img); }
	Bar::Item& Add(bool en, const char *text, const Callback& cb) override { return AddItem(cb).Text(text).Enable(en); }
	Bar::Item& Add(const char *text, const Callback& cb) override { return AddItem(cb).Text(text); }
	Bar::Item& Add(bool en, const char *text, const ::Upp::Image& img, const Function<void ()>& fn) override { return AddItem(Callback() << fn).Text(text).Image(img).Enable(en); }
	Bar::Item& Add(const char *text, const ::Upp::Image& img, const Function<void ()>& fn) override { return AddItem(Callback() << fn).Text(text).Image(img); }
	Bar::Item& Add(bool en, const char *text, const Function<void ()>& fn) override { return AddItem(Callback() << fn).Text(text).Enable(en); }
	Bar::Item& Add(const char *text, const Function<void ()>& fn) override { return AddItem(Callback() << fn).Text(text); }

	Bar::Item& Sub(bool en, const char *text, const ::Upp::Image& img, const Function<void (Bar&)>& proc) { return AddSubMenu(proc).Text(text).Image(img).Enable(en); }
	Bar::Item& Sub(const char *text, const ::Upp::Image& img, const Function<void (Bar&)>& proc) { return AddSubMenu(proc).Text(text).Image(img); }
	Bar::Item& Sub(bool en, const char *text, const Function<void (Bar&)>& proc) { return AddSubMenu(proc).Text(text).Enable(en); }
	Bar::Item& Sub(const char *text, const Function<void (Bar&)>& proc) { return AddSubMenu(proc).Text(text); }

	Bar::Item& Add(const ::Upp::Image& img, const Callback& cb) { return AddItem(cb).Image(img); }
	Bar::Item& Add(bool en, const ::Upp::Image& img, const Callback& cb) { return AddItem(cb).Image(img).Enable(en); }

	// Add variants with String
	Bar::Item& Add(bool en, const String& text, const ::Upp::Image& img, const Callback& cb) override { return Add(en, ~text, img, cb); }
	Bar::Item& Add(const String& text, const ::Upp::Image& img, const Callback& cb) override { return Add(~text, img, cb); }
	Bar::Item& Add(bool en, const String& text, const Callback& cb) override { return Add(en, ~text, cb); }
	Bar::Item& Add(const String& text, const Callback& cb) override { return Add(~text, cb); }
	
	// Sub variants with String
	Bar::Item& Sub(bool en, const String& text, const ::Upp::Image& img, const Function<void (Bar&)>& proc) { return Sub(en, ~text, img, proc); }
	Bar::Item& Sub(const String& text, const ::Upp::Image& img, const Function<void (Bar&)>& proc) { return Sub(~text, img, proc); }
	Bar::Item& Sub(bool en, const String& text, const Function<void (Bar&)>& proc) { return Sub(en, ~text, proc); }
	Bar::Item& Sub(const String& text, const Function<void (Bar&)>& proc) { return Sub(~text, proc); }

	AutomationBar(AutomationVisitor& v) : v(v) {}
};

class GuiAutomationVisitor : public AutomationVisitor {
public:
	typedef GuiAutomationVisitor CLASSNAME;

	void Read(Ctrl& c);
	::Upp::Value Read(Ctrl& c, const String& path);
	bool Write(Ctrl& c, const String& path, const ::Upp::Value& val, bool do_action = true);
};

#endif
