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
	else if (selected)
		row_bg = owner->bg_selected;
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
	if (owner->HasSelectionRange()) {
		int a = owner->GetRangeStart();
		int b = owner->GetRangeEnd();
		if (b >= kp_i && a <= kp_last) {
			int ra = max(a, kp_i);
			int rb = min(b, kp_last);
			int x0 = owner->title_tab_w + 1 + (ra - kp_i) * col;
			int x1 = owner->title_tab_w + 1 + (rb - kp_i + 1) * col;
			if (x1 > x0)
				d.DrawRect(x0, 0, x1 - x0, sz.cy, owner->range_bg);
		}
	}
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
	if (!owner)
		return false;
	dword base = key & ~(K_SHIFT|K_CTRL|K_ALT);
	int col = owner->selected_col;
	if ((key == K_LEFT || key == K_RIGHT) && col >= 0) {
		int delta = (key == K_LEFT) ? -1 : 1;
		int next_col = col + delta;
		next_col = min(max(next_col, 0), max(0, owner->length - 1));
		if (key & K_SHIFT) {
			if (owner->range_anchor < 0)
				owner->range_anchor = col;
			owner->SetSelectionRange(owner->range_anchor, next_col);
		}
		else {
			owner->range_anchor = next_col;
			owner->ClearSelectionRange();
		}
		if (next_col != owner->selected_col) {
			owner->selected_col = next_col;
			if (owner->WhenCursor)
				owner->WhenCursor(next_col);
		}
		owner->MakeColumnVisible(next_col);
		Refresh();
		return true;
	}
	if ((key == (K_CTRL|K_LEFT) || key == (K_CTRL|K_RIGHT)) && col >= 0) {
		int best = -1;
		for (int k = 0; k < keypoints.GetCount(); k++) {
			int kp = keypoints[k];
			if (key == (K_CTRL|K_LEFT)) {
				if (kp < col && (best < 0 || kp > best))
					best = kp;
			}
			else {
				if (kp > col && (best < 0 || kp < best))
					best = kp;
			}
		}
		if (best >= 0) {
			owner->selected_col = best;
			if (owner->WhenCursor)
				owner->WhenCursor(best);
			owner->MakeColumnVisible(best);
			Refresh();
		}
		return true;
	}
	if (base == K_K || base == K_INSERT) {
		if (owner->WhenKeyframeToggle)
			owner->WhenKeyframeToggle(id, owner->selected_col);
		return true;
	}
	if (base == K_DELETE || base == K_BACKSPACE) {
		if (owner->WhenKeyframeRemove)
			owner->WhenKeyframeRemove(id, owner->selected_col);
		return true;
	}
	if (base == K_A) {
		if (owner->WhenToggleAutoKey)
			owner->WhenToggleAutoKey();
		return true;
	}
	return false;
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
			owner->SelectRow(id, keyflags);
			drag_title = true;
			if (owner->WhenRowSelect)
				owner->WhenRowSelect(id);
		}
	}
	int col = owner->GetColumnWidth();
	int x = p.x - owner->title_tab_w - 1;
	int kp_i = (col > 0) ? owner->hsb / col + x / col : -1;
	
	bool shift = (keyflags & K_SHIFT);
	if (kp_i >= 0 && kp_i < owner->length) {
		bool on_kp = owner->IsKeyframeAt(id, kp_i);
		if (on_kp && !(keyflags & K_CTRL)) {
			drag_keyframe = true;
			drag_keyframe_frame = kp_i;
		}
		if (shift) {
			if (owner->range_anchor < 0)
				owner->range_anchor = owner->selected_col >= 0 ? owner->selected_col : kp_i;
			owner->SetSelectionRange(owner->range_anchor, kp_i);
		}
		else {
			owner->range_anchor = kp_i;
			owner->ClearSelectionRange();
		}
		if (kp_i != owner->selected_col) {
			owner->selected_col = kp_i;
			if (owner->WhenCursor)
				owner->WhenCursor(kp_i);
		}
		owner->MakeColumnVisible(kp_i);
		dragging = true;
		drag_range = shift;
		SetCapture();
	}
	
	if (!HasFocus())
		SetFocus();
	else
		Refresh();
	
}

void TimelineRowCtrl::LeftDrag(Point p, dword keyflags) {
	if (!owner)
		return;
	if (drag_keyframe) {
		int col = owner->GetColumnWidth();
		int x = p.x - owner->title_tab_w - 1;
		int kp_i = (col > 0) ? owner->hsb / col + x / col : -1;
		if (kp_i < 0 || kp_i >= owner->length)
			return;
		if (kp_i != owner->selected_col) {
			owner->selected_col = kp_i;
			if (owner->WhenCursor)
				owner->WhenCursor(kp_i);
			owner->MakeColumnVisible(kp_i);
			Refresh();
		}
		return;
	}
	if (drag_title) {
		Point screen = GetScreenRect().TopLeft() + p;
		Point local = screen - owner->GetScreenRect().TopLeft();
		int row = owner->GetRowAt(local);
		if (row >= 0) {
			owner->SelectRowRange(id, row);
		}
		return;
	}
	if (!dragging)
		return;
	int col = owner->GetColumnWidth();
	int x = p.x - owner->title_tab_w - 1;
	int kp_i = (col > 0) ? owner->hsb / col + x / col : -1;
	if (kp_i < 0 || kp_i >= owner->length)
		return;
	if (drag_range) {
		if (owner->range_anchor < 0)
			owner->range_anchor = owner->selected_col >= 0 ? owner->selected_col : kp_i;
		owner->SetSelectionRange(owner->range_anchor, kp_i);
	}
	else {
		owner->range_anchor = kp_i;
		owner->ClearSelectionRange();
	}
	if (kp_i != owner->selected_col) {
		owner->selected_col = kp_i;
		if (owner->WhenCursor)
			owner->WhenCursor(kp_i);
		owner->MakeColumnVisible(kp_i);
		Refresh();
	}
}

