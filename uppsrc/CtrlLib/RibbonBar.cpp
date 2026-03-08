#include "CtrlLib.h"

NAMESPACE_UPP

RibbonGroup::RibbonGroup()
{
	style = &RibbonBar::StyleDefault();
	label_gap = style->label_gap;
	lbl.SetAlign(ALIGN_CENTER);
	lbl.SetFont(style->full_font);
	Add(list);
	Add(lbl);
	list.Hide();
	list.ShowText(true);
	show_list_text = true;
	use_large = true;
	content_min = Size(0, 0);
}

RibbonGroup& RibbonGroup::SetStyle(const RibbonStyle& s)
{
	style = &s;
	label_gap = style->label_gap;
	lbl.SetFont(style->full_font);
	list.SetStyle(*style);
	for(int i = 0; i < large_btn.GetCount(); i++) {
		large_btn[i].SetRibbonIconArea(style->full_icon_area)
		            .SetRibbonIconMax(style->full_icon)
		            .SetRibbonFont(style->full_font);
	}
	Refresh();
	return *this;
}

RibbonGroup& RibbonGroup::SetContentMinSize(Size sz)
{
	content_min = sz;
	RefreshParentLayout();
	return *this;
}

RibbonGroup& RibbonGroup::SetLabel(const String& text)
{
	lbl.SetText(text);
	return *this;
}

RibbonGroup& RibbonGroup::SetLarge(Event<Bar&> barproc)
{
	class LargeBar : public Bar {
	public:
		struct ItemData : public Item {
			String text;
			::Upp::Image image;
			Event<> cb;
			Event<Bar&> submenu;
			bool enabled;

			Item& Text(const char *t) override             { text = t; return *this; }
			Item& Image(const ::Upp::Image& img) override  { image = img; return *this; }
			Item& Enable(bool e = true) override           { enabled = e; return *this; }

			ItemData() { enabled = true; }
		};

		Array<ItemData> items;

		Item& AddItem(Event<> cb) override
		{
			ItemData& it = items.Add();
			it.cb = cb;
			return it;
		}

		Item& AddSubMenu(Event<Bar&> proc) override
		{
			ItemData& it = items.Add();
			it.submenu = proc;
			return it;
		}

		void AddCtrl(Ctrl *ctrl, int gapsize) override {}
		void AddCtrl(Ctrl *ctrl, Size sz) override {}
		bool IsEmpty() const override { return items.IsEmpty(); }
		void Separator() override {}
	};

	LargeBar b;
	barproc(b);

	for(int i = 0; i < large_btn.GetCount(); i++)
		large_btn[i].Remove();
	large_btn.Clear();

	for(int i = 0; i < b.items.GetCount(); i++) {
		LargeBar::ItemData& it = b.items[i];
		Button& btn = large_btn.Add();
		Add(btn);
		btn.SetRibbonMode(Button::RIBBON_LARGE)
		   .SetRibbonIconArea(style->full_icon_area)
		   .SetRibbonIconMax(style->full_icon)
		   .SetRibbonFont(style->full_font);
		btn.SetRibbonLook(style->group_look, style->list_hot, style->list_push);
		btn.SetLabel(it.text);
		if(it.text.IsEmpty())
			btn.SetLabel(" ");
		btn.SetImage(it.image);
		btn.ShowRibbonLabel(true);
		btn.Enable(it.enabled);
		if(it.submenu)
			btn.SetRibbonMenu(it.submenu);
		btn.WhenAction = it.cb;
		btn.Show();
		LOG("RibbonBar full-button: text='" << it.text << "' image=" << (IsNull(it.image) ? "null" : "set")
		    << " size=" << btn.GetSize() << " enabled=" << it.enabled);
	}

	use_large = true;
	list.Hide();
	if(list_ctrl)
		list_ctrl->Hide();
	return *this;
}

RibbonGroup& RibbonGroup::SetList(Event<Bar&> barproc)
{
	list.Show();
	list.ShowText(show_list_text);
	list.Set(barproc);
	for(int i = 0; i < large_btn.GetCount(); i++)
		large_btn[i].Hide();
	use_large = false;
	if(list_ctrl)
		list_ctrl->Hide();
	return *this;
}

