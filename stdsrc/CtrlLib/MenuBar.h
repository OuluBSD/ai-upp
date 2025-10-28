#pragma once
#ifndef _CtrlLib_MenuBar_h_
#define _CtrlLib_MenuBar_h_

#include "Ctrl.h"
#include <vector>
#include <functional>

namespace Upp {

// MenuItem - represents a single item in a menu
class MenuItem {
public:
	String        text;
	String        help;
	Value         key;
	Image         image;
	Callback      action;
	bool          enabled;
	bool          separator;
	bool          checked;
	int           hotkey;
	
	MenuItem();
	
	MenuItem& Text(const char *s) { text = s; return *this; }
	MenuItem& Help(const char *s) { help = s; return *this; }
	MenuItem& Image(const Upp::Image& img) { image = img; return *this; }
	MenuItem& Key(const Value& v) { key = v; return *this; }
	MenuItem& Action(Callback cb) { action = cb; return *this; }
	MenuItem& Enabled(bool b = true) { enabled = b; return *this; }
	MenuItem& Disabled() { enabled = false; return *this; }
	MenuItem& Separator() { separator = true; return *this; }
	MenuItem& Checked(bool b = true) { checked = b; return *this; }
	MenuItem& HotKey(int hk) { hotkey = hk; return *this; }
	
	bool IsEnabled() const { return enabled; }
	bool IsSeparator() const { return separator; }
};

// MenuBarCtrl - the visual menu bar control
class MenuBarCtrl : public Ctrl {
	std::vector<MenuItem> items;
	int                   hotitem;
	int                   presseditem;
	
public:
	virtual void Paint(Draw& w) override;
	virtual void LeftDown(Point p, dword keyflags) override;
	virtual void LeftUp(Point p, dword keyflags) override;
	virtual void MouseMove(Point p, dword keyflags) override;
	virtual void MouseLeave() override;
	virtual bool Key(dword key, int count) override;
	
	void AddItem(const MenuItem& item);
	void Insert(int index, const MenuItem& item);
	void Remove(int index);
	void Clear();
	
	int GetItemCount() const { return items.size(); }
	const MenuItem& GetItem(int index) const { return items[index]; }
	MenuItem& GetItem(int index) { return items[index]; }
	
	int HitTest(Point p) const;
	
	MenuBarCtrl();
	virtual ~MenuBarCtrl() {}
};

// MenuBar - a container for menu items that can be used in windows
class MenuBar {
	std::vector<MenuItem> items;
	MenuBarCtrl           *ctrl;
	Ctrl                  *parent;
	
public:
	void Add(const MenuItem& item);
	void Insert(int index, const MenuItem& item);
	void Remove(int index);
	void Clear();
	
	void Popup(Ctrl& parent, Point pos);
	void Close();
	
	// Static helper methods for common menu operations
	static bool Scan(Callback& whenbar, dword key);
	static void Execute(Callback& whenbar);
	
	// Helper functions for common menu items
	static MenuItem Separator() { MenuItem item; return item.Separator(); }
	static MenuItem Exit(const char *text = "Exit") { 
		MenuItem item; 
		return item.Text(text).Action([]() { ExitProcess(); }); 
	}
	
	MenuBar();
	~MenuBar();
};

// Menu popup functionality
class PopupMenu {
	std::vector<MenuItem> items;
	
public:
	void Add(const MenuItem& item);
	void AddSeparator() { MenuItem sep; sep.Separator(); Add(sep); }
	void Popup(Ctrl& parent, Point pos = Null);
	
	PopupMenu& Title(const char *title) { /* Implement title if needed */ return *this; }
	
	PopupMenu();
	~PopupMenu();
};

}

#endif