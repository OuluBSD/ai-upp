#ifndef _CtrlLib_RibbonBar_h_
#define _CtrlLib_RibbonBar_h_

struct RibbonStyle : ChStyle<RibbonStyle> {
	Size full_button;
	Size full_icon_area;
	Size full_icon;
	Size list_icon;
	int  list_row_cy;
	int  label_gap;
	int  group_gap;
	Font full_font;
	Font list_font;
	Value group_look;
	Value list_look;
	Value list_hot;
	Value list_push;
};

class RibbonList : public ParentCtrl {
public:
	struct Row : Moveable<Row> {
		Image  icon;
		String text;
		Event<> WhenAction;
		Ptr<Ctrl> extra;
		bool   enabled;
	};

	RibbonList();

	RibbonList& Clear();
	RibbonList& Add(const Image& icon, const String& text, Event<> cb);
	RibbonList& AddCtrl(Ctrl& ctrl);
	RibbonList& Set(Event<Bar&> bar);
	RibbonList& ShowText(bool show = true);
	RibbonList& EnableRow(int i, bool enable = true);

	Size GetMinSize() const override;
	void Layout() override;
	void Paint(Draw& w) override;
	void MouseMove(Point p, dword keyflags) override;
	void MouseLeave() override;
	void LeftDown(Point p, dword keyflags) override;

	RibbonList& SetStyle(const RibbonStyle& s);

private:
	Vector<Row> row;
	Array<Button> btn;
	int         hot;
	int         push;
	bool        show_text;
	const RibbonStyle *style;

	int  RowFromPoint(Point p) const;
	Rect RowRect(int i) const;
};

class RibbonGroup : public ParentCtrl {
public:
	RibbonGroup& SetLabel(const String& text);
	RibbonGroup& SetLarge(Event<Bar&> bar);
	RibbonGroup& SetList(Event<Bar&> bar);
	RibbonGroup& SetListCtrl(Ctrl& ctrl);
	RibbonGroup& ShowListText(bool show = true);
	RibbonGroup& EnableList(bool enable = true);
	RibbonGroup& SetStyle(const RibbonStyle& s);
	RibbonGroup& SetContentMinSize(Size sz);

	Array<Button>& LargeButtons()                   { return large_btn; }
	RibbonList&  List()                             { return list; }

	Size GetMinSize() const override;
	void Layout() override;
	void Paint(Draw& w) override;

	RibbonGroup();

private:
	Array<Button> large_btn;
	RibbonList list;
	Ptr<Ctrl> list_ctrl;
	Label   lbl;
	int     label_gap;
	bool    show_list_text;
	const RibbonStyle *style;
	bool    use_large;
	Size    content_min;
};

class RibbonPage : public ParentCtrl {
public:
	RibbonGroup& AddGroup(const String& label);
	RibbonGroup& AddGroup(const String& label, Event<Bar&> bar);
	void         ClearGroups();

	Size GetMinSize() const override;
	void Layout() override;

	RibbonPage();

private:
	Array<RibbonGroup> group;
	Array<SeparatorCtrl> sep;
	int                group_gap;
};

class RibbonBar : public FrameTop<ParentCtrl> {
public:
	enum DisplayMode {
		RIBBON_ALWAYS,
		RIBBON_TABS,
		RIBBON_AUTOHIDE,
	};

	enum QuickAccessPos {
		QAT_TOP,
		QAT_BOTTOM,
	};

	using Style = RibbonStyle;

	RibbonBar();

	RibbonPage& AddTab(const String& text);
	RibbonPage& AddContextTab(const String& context, const String& text);

	void        RemoveTab(int tab);
	void        ClearTabs();
	int         GetTabCount() const                  { return tab.GetCount(); }

	RibbonBar&  RenameTab(int tab, const String& text);
	RibbonBar&  ShowContext(const String& context, bool show = true);

	RibbonBar&  SetDisplayMode(DisplayMode mode);
	DisplayMode GetDisplayMode() const               { return (DisplayMode)display_mode; }

	RibbonBar&  SetQuickAccess(Event<Bar&> bar);
	ToolBar&    QuickAccess()                        { return qat; }
	RibbonBar&  SetQuickAccessPos(QuickAccessPos pos);

	RibbonBar&  Expand();
	RibbonBar&  Collapse();
	RibbonBar&  ToggleExpanded();
	bool        IsExpanded() const                   { return expanded; }

	Event<int>  WhenTab;
	Event<Bar&> WhenTabMenu;

	bool        Key(dword key, int count) override;
	void        Layout() override;
	void        Paint(Draw& w) override;
	void        MouseLeave() override;

private:
	struct TabInfo : Moveable<TabInfo> {
		String text;
		String context;
		bool   contextual;
		bool   visible;
		int    page_index;
	};

	class RibbonTabCtrl : public TabCtrl {
	public:
		Event<> WhenDouble;
		Event<Point> WhenMenu;
		Event<int> WhenSame;
		void    LeftDouble(Point p, dword keyflags) override;
		void    LeftDown(Point p, dword keyflags) override;
		void    RightDown(Point p, dword keyflags) override;
	};

	RibbonTabCtrl      tabs;
	ParentCtrl         page_area;
	ToolBar            qat;
	Array<RibbonPage>  page;

	Vector<TabInfo>    tab;
	Vector<int>        visible_map;

	int                display_mode;
	int                qat_pos;
	bool               expanded;
	bool               show_keytips;
	const Style       *style;

	void RebuildTabs();
	void SyncSelection();
	void UpdateVisibility();
	void OnTab();
	int  GetCurrentInfoIndex() const;
	int  GetTabHeight();
	void DrawKeyTips(Draw& w);
	void PopupTabMenu(Point p);

public:
	static const Style& StyleDefault();
};

#endif