RibbonGroup& RibbonGroup::SetListCtrl(Ctrl& ctrl)
{
	list_ctrl = &ctrl;
	Add(ctrl);
	ctrl.Show();
	list.Hide();
	for(int i = 0; i < large_btn.GetCount(); i++)
		large_btn[i].Hide();
	use_large = false;
	return *this;
}

RibbonGroup& RibbonGroup::ShowListText(bool show)
{
	show_list_text = show;
	list.ShowText(show);
	return *this;
}

RibbonGroup& RibbonGroup::EnableList(bool enable)
{
	list.Enable(enable);
	return *this;
}

Size RibbonGroup::GetMinSize() const
{
	Size l = lbl.GetMinSize();
	Size b(0, 0);
	if(list_ctrl)
		b = list_ctrl->GetMinSize();
	else if(list.IsShown())
		b = list.GetMinSize();
	else {
		int cx = large_btn.GetCount() * style->full_button.cx;
		int cy = style->full_button.cy;
		b = Size(cx, cy);
	}
	if(content_min.cx)
		b.cx = max(b.cx, content_min.cx);
	if(content_min.cy)
		b.cy = max(b.cy, content_min.cy);
	int cy = b.cy + label_gap + l.cy;
	int cx = max(b.cx, l.cx);
	return Size(cx, cy);
}

void RibbonGroup::Layout()
{
	Size sz = GetSize();
	int  label_cy = lbl.GetMinSize().cy;
	int  bar_cy = max(0, sz.cy - label_cy - label_gap);
	list.LeftPos(0, sz.cx).TopPos(0, bar_cy);
	if(list_ctrl)
		list_ctrl->SetRect(0, 0, sz.cx, bar_cy);
	if(use_large) {
		int x = 0;
		for(int i = 0; i < large_btn.GetCount(); i++) {
			Button& b = large_btn[i];
			int cx = style->full_button.cx;
			int cy = min(bar_cy, style->full_button.cy);
			b.SetRect(x, 0, cx, cy);
			x += cx;
		}
	}
	lbl.LeftPos(0, sz.cx).BottomPos(0, label_cy);
}

void RibbonGroup::Paint(Draw& w)
{
	if(style)
		w.DrawRect(GetSize(), style->group_look);
}

RibbonPage::RibbonPage()
{
	group_gap = RibbonBar::StyleDefault().group_gap;
}

RibbonGroup& RibbonPage::AddGroup(const String& label)
{
	RibbonGroup& g = group.Add();
	g.SetLabel(label);
	g.SetStyle(RibbonBar::StyleDefault());
	Add(g);
	if(group.GetCount() > 1) {
		SeparatorCtrl& s = sep.Add();
		Add(s);
	}
	return g;
}

RibbonGroup& RibbonPage::AddGroup(const String& label, Event<Bar&> bar)
{
	RibbonGroup& g = AddGroup(label);
	g.SetLarge(bar);
	return g;
}

void RibbonPage::ClearGroups()
{
	group.Clear();
	sep.Clear();
}

Size RibbonPage::GetMinSize() const
{
	int cx = 0;
	int cy = 0;
	for(int i = 0; i < group.GetCount(); i++) {
		Size sz = group[i].GetMinSize();
		cx += sz.cx;
		if(i)
			cx += group_gap;
		cy = max(cy, sz.cy);
	}
	return Size(cx, cy);
}

void RibbonPage::Layout()
{
	Size sz = GetSize();
	int  x = 0;
	int  sep_i = 0;
	for(int i = 0; i < group.GetCount(); i++) {
		Size gs = group[i].GetMinSize();
		group[i].SetRect(x, 0, gs.cx, sz.cy);
		x += gs.cx + group_gap;
		if(i < group.GetCount() - 1 && sep_i < sep.GetCount()) {
			sep[sep_i].SetRect(x - group_gap / 2, DPI(6), 1, sz.cy - DPI(12));
			sep_i++;
		}
	}
}

void RibbonBar::RibbonTabCtrl::LeftDouble(Point p, dword keyflags)
{
	TabCtrl::LeftDouble(p, keyflags);
	WhenDouble();
}

void RibbonBar::RibbonTabCtrl::LeftDown(Point p, dword keyflags)
{
	int tab_i = GetTab(p);
	if(tab_i >= 0 && tab_i == Get())
		WhenSame(tab_i);
	TabCtrl::LeftDown(p, keyflags);
}

