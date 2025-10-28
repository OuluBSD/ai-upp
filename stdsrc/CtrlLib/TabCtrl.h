#pragma once
#ifndef _CtrlLib_TabCtrl_h_
#define _CtrlLib_TabCtrl_h_

#include "Ctrl.h"
#include <vector>

namespace Upp {

class TabCtrl : public Ctrl {
public:
	struct Style : public ChStyle<Style> {
		Font        font;
		int         tabheight;
		int         margin;
		Rect        sel;
		Rect        edge;
		int         extendleft;
		Value       normal[4];
		Value       first[4];
		Value       last[4];
		Value       both[4];
		Value       body;
		Color       text_color[4];
		
		static Style Default();
	};

private:
	struct Item {
		TabCtrl      *owner;
		String        text;
		PaintRect     pict;
		Ctrl         *ctrl;
		Ctrl         *slave;
		bool          enabled;
		Value         key;
		int           x, cx;
		Point         pictpos, textpos;

		Item& Text(const String& text);
		Item& Picture(const Image& img);
		Item& SetCtrl(Ctrl *ctrl);
		Item& Slave(Ctrl *slave);
		Item& Enable(bool en = true);
		int   Right() const { return x + cx; }
		bool  IsEnabled() const { return enabled; }
		
		void  Layout(int xp, int y, int cy);
		void  Paint(Draw& w, int state);
		
		Item();
	};

	std::vector<Item> tab;
	int               sel;
	int               hot;
	int               x0;
	bool              no_accept;
	bool              accept_current;
	One<Style>        style;
	
	Ctrl              tabs;
	Ctrl              pane;
	Button            left;
	Button            right;

	void  SyncTabs();
	void  PaintTabs(Draw& w);
	void  SyncHot();
	int   GetTab(Point p) const;
	void  ScrollInto(int i);
	void  PaintItem(Draw& w, int i, int state);
	int   TabsRight();

public:
	virtual void  Paint(Draw& w) override;
	virtual void  LeftDown(Point p, dword keyflags) override;
	virtual void  MouseMove(Point p, dword keyflags) override;
	virtual void  MouseLeave() override;
	virtual void  CancelMode() override;
	virtual bool  Key(dword key, int count) override;
	virtual bool  HotKey(dword key) override;
	virtual bool  Accept() override;
	virtual void  Layout() override;
	virtual Rect  GetOpaqueRect() const override;
	
	void  SetStyle(const Style& s) { style = s; }
	const Style& GetStyle() const { return *style; }
	
	TabCtrl& NoAccept(bool b = true) { no_accept = b; return *this; }
	TabCtrl& AcceptCurrent(bool b = true) { accept_current = b; return *this; }
	
	// Tab management
	Item& Add();
	Item& Add(const char *text);
	Item& Add(const Image& m, const char *text);
	Item& Add(Ctrl& slave, const char *text);
	Item& Add(Ctrl& slave, const Image& m, const char *text);
	
	Item& Insert(int i);
	Item& Insert(int i, const char *text);
	Item& Insert(int i, const Image& m, const char *text);
	Item& Insert(int i, Ctrl& slave, const char *text);
	Item& Insert(int i, Ctrl& slave, const Image& m, const char *text);
	Item& Insert(Ctrl& before_slave);
	Item& Insert(Ctrl& before_slave, const char *text);
	Item& Insert(Ctrl& before_slave, const Image& m, const char *text);
	Item& Insert(Ctrl& before_slave, Ctrl& slave, const char *text);
	Item& Insert(Ctrl& before_slave, Ctrl& slave, const Image& m, const char *text);
	
	void  Remove(int i);
	void  Remove(Ctrl& slave);
	void  Reset();
	
	int   GetCount() const { return tab.size(); }
	int   Get() const { return sel; }
	void  Set(int i);
	void  Set(Ctrl& slave);
	void  Set(const Value& data);
	Value GetData() const;
	
	int   Find(const Ctrl& slave) const;
	int   FindInsert(Ctrl& slave);
	
	// Navigation
	void  Go(int d);
	void  GoNext() { Go(1); }
	void  GoPrev() { Go(-1); }
	
	// Tab properties
	const String& GetTabText(int i) const { return tab[i].text; }
	void          SetTabText(int i, const String& text) { tab[i].Text(text); }
	bool          IsTabEnabled(int i) const { return tab[i].IsEnabled(); }
	void          EnableTab(int i, bool enable = true) { tab[i].Enable(enable); }
	
	// Accessors
	Ctrl& GetTabCtrl(int i) { return *tab[i].ctrl; }
	Ctrl& GetTabSlave(int i) { return *tab[i].slave; }
	
	// Scrolling
	void  Left();
	void  Right();
	
	// Layout
	Size  ComputeSize();
	Size  ComputeSize(Size pane);
	
	TabCtrl();
	virtual ~TabCtrl() {}
};

// TabDlg - dialog with tabs
class TabDlg : public TopWindow {
	TabCtrl    tabctrl;
	Size       sz;
	Button     ok;
	Button     cancel;
	Button     apply;
	Button     exit;
	bool       binit;

	void PlaceButton(Button& b, int& r);
	void Rearrange();
	TabCtrl::Item& Add0(Ctrl& tab, const char *text);

public:
	TabCtrl& GetTabCtrl() { return tabctrl; }
	const TabCtrl& GetTabCtrl() const { return tabctrl; }
	
	TabDlg& AButton(Button& b);
	
	Button& OK() { return ok; }
	Button& Cancel() { return cancel; }
	Button& Apply() { return apply; }
	Button& Exit() { return exit; }
	
	TabDlg();
};

}

#endif