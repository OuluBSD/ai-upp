#pragma once
#ifndef _CtrlLib_ToolBar_h_
#define _CtrlLib_ToolBar_h_

#include "Ctrl.h"
#include <vector>
#include <functional>
#include <memory>

namespace Upp {

// ToolButton - individual button in a toolbar
class ToolButton : public Ctrl, public Bar::Item {
	using Ctrl::Key;

public:
	virtual void   Paint(Draw& w) override;
	virtual void   MouseEnter(Point, dword) override;
	virtual void   MouseLeave() override;
	virtual Size   GetMinSize() const override;
	virtual void   LeftDown(Point, dword) override;
	virtual void   LeftRepeat(Point, dword) override;
	virtual void   LeftUp(Point, dword) override;
	virtual bool   HotKey(dword key) override;
	virtual String GetDesc() const override;
	virtual int    OverPaint() const override;

	virtual Bar::Item& Text(const char *text) override;
	virtual Bar::Item& Key(dword key) override;
	virtual Bar::Item& Repeat(bool repeat = true) override;
	virtual Bar::Item& Image(const Image& img) override;
	virtual Bar::Item& Enable(bool _enable = true) override;
	virtual Bar::Item& Tip(const char *tip) override;
	virtual Bar::Item& Help(const char *help) override;
	virtual Bar::Item& Topic(const char *help) override;
	virtual Bar::Item& Description(const char *desc) override;
	virtual Bar::Item& Radio(bool check) override;
	virtual Bar::Item& Check(bool check) override;
	virtual void       FinalSync() override;

public:
	struct Style : ChStyle<Style> {
		Value  look[6]; // Different states: normal, hover, pressed, disabled, checked, checked-hover
		Font   font;
		Color  textcolor[6]; // Text colors for different states
		bool   light[6]; // Light effect for different states
		int    contrast[6]; // Contrast effect for different states
		Point  offset[6]; // Offset for different states
		int    overpaint;
		
		static const Style& StyleDefault();
		static const Style& StyleSolid();
	};

protected:
	String  text;
	String  tiptext;
	dword   accel;
	bool    checked;
	bool    paint_checked;
	bool    repeat;

	byte    kind;
	Size    minsize;
	Size    maxiconsize;
	bool    nodarkadjust;

	const Style      *style;

private:
	Image img;

	void       SendHelpLine();
	void       ClearHelpLine();
	void       UpdateTip();

public:
	enum Kind { 
		NOLABEL = 0,     // No text label
		RIGHTLABEL = 1,  // Text to the right of icon
		BOTTOMLABEL = 2  // Text below the icon
	};

	void  ResetKeepStyle();
	void  Reset();

	static const Style& StyleDefault();
	static const Style& StyleSolid();

	bool		IsChecked() const { return checked; }
	Image       GetImage() const { return img; }

	ToolButton& SetStyle(const Style& s);
	ToolButton& MinSize(Size sz)         { minsize = sz; return *this; }
	ToolButton& MaxIconSize(Size sz)     { maxiconsize = sz; return *this; }
	ToolButton& Kind(int _kind);
	ToolButton& Label(const char *text, int kind);
	ToolButton& Label(const char *text);
	ToolButton& NoDarkAdjust(bool b = true) { nodarkadjust = b; return *this; }
	
	// Accessors
	String      GetText() const { return text; }
	String      GetTip() const { return tiptext; }
	dword       GetKey() const { return accel; }
	bool        IsEnabled() const { return IsEnabled(); }
	bool        IsRepeat() const { return repeat; }
	int         GetKind() const { return kind; }
	Size        GetMinSizeSetting() const { return minsize; }
	Size        GetMaxIconSizeSetting() const { return maxiconsize; }

	ToolButton();
	virtual ~ToolButton();
};

// ToolBar - container for tool buttons
class ToolBar : public BarCtrl {
public:
	virtual bool HotKey(dword key) override;
	virtual void Paint(Draw& w) override;

protected:
	virtual Item& AddItem(Event<>  cb) override;
	virtual Item& AddSubMenu(Event<Bar&> proc) override;

public:
	struct Style : ChStyle<Style> {
		ToolButton::Style    buttonstyle;
		Size                 buttonminsize;
		Size                 maxiconsize;
		int                  buttonkind;
		Value                look, arealook;
		SeparatorCtrl::Style breaksep;
		SeparatorCtrl::Style separator;
		
		static const Style& StyleDefault();
	};

private:
	int               ii;
	Array<ToolButton> item;
	int               lock;
	Event<Bar&>       proc;
	const Style      *style;
	int               arealook;

	Size              buttonminsize;
	Size              maxiconsize;
	int               kind;
	bool              nodarkadjust;

protected:
	enum {
		TIMEID_POST = BarCtrl::TIMEID_COUNT,
		TIMEID_COUNT
	};

public:
	virtual bool IsToolBar() const override { return true; }

	static int GetStdHeight();

	void Clear();
	void Set(Event<Bar&> bar);
	void Post(Event<Bar&> bar);

	static const Style& StyleDefault();

	ToolBar& SetStyle(const Style& s)               { style = &s; Refresh(); return *this; }

	ToolBar& ButtonMinSize(Size sz)                 { buttonminsize = sz; return *this; }
	ToolBar& MaxIconSize(Size sz)                   { maxiconsize = sz; return *this; }
	ToolBar& ButtonKind(int _kind)                  { kind = _kind; return *this; }
	ToolBar& AreaLook(int q = 1)                    { arealook = q; Refresh(); return *this; }
	ToolBar& NoDarkAdjust(bool b = true)            { nodarkadjust = b; return *this; }
	
	// Accessors
	int GetItemCount() const { return item.GetCount(); }
	ToolButton& GetItem(int i) { return item[i]; }
	const ToolButton& GetItem(int i) const { return item[i]; }

	typedef ToolBar  CLASSNAME;

	ToolBar();
	virtual ~ToolBar();
};

// StaticBarArea - static bar area for toolbars
class StaticBarArea : public Ctrl {
public:
	virtual void Paint(Draw& w) override;

private:
	bool upperframe;

public:
	StaticBarArea& UpperFrame(bool b) { upperframe = b; Refresh(); return *this; }
	StaticBarArea& NoUpperFrame()     { return UpperFrame(false); }

	StaticBarArea();
};

// LRUList - Recently Used List for menus/toolbars
class LRUList {
	Vector<String> lru;
	int            limit;
	void           Select(String s, Event<const String&> WhenSelect);

public:
	static int GetStdHeight();

	void        Serialize(Stream& stream);

	void        operator()(Bar& bar, Event<const String&> WhenSelect, int count = INT_MAX, int from = 0);

	void        NewEntry(const String& path);
	void        RemoveEntry(const String& path);

	int         GetCount() const                        { return lru.GetCount(); }

	LRUList&    Limit(int _limit)                       { limit = _limit; return *this; }
	int         GetLimit() const                        { return limit; }

	typedef LRUList CLASSNAME;

	LRUList()   { limit = 6; }
};

// ToolTip - tooltip popup for controls
class ToolTip : public Ctrl {
public:
	virtual void Paint(Draw& w) override;
	virtual Size GetMinSize() const override;

private:
	String  text;

public:
	void   Set(const char *_text)        { text = _text; }
	String Get() const                   { return text; }

	void PopUp(Ctrl *owner, Point p, bool effect);

	ToolTip();
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

void PerformDescription();

}

#endif