void RibbonBar::RibbonTabCtrl::RightDown(Point p, dword keyflags)
{
	TabCtrl::RightDown(p, keyflags);
	WhenMenu(p);
}

RibbonBar::RibbonBar()
{
	display_mode = RIBBON_ALWAYS;
	qat_pos = QAT_TOP;
	expanded = true;
	show_keytips = false;
	style = &StyleDefault();

	Add(tabs);
	Add(page_area);
	Add(qat);

	tabs.WhenSet = [=] { OnTab(); };
	tabs.WhenDouble = [=] { ToggleExpanded(); };
	tabs.WhenMenu = [=](Point p) { PopupTabMenu(p); };
	tabs.WhenSame = [=](int) {
		if(display_mode == RIBBON_TABS)
			ToggleExpanded();
	};
	qat.NoDarkAdjust();
}

RibbonPage& RibbonBar::AddTab(const String& text)
{
	RibbonPage& p = page.Add();
	page_area.Add(p.SizePos());
	p.Hide();

	TabInfo& ti = tab.Add();
	ti.text = text;
	ti.contextual = false;
	ti.visible = true;
	ti.context.Clear();
	ti.page_index = page.GetCount() - 1;

	RebuildTabs();
	return p;
}

RibbonPage& RibbonBar::AddContextTab(const String& context, const String& text)
{
	RibbonPage& p = page.Add();
	page_area.Add(p.SizePos());
	p.Hide();

	TabInfo& ti = tab.Add();
	ti.text = text;
	ti.contextual = true;
	ti.visible = false;
	ti.context = context;
	ti.page_index = page.GetCount() - 1;

	RebuildTabs();
	return p;
}

void RibbonBar::RemoveTab(int tab_i)
{
	if(tab_i < 0 || tab_i >= tab.GetCount())
		return;
	int page_index = tab[tab_i].page_index;
	tab.Remove(tab_i);
	if(page_index >= 0 && page_index < page.GetCount())
		page.Remove(page_index);
	for(int i = 0; i < tab.GetCount(); i++)
		if(tab[i].page_index > page_index)
			tab[i].page_index--;
	RebuildTabs();
}

void RibbonBar::ClearTabs()
{
	tab.Clear();
	page.Clear();
	tabs.Reset();
	visible_map.Clear();
	page_area.Refresh();
	UpdateVisibility();
}

RibbonBar& RibbonBar::RenameTab(int tab_i, const String& text)
{
	if(tab_i < 0 || tab_i >= tab.GetCount())
		return *this;
	tab[tab_i].text = text;
	RebuildTabs();
	return *this;
}

RibbonBar& RibbonBar::ShowContext(const String& context, bool show)
{
	for(int i = 0; i < tab.GetCount(); i++)
		if(tab[i].contextual && tab[i].context == context)
			tab[i].visible = show;
	RebuildTabs();
	return *this;
}

RibbonBar& RibbonBar::SetDisplayMode(DisplayMode mode)
{
	display_mode = mode;
	if(display_mode == RIBBON_ALWAYS)
		expanded = true;
	else if(display_mode == RIBBON_TABS)
		expanded = false;
	UpdateVisibility();
	return *this;
}

RibbonBar& RibbonBar::SetQuickAccess(Event<Bar&> bar)
{
	qat.Set(bar);
	UpdateVisibility();
	return *this;
}

RibbonBar& RibbonBar::SetQuickAccessPos(QuickAccessPos pos)
{
	qat_pos = pos;
	RefreshParentLayout();
	return *this;
}

RibbonBar& RibbonBar::Expand()
{
	expanded = true;
	UpdateVisibility();
	return *this;
}

RibbonBar& RibbonBar::Collapse()
{
	if(display_mode == RIBBON_ALWAYS)
		return *this;
	expanded = false;
	UpdateVisibility();
	return *this;
}

RibbonBar& RibbonBar::ToggleExpanded()
{
	if(display_mode == RIBBON_ALWAYS)
		display_mode = RIBBON_TABS;
	expanded = !expanded;
	UpdateVisibility();
	return *this;
}

bool RibbonBar::Key(dword key, int count)
{
	if(key == (K_CTRL|K_F1)) {
		if(display_mode == RIBBON_ALWAYS)
			SetDisplayMode(RIBBON_TABS);
		else
			SetDisplayMode(RIBBON_ALWAYS);
		return true;
	}
	if(key == K_ALT_KEY) {
		show_keytips = !show_keytips;
		Refresh();
		return true;
	}
	return ParentCtrl::Key(key, count);
}

