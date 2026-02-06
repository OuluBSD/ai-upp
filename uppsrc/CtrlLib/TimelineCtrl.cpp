#include "CtrlLib.h"

NAMESPACE_UPP

TimelineRowCtrl::TimelineRowCtrl() {
	WantFocus();
	
}

void TimelineRowCtrl::Paint(Draw& d) {
	Size sz = GetSize();
	
	bool has_focus = HasFocus();
	Color row_bg = owner->bg;
	if (active)
		row_bg = owner->bg_active;
	else if (has_focus)
		row_bg = owner->bg_focused;
	d.DrawRect(sz, row_bg);
	
	Font fnt = SansSerif(sz.cy-6);
	fnt.Bold();
	int xpad = 2 + indent * 12;
	if (has_children) {
		String mark = expanded ? "-" : "+";
		d.DrawRect(xpad, 2, 10, sz.cy - 4, Blend(owner->accent, owner->bg, 220));
		d.DrawText(xpad + 2, 2, mark, fnt, owner->text);
		xpad += 12;
	}
	d.DrawText(xpad, 2, title, fnt, owner->text);
	
	int col = owner->GetColumnWidth();
	int x = owner->title_tab_w + 1;
	int kp_i = owner->hsb / col;
	int visible_cols = (col > 0) ? (sz.cx - owner->title_tab_w) / col + 2 : 0;
	int kp_last = min(owner->length, kp_i + max(0, visible_cols));
	int line_mid = sz.cy / 2;
	Color tween_clr = Blend(owner->kp_col_accent, row_bg, 200);
	if (keypoints.GetCount() > 1) {
		for (int i = 1; i < keypoints.GetCount(); i++) {
			int a = keypoints[i - 1];
			int b = keypoints[i];
		if (b < kp_i || a > kp_last)
				continue;
			int x0 = owner->title_tab_w + 1 + (a - kp_i) * col;
			int x1 = owner->title_tab_w + 1 + (b - kp_i) * col;
			d.DrawLine(x0, line_mid, x1, line_mid, 1, tween_clr);
		}
	}
	
	int* kp_iter = keypoints.Begin();
	int* kp_end = keypoints.End();
	int selected_i = owner->selected_col;
	bool has_kp = keypoints.GetCount() > 0;
	
	for(; kp_i < owner->length; kp_i++) {
		bool first_in_second = (kp_i % owner->kps) == 0;
		
		bool is_keypoint = false;
		if (kp_iter != kp_end) {
			while (*kp_iter < kp_i && kp_iter != kp_end)
				kp_iter++;
			if (kp_iter != kp_end && *kp_iter == kp_i)
				is_keypoint = true;
		}
		
		if (selected_i == kp_i) {
			Color sel = owner->bg_focused_keypoint;
			if (active)
				sel = owner->bg_active_keypoint;
			d.DrawRect(x, 0, col, sz.cy, sel);
		}
		
		if (first_in_second) {
			d.DrawLine(x, 0, x, sz.cy - 1, 1, owner->kp_second_accent);
		}
		else {
			int y0 = sz.cy - 4;
			d.DrawLine(x, y0, x, sz.cy - 1, 1, Blend(owner->kp_col_accent, row_bg, 220));
		}
		
		if (is_keypoint) {
			int pad = 3;
			int rad = col - pad * 2;
			d.DrawRect(x + pad, pad, rad, rad, Black());
		}
		else if (!has_kp && kp_i == 0) {
			int pad = 3;
			int rad = col - pad * 2;
			d.DrawEllipse(x + pad, pad, rad, rad, Black());
			d.DrawEllipse(x + pad + 1, pad + 1, rad - 2, rad - 2, White());
		}
		
		x += col;
	}
	
	d.DrawLine(0, sz.cy-1, sz.cx-1, sz.cy-1, 1, owner->accent);
	d.DrawLine(owner->title_tab_w, 0, owner->title_tab_w, sz.cy-1, 1, owner->accent);
}

bool TimelineRowCtrl::Key(dword key, int) {
	
	return true;
}

void TimelineRowCtrl::LeftDown(Point p, dword keyflags) {
	if (owner) {
		if (p.x < owner->title_tab_w) {
			int xpad = 2 + indent * 12;
			if (has_children && p.x >= xpad && p.x <= xpad + 10) {
				expanded = !expanded;
				if (owner->WhenRowToggle)
					owner->WhenRowToggle(id);
				Refresh();
				return;
			}
			if (owner->WhenRowSelect)
				owner->WhenRowSelect(id);
		}
	}
	int col = owner->GetColumnWidth();
	int x = p.x - owner->title_tab_w - 1;
	int kp_i = x / col;
	
	bool kp_changes = false;
	if (kp_i >= 0 && kp_i < owner->length && kp_i != owner->selected_col) {
		kp_changes = true;
		owner->selected_col = kp_i;
	}
	
	if (!HasFocus())
		SetFocus();
	else
		Refresh();
	
	if (kp_changes)
		owner->WhenCursor(kp_i);
	
}

