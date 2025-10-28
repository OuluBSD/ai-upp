#pragma once
#ifndef _CtrlLib_MenuBar_h_
#define _CtrlLib_MenuBar_h_

#include "Ctrl.h"
#include <vector>
#include <functional>
#include <memory>

namespace Upp {

// Forward declarations
class MenuItemBase;
class SubMenuBase;
class MenuBar;

// MenuItemBase - base class for menu items
class MenuItemBase : public Ctrl, public Bar::Item {
public:
	virtual Bar::Item& Text(const char *text) override;
	virtual Bar::Item& Key(dword key) override;
	virtual Bar::Item& Image(const Image& img) override;
	virtual Bar::Item& Enable(bool _enable) override;
	virtual Bar::Item& Tip(const char *tip) override;
	virtual Bar::Item& Help(const char *help) override;
	virtual Bar::Item& Topic(const char *help) override;
	virtual Bar::Item& Description(const char *desc) override;
	virtual Bar::Item& Check(bool check) override;
	virtual Bar::Item& Radio(bool check) override;
	virtual Bar::Item& Bold(bool bold) override;

	virtual String GetDesc() const override;
	virtual dword  GetAccessKeys() const;
	virtual void   AssignAccessKeys(dword used);

	using   Ctrl::Key;

protected:
	enum {
		NOTHING, CHECK0, CHECK1, RADIO0, RADIO1
	};

	enum {
		NORMAL, HIGHLIGHT, PUSH
	};

	String  text;
	dword   accel;
	int     state;
	int     leftgap, textgap;
	Font    font;
	bool    isenabled;
	byte    type;
	int     accesskey;
	Size    maxiconsize;
	
	// Style information
	struct Style {
		Color item; // hot menu item background in popup menu
		Color topitem[3]; // top menu item background normal/hot/pressed
		Color topbar; // deprecated
		Color menutext; // normal state popup menu item text
		Color itemtext; // hot state popup menu item text
		Color topitemtext[3]; // top menu item text normal/hot/pressed
		Color breaksep; // separator between menu bars
		Color look; // top menu background
		Color arealook; // top menu backgroung if arealook and in frame (can be null, then 'look')
		Color popupframe; // static frame of whole popup menu
		Color popupbody; // background of whole popup menu
		Color popupiconbar; // if there is special icon background in popup menu
		Color separator;
		Size  maxiconsize; // limit of icon size
		int   leftgap; // between left border and icon
		int   textgap;
		int   lsepm;
		int   rsepm;
		Point pullshift; // offset of submenu popup
		bool  opaquetest; // If true, topmenu item can change hot text color
		
		static Style Default();
	};
	
	const Style *style;
	bool    nodarkadjust;

	MenuBar *GetMenuBar() const;

public:
	virtual void SyncState() = 0;

	void           DrawMenuText(Draw& w, int x, int y, const String& s, Font f, bool enabled, bool hl,
	                            Color color, Color hlcolor);
	void           PaintTopItem(Draw& w, int state);

	bool           IsItemEnabled() const          { return isenabled; }
	String         GetText() const                { return text; }
	MenuItemBase&  LeftGap(int cx)                { leftgap = cx; return *this; }
	MenuItemBase&  TextGap(int cx)                { textgap = cx; return *this; }
	MenuItemBase&  SetFont(Font f)                { font = f; return *this; }
	MenuItemBase&  Style(const Style *s)          { style = s; return *this; }
	Font           GetFont() const                { return font; }
	MenuItemBase&  MaxIconSize(Size sz)          { maxiconsize = sz; return *this; }
	bool           InOpaqueBar() const;
	MenuItemBase&  NoDarkAdjust(bool b = true)  { nodarkadjust = b; return *this; }

	MenuItemBase();
};

// MenuItem - regular menu item
class MenuItem : public MenuItemBase {
public:
	virtual void  Paint(Draw& w) override;
	virtual void  MouseEnter(Point, dword) override;
	virtual void  MouseLeave() override;
	virtual Size  GetMinSize() const override;
	virtual void  LeftUp(Point, dword) override;
	virtual void  RightUp(Point, dword) override;
	virtual void  GotFocus() override;
	virtual void  LostFocus() override;
	virtual bool  Key(dword key, int count) override;
	virtual bool  HotKey(dword key) override;
	virtual void  SyncState() override;