void RibbonBar::Layout()
{
	Size sz = GetSize();
	int  tab_h = GetTabHeight();
	int  qat_h = qat.IsShown() ? qat.GetMinSize().cy : 0;
	int  y = 0;
	bool show_content = (display_mode == RIBBON_ALWAYS) || expanded;

	if(qat_pos == QAT_TOP) {
		if(qat_h) {
			qat.TopPos(0, qat_h).HSizePos();
			y += qat_h;
		}
	}

	tabs.TopPos(y, tab_h).HSizePos();
	y += tab_h;

	if(qat_pos == QAT_BOTTOM && qat_h) {
		if(show_content) {
			page_area.SetRect(0, y, sz.cx, max(0, sz.cy - y - qat_h));
			page_area.Show();
		}
		else {
			page_area.SetRect(0, y, sz.cx, 0);
			page_area.Hide();
		}
		qat.BottomPos(0, qat_h).HSizePos();
	}
	else {
		if(show_content) {
			page_area.SetRect(0, y, sz.cx, max(0, sz.cy - y));
			page_area.Show();
		}
		else {
			page_area.SetRect(0, y, sz.cx, 0);
			page_area.Hide();
		}
	}
}

void RibbonBar::Paint(Draw& w)
{
	if(style)
		w.DrawRect(GetSize(), style->group_look);
	else
		w.DrawRect(GetSize(), SColorFace());
	ParentCtrl::Paint(w);
	DrawKeyTips(w);
}

void RibbonBar::MouseLeave()
{
	if(display_mode == RIBBON_AUTOHIDE && expanded) {
		expanded = false;
		UpdateVisibility();
	}
	ParentCtrl::MouseLeave();
}

void RibbonBar::RebuildTabs()
{
	int current = GetCurrentInfoIndex();
	tabs.Reset();
	visible_map.Clear();
	for(int i = 0; i < tab.GetCount(); i++) {
		if(!tab[i].visible)
			continue;
		tabs.Add(tab[i].text);
		visible_map.Add(i);
	}
	if(visible_map.IsEmpty()) {
		UpdateVisibility();
		return;
	}
	int select = 0;
	for(int i = 0; i < visible_map.GetCount(); i++) {
		if(visible_map[i] == current) {
			select = i;
			break;
		}
	}
	tabs.Set(select);
	OnTab();
}

void RibbonBar::SyncSelection()
{
	int info = GetCurrentInfoIndex();
	for(int i = 0; i < page.GetCount(); i++) {
		bool show = (i == (info >= 0 ? tab[info].page_index : -1));
		if(show)
			page[i].Show();
		else
			page[i].Hide();
	}
}

void RibbonBar::UpdateVisibility()
{
	bool show_content = (display_mode == RIBBON_ALWAYS) || expanded;
	if(display_mode == RIBBON_ALWAYS)
		expanded = true;

	if(show_content)
		SyncSelection();
	else
		for(int i = 0; i < page.GetCount(); i++)
			page[i].Hide();

	int tab_h = GetTabHeight();
	int qat_h = qat.IsShown() ? qat.GetMinSize().cy : 0;
	int content_h = 0;
	int info = GetCurrentInfoIndex();
	if(show_content && info >= 0) {
		int page_index = tab[info].page_index;
		if(page_index >= 0 && page_index < page.GetCount())
			content_h = page[page_index].GetMinSize().cy;
	}
	Height(tab_h + qat_h + content_h);
	RefreshParentLayout();
	Refresh();
}

void RibbonBar::OnTab()
{
	if(display_mode != RIBBON_ALWAYS)
		expanded = true;
	UpdateVisibility();
	int info = GetCurrentInfoIndex();
	if(info >= 0)
		WhenTab(info);
}

int RibbonBar::GetCurrentInfoIndex() const
{
	int idx = tabs.Get();
	if(idx < 0 || idx >= visible_map.GetCount())
		return -1;
	return visible_map[idx];
}

int RibbonBar::GetTabHeight()
{
	return tabs.ComputeSize().cy;
}