void TimelineRowCtrl::LeftUp(Point p, dword keyflags) {
	if (HasCapture())
		ReleaseCapture();
	if (drag_keyframe && owner && drag_keyframe_frame >= 0) {
		int col = owner->GetColumnWidth();
		int x = p.x - owner->title_tab_w - 1;
		int kp_i = (col > 0) ? owner->hsb / col + x / col : -1;
		if (kp_i >= 0 && kp_i < owner->length && kp_i != drag_keyframe_frame) {
			if (owner->WhenKeyframeMove)
				owner->WhenKeyframeMove(id, drag_keyframe_frame, kp_i);
		}
	}
	dragging = false;
	drag_title = false;
	drag_keyframe = false;
	drag_keyframe_frame = -1;
	Ctrl::LeftUp(p, keyflags);
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
	range_bg = Blend(accent, bg, 220);
	bg_selected = Blend(bg, accent, 220);
	
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

void TimelineCtrl::SetSelectedColumn(int i) {
	if (length <= 0) {
		selected_col = 0;
		return;
	}
	int clamped = min(max(i, 0), max(0, length - 1));
	selected_col = clamped;
	MakeColumnVisible(clamped);
	Refresh();
}

void TimelineCtrl::SetSelectionRange(int a, int b) {
	if (length <= 0) {
		ClearSelectionRange();
		return;
	}
	range_start = min(a, b);
	range_end = max(a, b);
	range_start = min(max(range_start, 0), max(0, length - 1));
	range_end = min(max(range_end, 0), max(0, length - 1));
	if (WhenRangeSelect)
		WhenRangeSelect(range_start, range_end);
	Refresh();
}

void TimelineCtrl::ClearSelectionRange() {
	range_start = -1;
	range_end = -1;
	Refresh();
}

void TimelineCtrl::MakeColumnVisible(int col) {
	int colw = GetColumnWidth();
	if (colw <= 0)
		return;
	Size sz = GetSize();
	int vieww = max(0, sz.cx - title_tab_w);
	int visible = vieww > 0 ? vieww / colw : 0;
	if (visible <= 0)
		return;
	int first = hsb.Get() / colw;
	int last = first + visible - 1;
	if (col < first)
		hsb.Set(col * colw);
	else if (col > last)
		hsb.Set(max(0, (col - visible + 1) * colw));
}

Vector<int> TimelineCtrl::GetSelectedRows() const {
	Vector<int> out;
	out.SetCount(selected_rows.GetCount());
	for (int i = 0; i < selected_rows.GetCount(); i++)
		out[i] = selected_rows[i];
	return out;
}

bool TimelineCtrl::IsKeyframeAt(int row, int frame) const {
	if (row < 0 || row >= rows.GetCount())
		return false;
	for (int i = 0; i < rows[row].keypoints.GetCount(); i++) {
		if (rows[row].keypoints[i] == frame)
			return true;
	}
	return false;
}

void TimelineCtrl::SelectRow(int row, dword keyflags) {
	if (row < 0 || row >= rows.GetCount())
		return;
	bool shift = (keyflags & K_SHIFT);
	bool ctrl = (keyflags & K_CTRL);
	if (shift) {
		if (row_anchor < 0)
			row_anchor = row;
		SelectRowRange(row_anchor, row);
		return;
	}
	if (ctrl) {
		ToggleRowSelection(row);
		row_anchor = row;
		return;
	}
	selected_rows.Clear();
	selected_rows.Add(row);
	row_anchor = row;
	for (int i = 0; i < rows.GetCount(); i++)
		rows[i].SetSelected(selected_rows.Find(i) >= 0);
	Refresh();
}

void TimelineCtrl::SelectRowRange(int a, int b) {
	if (a < 0 || b < 0)
		return;
	int lo = min(a, b);
	int hi = max(a, b);
	selected_rows.Clear();
	for (int i = lo; i <= hi; i++) {
		selected_rows.Add(i);
	}
	for (int i = 0; i < rows.GetCount(); i++)
		rows[i].SetSelected(selected_rows.Find(i) >= 0);
	Refresh();
}

void TimelineCtrl::ClearRowSelection() {
	selected_rows.Clear();
	for (int i = 0; i < rows.GetCount(); i++)
		rows[i].SetSelected(false);
	Refresh();
}

void TimelineCtrl::ToggleRowSelection(int row) {
	int idx = selected_rows.Find(row);
	if (idx >= 0)
		selected_rows.Remove(idx);
	else
		selected_rows.Add(row);
	for (int i = 0; i < rows.GetCount(); i++)
		rows[i].SetSelected(selected_rows.Find(i) >= 0);
	Refresh();
}

int TimelineCtrl::GetRowAt(Point p) const {
	int y = p.y + vsb.Get();
	int row = y / max(1, line_height);
	if (row < 0 || row >= rows.GetCount())
		return -1;
	return row;
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