	virtual Bar::Item& Image(const Image& img) override;

private:
	Image licon, ricon;

	void  SendHelpLine();
	void  ClearHelpLine();
	using MenuItemBase::Key;

protected:
	virtual int  GetVisualState();

public:
	MenuItem& RightImage(const Image& img);
};

// SubMenuBase - base class for submenu items
class SubMenuBase {
protected:
	MenuBar  menu;
	Event<Bar&> proc;
	MenuBar *parentmenu;

	void     Pull(Ctrl *item, Point p, Size sz);

public:
	virtual  void Pull() = 0;

	void SetParent(MenuBar *m);
	void Set(Event<Bar&> _submenu);
	Event<Bar&> Get();

	SubMenuBase();
	virtual ~SubMenuBase();
};

// SubMenuItem - submenu item in popup menus
class SubMenuItem : public MenuItem, public SubMenuBase {
public:
	virtual void MouseEnter(Point, dword) override;
	virtual void MouseLeave() override;
	virtual void GotFocus() override;
	virtual bool HotKey(dword key) override;
	virtual bool Key(dword key, int count) override;
	virtual int  GetVisualState() override;
	virtual void Pull() override;

protected:
	enum {
		TIMEID_PULL = BarCtrl::TIMEID_COUNT,
		TIMEID_COUNT
	};

public:
	typedef SubMenuItem CLASSNAME;

	SubMenuItem();
};

// TopSubMenuItem - submenu item in top-level menu bar
class TopSubMenuItem : public MenuItemBase, public SubMenuBase {
public:
	virtual void Paint(Draw& w) override;
	virtual void MouseEnter(Point, dword) override;
	virtual void MouseLeave() override;
	virtual void GotFocus() override;
	virtual void LostFocus() override;
	virtual void LeftDown(Point, dword) override;
	virtual void SyncState() override;
	virtual Size GetMinSize() const override;
	virtual bool Key(dword key, int) override;
	virtual bool HotKey(dword key) override;
	virtual void Pull() override;

private:
	int  GetState();
	using MenuItemBase::Key;

public:
	TopSubMenuItem();
};

// TopMenuItem - regular item in top-level menu bar
class TopMenuItem : public MenuItemBase {
public:
	virtual void  Paint(Draw& w) override;
	virtual void  MouseEnter(Point, dword) override;
	virtual void  MouseLeave() override;
	virtual void  LeftUp(Point, dword) override;
	virtual void  LeftDown(Point, dword) override;
	virtual void  GotFocus() override;
	virtual void  LostFocus() override;
	virtual bool  Key(dword key, int count) override;
	virtual Size  GetMinSize() const override;
	virtual void  SyncState() override;

	static int GetStdHeight(Font font = StdFont());

private:
	int  GetState();
	using MenuItemBase::Key;

public:
	TopMenuItem();
};

// MenuBar - main menu bar class
class MenuBar : public BarCtrl {
public:
	virtual void  LeftDown(Point, dword) override;
	virtual bool  Key(dword key, int count) override;
	virtual bool  HotKey(dword key) override;
	virtual void  ChildGotFocus() override;
	virtual void  ChildLostFocus() override;
	virtual void  Deactivate() override;
	virtual void  CancelMode() override;
	virtual void  Paint(Draw& w) override;
	virtual bool  IsMenuBar() const override { return true; }
	virtual bool  IsEmpty() const override;
	virtual void  Separator() override;

protected:
	virtual Item& AddItem(Event<> cb) override;
	virtual Item& AddSubMenu(Event<Bar&> proc) override;
	virtual Value GetBackground() const override;

public:
	struct Style : ChStyle<Style> {
		Value item; // hot menu item background in popup menu
		Value topitem[3]; // top menu item background normal/hot/pressed
		Value topbar; // deprecated
		Color menutext; // normal state popup menu item text
		Color itemtext; // hot state popup menu item text
		Color topitemtext[3]; // top menu item text normal/hot/pressed
		SeparatorCtrl::Style breaksep; // separator between menu bars
		Value look; // top menu background
		Value arealook; // top menu backgroung if arealook and in frame (can be null, then 'look')
		Value popupframe; // static frame of whole popup menu
		Value popupbody; // background of whole popup menu
		Value popupiconbar; // if there is special icon background in popup menu
		SeparatorCtrl::Style separator;
		Size  maxiconsize; // limit of icon size
		int   leftgap; // between left border and icon
		int   textgap;
		int   lsepm;
		int   rsepm;
		Point pullshift; // offset of submenu popup
		bool  opaquetest; // If true, topmenu item can change hot text color
		Value icheck; // background of Check (or Radio) item with image