void RibbonBar::DrawKeyTips(Draw& w)
{
	if(!show_keytips)
		return;
	int info = GetCurrentInfoIndex();
	if(info < 0 || info >= tab.GetCount())
		return;
	String title = tab[info].text;
	if(IsNull(title))
		return;
	String key = ToUpper(title.Left(1));
	Font fnt = StdFont().Height(10).Bold();
	Size tsz = GetTextSize(key, fnt);
	Rect tr = tabs.GetRect();
	Point p(tr.left + DPI(8), tr.top + DPI(4));
	w.DrawRect(p.x, p.y, tsz.cx + DPI(6), tsz.cy + DPI(4), Yellow());
	w.DrawText(p.x + DPI(3), p.y + DPI(2), key, fnt, Black());
}

void RibbonBar::PopupTabMenu(Point p)
{
	if(WhenTabMenu) {
		MenuBar::Execute(&tabs, [&](Bar& bar) { WhenTabMenu(bar); }, p);
		return;
	}
	MenuBar::Execute(&tabs, [&](Bar& bar) {
		bar.Add("Always show ribbon", [=] { SetDisplayMode(RIBBON_ALWAYS); }).Check(display_mode == RIBBON_ALWAYS);
		bar.Add("Show tabs", [=] { SetDisplayMode(RIBBON_TABS); }).Check(display_mode == RIBBON_TABS);
		bar.Add("Auto-hide", [=] { SetDisplayMode(RIBBON_AUTOHIDE); }).Check(display_mode == RIBBON_AUTOHIDE);
		bar.Separator();
		bar.Add(expanded ? "Collapse" : "Expand", [=] { ToggleExpanded(); });
	}, p);
}

const RibbonBar::Style& RibbonBar::StyleDefault()
{
	static RibbonBar::Style s;
	ONCELOCK {
		s.full_button = Size(DPI(62), DPI(104));
		s.full_icon_area = Size(DPI(52), DPI(56));
		s.full_icon = Size(DPI(45), DPI(40));
		s.list_icon = Size(DPI(22), DPI(20));
		s.list_row_cy = DPI(28);
		s.label_gap = DPI(4);
		s.group_gap = DPI(6);
		s.full_font = StdFont().Height(13);
		s.list_font = StdFont().Height(12);
		s.group_look = SColorFace();
		s.list_look = SColorPaper();
		s.list_hot = Blend(SColorHighlight(), SColorPaper(), 128);
		s.list_push = SColorHighlight();
	}
	return s;
}

RibbonList::RibbonList()
{
	hot = -1;
	push = -1;
	show_text = true;
	style = &RibbonBar::StyleDefault();
	NoWantFocus();
}

RibbonList& RibbonList::SetStyle(const RibbonStyle& s)
{
	style = &s;
	for(int i = 0; i < btn.GetCount(); i++) {
		btn[i].SetRibbonMode(Button::RIBBON_LIST)
		      .SetRibbonIconArea(style->list_icon)
		      .SetRibbonIconMax(style->list_icon)
		      .SetRibbonFont(style->list_font)
		      .SetRibbonLook(style->list_look, style->list_hot, style->list_push)
		      .ShowRibbonLabel(show_text);
	}
	Refresh();
	return *this;
}

RibbonList& RibbonList::Clear()
{
	row.Clear();
	btn.Clear();
	Refresh();
	return *this;
}

RibbonList& RibbonList::Add(const Image& icon, const String& text, Event<> cb)
{
	Row& r = row.Add();
	r.icon = icon;
	r.text = text;
	r.WhenAction = cb;
	r.enabled = true;
	Button& b = btn.Add();
	ParentCtrl::Add(b);
	b.SetRibbonMode(Button::RIBBON_LIST)
	 .SetRibbonIconArea(style->list_icon)
	 .SetRibbonIconMax(style->list_icon)
	 .SetRibbonFont(style->list_font)
	 .SetRibbonLook(style->list_look, style->list_hot, style->list_push)
	 .ShowRibbonLabel(show_text);
	b.SetImage(icon);
	b.SetLabel(text);
	b.WhenAction = cb;
	b.Show();
	return *this;
}

RibbonList& RibbonList::AddCtrl(Ctrl& ctrl)
{
	Row& r = row.Add();
	r.enabled = true;
	r.extra = &ctrl;
	ParentCtrl::Add(ctrl);
	return *this;
}