void TimelineRowCtrl::RightDown(Point p, dword keyflags) {
	if (!owner)
		return;
	if (owner->WhenRowMenu) {
		MenuBar::Execute([&](Bar& bar) { owner->WhenRowMenu(bar, id); }, GetMousePos());
		return;
	}
}

void TimelineRowCtrl::GotFocus() {
	Refresh();
	
}

void TimelineRowCtrl::LostFocus() {
	Refresh();
	
}

void TimelineRowCtrl::SetKeypoints(const Vector<int>& keypoints) {
	this->keypoints.SetCount(keypoints.GetCount());
	if (!keypoints.IsEmpty())
		memcpy(this->keypoints.Begin(), keypoints.Begin(), sizeof(int) * keypoints.GetCount());
	Refresh();
}









TimelineCtrl::TimelineCtrl() {
	bg = Color(218, 222, 228);
	bg_focused = Color(218-64, 222-64, 228-64);
	bg_focused_keypoint = Color(218-128, 222-128, 255);
	bg_active = Color(210, 216, 228);
	bg_active_keypoint = Color(200, 210, 255);
	accent = Color(82, 87, 91);
	text = Color(33, 34, 36);
	kp_second_accent = text;
	kp_col_accent = accent;
	
	AddFrame(vsb);
	AddFrame(hsb.Horz());
	
	vsb.WhenScroll = THISBACK(OnScroll);
	vsb.SetLine(GetLineHeight());
	hsb.WhenScroll = THISBACK(OnScroll);
	hsb.SetLine(GetColumnWidth());
}

void TimelineCtrl::OnScroll() {
	Refresh();
}

void TimelineCtrl::Paint(Draw& d) {
	Size sz = GetSize();
	
	d.DrawRect(sz, bg);
	
}

void TimelineCtrl::LeftDown(Point p, dword keyflags) {
	
}

void TimelineCtrl::RightDown(Point p, dword keyflags) {
	if (WhenMenu)
		MenuBar::Execute(WhenMenu, GetMousePos());
}

void TimelineCtrl::MouseWheel(Point p, int zdelta, dword keyflags) {
	if (keyflags & K_SHIFT)
		hsb.Wheel(zdelta);
	else
		vsb.Wheel(zdelta);
}

bool TimelineCtrl::Key(dword key, int) {
	return vsb.VertKey(key) || hsb.HorzKey(key);
}

/*TimelineRowCtrl& TimelineCtrl::GetAddRow(int id) {
	int i = rows.Find(id);
	if (i >= 0)
		return rows[i];
	
	TimelineRowCtrl& row = rows.GetAdd(id);
	row.owner = this;
	row.id = id;
	
	sb.SetTotal(rows.GetCount() * GetLineHeight());
	
	Ctrl::Add(row);
	Layout();
	
	return row;
}*/

void TimelineCtrl::SetCount(int c) {
	int old_count = rows.GetCount();
	rows.SetCount(c);
	
	for(int i = old_count; i < c; i++) {
		rows[i].owner = this;
		rows[i].id = i;
		Add(rows[i]);
	}
	
	vsb.SetTotal(rows.GetCount() * GetLineHeight());
	Layout();
}

void TimelineCtrl::SetLength(int i) {
	length = i;
	
	hsb.SetTotal(length * GetColumnWidth());
}

void TimelineCtrl::SetKeypointColumnWidth(int i) {
	kp_col = i;
	Layout();
	Refresh();
}

TimelineRowCtrl& TimelineCtrl::GetRowIndex(int i) {
	return rows[i];
}

void TimelineCtrl::Layout() {
	Size sz = GetSize();
	vsb.SetPage(sz.cy);

	int fcy = GetLineHeight();
	int i = vsb / fcy;
	int y = i * fcy - vsb;
	
	Size row_sz(sz.cx, fcy);
	for(; i < rows.GetCount(); i++) {
		TimelineRowCtrl& row = rows[i];
		
		row.SetRect(RectC(0, y, row_sz.cx, row_sz.cy));
		
		y += fcy;
	}
}

END_UPP_NAMESPACE