		static const Style& StyleDefault();
	};

private:
	Array<MenuItemBase> item;

	MenuBar     *parentmenu;
	MenuBar     *submenu;
	Ctrl        *submenuitem;
	std::shared_ptr<Ctrl>    restorefocus;
	bool         doeffect;
	Font         font;
	int          leftgap;
	int          lock;
	const Style *style;
	int          arealook;
	Size         maxiconsize;
	LookFrame    frame;
	bool         nodarkadjust;
	bool         action_taken = false; // local menu resulted in action invoked (not cancel)

	void     SetParentMenu(MenuBar *parent)    { parentmenu = parent; style = parent->style; }
	MenuBar *GetParentMenu()                   { return parentmenu; }
	void     SetActiveSubmenu(MenuBar *sm, Ctrl *menuitem);
	MenuBar *GetActiveSubmenu()                { return submenu; }
	MenuBar *GetMasterMenu();
	MenuBar *GetLastSubmenu();
	void     DelayedClose();
	void     KillDelayedClose();
	void     SubmenuClose();
	void     PostDeactivate();
	void     SyncState();
	void     SetupRestoreFocus();
	void     PopUp(Ctrl *owner, Point p, Size rsz = Size(0, 0));

protected:
	enum {
		TIMEID_STOP = BarCtrl::TIMEID_COUNT,
		TIMEID_SUBMENUCLOSE,
		TIMEID_POST,
		TIMEID_COUNT
	};

public:
	Event<>  WhenSubMenuOpen;
	Event<>  WhenSubMenuClose;

	static int GetStdHeight(Font font = StdFont());

	void     CloseMenu();

	void     Set(Event<Bar&> menu);
	void     Post(Event<Bar&> bar);

	void     PopUp(Point p)                         { PopUp(GetActiveCtrl(), p); }
	void     PopUp()                                { PopUp(GetMousePos()); }

	bool     Execute(Ctrl *owner, Point p);
	bool     Execute(Point p)                       { return Execute(GetActiveCtrl(), p); }
	bool     Execute()                              { return Execute(GetMousePos()); }

	static bool Execute(Ctrl *owner, Event<Bar&> proc, Point p);
	static bool Execute(Event<Bar&> proc, Point p)  { return Execute(GetActiveCtrl(), proc, p); }
	static bool Execute(Event<Bar&> proc)          { return Execute(proc, GetMousePos()); }

	void     Clear();

	static const Style& StyleDefault();

	MenuBar& LeftGap(int cx)                        { leftgap = cx; return *this; }
	MenuBar& SetFont(Font f)                        { font = f; return *this; }
	MenuBar& SetStyle(const Style& s);
	Font     GetFont() const                        { return font; }
	MenuBar& AreaLook(int q = 1)                    { arealook = q; Refresh(); return *this; }
	MenuBar& MaxIconSize(Size sz)                   { maxiconsize = sz; return *this; }
	MenuBar& MaxIconSize(int n)                     { return MaxIconSize(Size(n, n)); }
	Size     GetMaxIconSize() const                 { return maxiconsize; }
	MenuBar& NoDarkAdjust(bool b = true)            { nodarkadjust = b; return *this; }

	typedef MenuBar CLASSNAME;

	MenuBar();
	virtual ~MenuBar();
};

}

#endif