RibbonList& RibbonList::Set(Event<Bar&> bar)
{
	Clear();

	class ListBar : public Bar {
	public:
		RibbonList *list;

		struct ListItem : public Item {
			String text;
			::Upp::Image  image;
			Event<> cb;
			Event<Bar&> submenu;
			bool   enabled;

			Item& Text(const char *t) override             { text = t; return *this; }
			Item& Image(const ::Upp::Image& img) override  { image = img; return *this; }
			Item& Enable(bool e = true) override           { enabled = e; return *this; }

			ListItem() { enabled = true; }
		};

		Item& AddItem(Event<> cb) override
		{
			ListItem& it = items.Add();
			it.cb = cb;
			return it;
		}

		Item& AddSubMenu(Event<Bar&> proc) override
		{
			ListItem& it = items.Add();
			it.submenu = proc;
			return it;
		}

		void AddCtrl(Ctrl *ctrl, int gapsize) override { if(ctrl) list->AddCtrl(*ctrl); }
		void AddCtrl(Ctrl *ctrl, Size sz) override     { if(ctrl) list->AddCtrl(*ctrl); }

		bool IsEmpty() const override                  { return items.IsEmpty(); }
		void Separator() override                      { list->Add(Image(), String(), Null); }

		Array<ListItem> items;

		ListBar() { list = nullptr; }
	} b;

	b.list = this;
	bar(b);

	for(int i = 0; i < b.items.GetCount(); i++) {
		ListBar::ListItem& it = b.items[i];
		Add(it.image, it.text, it.cb);
		int idx = btn.GetCount() - 1;
		if(idx >= 0) {
			btn[idx].Enable(it.enabled);
			if(it.submenu)
				btn[idx].SetRibbonMenu(it.submenu);
		}
	}
	Refresh();
	return *this;
}

RibbonList& RibbonList::ShowText(bool show)
{
	show_text = show;
	for(int i = 0; i < btn.GetCount(); i++)
		btn[i].ShowRibbonLabel(show);
	Refresh();
	return *this;
}

RibbonList& RibbonList::EnableRow(int i, bool enable)
{
	if(i < 0 || i >= row.GetCount())
		return *this;
	row[i].enabled = enable;
	if(i < btn.GetCount())
		btn[i].Enable(enable);
	Refresh();
	return *this;
}

Size RibbonList::GetMinSize() const
{
	int cx = 0;
	int cy = row.GetCount() * style->list_row_cy;
	for(int i = 0; i < row.GetCount(); i++) {
		Size extra(0, 0);
		if(row[i].extra)
			extra = row[i].extra->GetMinSize();
		int row_cx = style->list_icon.cx + DPI(8) + (show_text ? GetTextSize(row[i].text, style->list_font).cx : 0)
		             + (extra.cx ? extra.cx + DPI(6) : 0);
		cx = max(cx, row_cx);
	}
	return Size(cx, max(cy, style->full_button.cy));
}

void RibbonList::Layout()
{
	int y = 0;
	for(int i = 0; i < row.GetCount(); i++) {
		if(i < btn.GetCount())
			btn[i].SetRect(0, y, GetSize().cx, style->list_row_cy);
		if(row[i].extra) {
			Size es = row[i].extra->GetMinSize();
			row[i].extra->SetRect(GetSize().cx - es.cx - DPI(4), y + (style->list_row_cy - es.cy) / 2, es.cx, es.cy);
		}
		y += style->list_row_cy;
	}
}

int RibbonList::RowFromPoint(Point p) const
{
	int i = p.y / style->list_row_cy;
	return i >= 0 && i < row.GetCount() ? i : -1;
}

Rect RibbonList::RowRect(int i) const
{
	return Rect(0, i * style->list_row_cy, GetSize().cx, (i + 1) * style->list_row_cy);
}

void RibbonList::Paint(Draw& w)
{
	w.DrawRect(GetSize(), style->list_look);
}

void RibbonList::MouseMove(Point p, dword keyflags)
{
	Ctrl::MouseMove(p, keyflags);
}

void RibbonList::MouseLeave()
{
	Ctrl::MouseLeave();
}

void RibbonList::LeftDown(Point p, dword keyflags)
{
	Ctrl::LeftDown(p, keyflags);
}

END_UPP_NAMESPACE
