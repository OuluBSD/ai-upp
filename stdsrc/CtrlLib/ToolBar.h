#pragma once
#ifndef _CtrlLib_ToolBar_h_
#define _CtrlLib_ToolBar_h_

#include "Ctrl.h"
#include <vector>

namespace Upp {

// ToolButton - individual button in a toolbar
class ToolButton : public Button {
	Image    image;
	String   text;
	String   tooltip;
	bool     enabled;
	bool     pressed;
	int      style;  // e.g., normal, toggle, separator
	
public:
	virtual void Paint(Draw& w) override;
	
	ToolButton& SetImage(const Image& img) { image = img; return *this; }
	ToolButton& SetText(const String& s) { text = s; Refresh(); return *this; }
	ToolButton& SetToolTip(const String& s) { tooltip = s; return *this; }
	ToolButton& SetStyle(int s) { style = s; return *this; }
	ToolButton& Enable(bool b = true) { enabled = b; SetEnabled(b); return *this; }
	ToolButton& Disable() { return Enable(false); }
	ToolButton& SetPressed(bool b = true) { pressed = b; return *this; }
	
	const Image& GetImage() const { return image; }
	const String& GetText() const { return text; }
	bool IsEnabled() const { return enabled; }
	bool IsPressed() const { return pressed; }
	
	ToolButton();
};

// ToolBar - container for tool buttons
class ToolBar : public Ctrl {
	std::vector<ToolButton*> buttons;
	int                      orientation;  // horizontal or vertical
	int                      buttonsize;
	bool                     showtext;
	bool                     showimage;
	
public:
	virtual void Paint(Draw& w) override;
	virtual void Layout() override;
	
	// Add different types of controls to the toolbar
	ToolButton& Add(const Image& img, const char *text = NULL);
	ToolButton& Add(const char *text = NULL);
	ToolButton& AddButton(const Image& img, const char *text = NULL);
	ToolButton& AddSeparator();
	ToolButton& AddCtrl(Ctrl& ctrl);  // Add any control to the toolbar
	
	// Set toolbar properties
	ToolBar& Horz() { orientation = 0; Refresh(); return *this; }
	ToolBar& Vert() { orientation = 1; Refresh(); return *this; }
	ToolBar& ButtonSize(int sz) { buttonsize = sz; Layout(); return *this; }
	ToolBar& ShowText(bool b = true) { showtext = b; Layout(); return *this; }
	ToolBar& ShowImage(bool b = true) { showimage = b; Layout(); return *this; }
	ToolBar& NoText() { showtext = false; Layout(); return *this; }
	ToolBar& NoImage() { showimage = false; Layout(); return *this; }
	
	// Toolbar operations
	void Clear();
	void SetGray(bool gray = true);  // Gray out disabled buttons
	
	// Access to buttons
	int GetButtonCount() const { return buttons.size(); }
	ToolButton& GetButton(int i) { return *buttons[i]; }
	const ToolButton& GetButton(int i) const { return *buttons[i]; }
	
	ToolBar();
	virtual ~ToolBar();
};

// Command - represents an action that can be triggered by menu/toolbar
class Command {
	Callback action;
	String   name;
	Image    icon;
	String   help;
	dword    hotkey;
	bool     enabled;
	
public:
	Command& operator=(const Callback& cb) { action = cb; return *this; }
	Command& Name(const char *s) { name = s; return *this; }
	Command& Icon(const Image& m) { icon = m; return *this; }
	Command& Help(const char *s) { help = s; return *this; }
	Command& HotKey(dword hk) { hotkey = hk; return *this; }
	Command& Enable(bool b = true) { enabled = b; return *this; }
	Command& Disable() { enabled = false; return *this; }
	
	void Execute() const { if(enabled && action) action(); }
	
	bool IsEnabled() const { return enabled; }
	const String& GetName() const { return name; }
	const Image& GetIcon() const { return icon; }
	const String& GetHelp() const { return help; }
	dword GetHotKey() const { return hotkey; }
	
	operator Callback() const { return action; }
	
	Command() : hotkey(0), enabled(true) {}
};

// CommandBar - unified interface for menu and toolbar commands
class CommandBar {
	std::vector<Command> commands;
	
public:
	Command& Add(const char *name, const Callback& action);
	Command& Add(const Image& icon, const char *name, const Callback& action);
	Command& AddSeparator();
	
	void Attach(MenuBar& menu);
	void Attach(ToolBar& toolbar);
	
	CommandBar();
};

}

#endif