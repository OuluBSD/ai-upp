#include "CtrlLib.h"

#ifndef CtrlCore_MenuImp_h
#define CtrlCore_MenuImp_h

namespace Upp {

class MenuItemBase : public Ctrl, public Bar::Item {
public:
	Bar::Item& Text(const char *text) override;
	Bar::Item& Key(dword key) override;
	Bar::Item& Image(const UPP::Image& img) override;
	Bar::Item& Enable(bool _enable) override;
	Bar::Item& Tip(const char *tip) override;
	Bar::Item& Help(const char *help) override;
	Bar::Item& Topic(const char *help) override;
	Bar::Item& Description(const char *desc) override;
	Bar::Item& Check(bool check) override;
	Bar::Item& Radio(bool check) override;
	Bar::Item& Bold(bool bold) override;
	Bar::Item& AccessValue(const ::Upp::Value& v) override;

	String GetDesc() const override;
	dword  GetAccessKeys() const override;
	void   AssignAccessKeys(dword used) override;

	bool   Access(Visitor& v) override;

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
	const MenuBar::Style *style;
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
	MenuItemBase&  Style(const MenuBar::Style *s) { style = s; return *this; }
	Font           GetFont() const                { return font; }
	MenuItemBase&  MaxIconSize(Size sz)           { maxiconsize = sz; return *this; }
	bool           InOpaqueBar() const;
	MenuItemBase&  NoDarkAdjust(bool b = true)    { nodarkadjust = b; return *this; }

	MenuItemBase();
};

class MenuItem : public MenuItemBase {
public:
	void  Paint(Draw& w) override;
	void  MouseEnter(Point, dword) override;
	void  MouseLeave() override;
	Size  GetMinSize() const override;
	void  LeftUp(Point, dword) override;
	void  RightUp(Point, dword) override;
	void  GotFocus() override;
	void  LostFocus() override;
	bool  Key(dword key, int count) override;
	bool  HotKey(dword key) override;
	void  SyncState() override;

	Bar::Item& Image(const UPP::Image& img) override;

private:
	UPP::Image licon, ricon;

	void  SendHelpLine();
	void  ClearHelpLine();
	using MenuItemBase::Key;

protected:
	virtual int  GetVisualState();

public:
	MenuItem& RightImage(const UPP::Image& img);
};

class SubMenuBase {
protected:
	MenuBar  menu;
	Event<Bar&> proc;
	MenuBar *parentmenu;

	void     Pull(Ctrl *item, Point p, Size sz);

public:
	virtual  void Pull() = 0;

	void SetParent(MenuBar *m)                           { parentmenu = m; menu.MaxIconSize(m->GetMaxIconSize()); }
	void Set(Event<Bar&> _submenu)                   { proc = _submenu; }
	Event<Bar&> Get()                                { return proc; }

	SubMenuBase()                                        { parentmenu = NULL; }
	virtual ~SubMenuBase() {}
};

class SubMenuItem : public MenuItem, public SubMenuBase {
public:
	void MouseEnter(Point, dword) override;
	void MouseLeave() override;
	void GotFocus() override;
	bool HotKey(dword key) override;
	bool Key(dword key, int count) override;
	int  GetVisualState() override;
	void Pull() override;
	bool Access(Visitor& v) override;

protected:
	enum {
		TIMEID_PULL = BarCtrl::TIMEID_COUNT,
		TIMEID_COUNT
	};

public:
	typedef SubMenuItem CLASSNAME;

	SubMenuItem();
};

class TopSubMenuItem : public MenuItemBase, public SubMenuBase {
public:
	void Paint(Draw& w) override;
	void MouseEnter(Point, dword) override;
	void MouseLeave() override;
	void GotFocus() override;
	void LostFocus() override;
	void LeftDown(Point, dword) override;
	void SyncState() override;
	Size GetMinSize() const override;
	bool Key(dword key, int) override;
	bool HotKey(dword key) override;
	void Pull() override;
	bool Access(Visitor& v) override;

private:
	int  GetState();
	using MenuItemBase::Key;

public:
	TopSubMenuItem() { NoInitFocus(); }
};

class TopMenuItem : public MenuItemBase {
public:
	void  Paint(Draw& w) override;
	void  MouseEnter(Point, dword) override;
	void  MouseLeave() override;
	void  LeftUp(Point, dword) override;
	void  LeftDown(Point, dword) override;
	void  GotFocus() override;
	void  LostFocus() override;
	bool  Key(dword key, int count) override;
	Size  GetMinSize() const override;
	void  SyncState() override;

	static int GetStdHeight(Font font = StdFont());

private:
	int  GetState();
	using MenuItemBase::Key;

public:
	TopMenuItem() { NoInitFocus(); }
};

}

#endif
