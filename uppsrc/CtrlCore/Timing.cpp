#include "CtrlCore.h"

namespace Upp {

TimingView::TimingView()
{
	WantFocus();
}

void TimingView::SetTimelineMode(bool b)
{
	if(timeline_mode == b)
		return;
	timeline_mode = b;
	dirty = true;
	Refresh();
}

void TimingView::SetContextFilter(const String& s)
{
	if(context_filter == s)
		return;
	context_filter = s;
	dirty = true;
	Refresh();
}

void TimingView::SetThreadFilter(uint64 id)
{
	if(thread_filter == id)
		return;
	thread_filter = id;
	dirty = true;
	Refresh();
}

void TimingView::SetTopRow(int row)
{
	top_row = max(0, row);
	Normalize();
	Refresh();
}

void TimingView::RefreshData()
{
	CollectData();
	Refresh();
}

void TimingView::SelectRow(int row)
{
	if(row < 0 || row >= RowCount())
		return;
	if(selected_row == row)
		return;
	selected_row = row;
	EnsureSelectionVisible();
	if(WhenSelect)
		WhenSelect(row);
	Refresh();
}

Vector<TimingRecord> TimingView::GetRecords() const
{
#ifdef flagTIMING
	return TimingManager::Global().GetRecords();
#else
	return Vector<TimingRecord>();
#endif
}

Vector<TimingSample> TimingView::GetTimeline() const
{
#ifdef flagTIMING
	return TimingManager::Global().GetTimeline();
#else
	return Vector<TimingSample>();
#endif
}

bool TimingView::MatchKey(const TimingKey& key) const
{
	if(thread_filter && key.thread_id != thread_filter)
		return false;
	if(!context_filter.IsEmpty() && key.context.Find(context_filter) < 0)
		return false;
	return true;
}

void TimingView::CollectData()
{
	dirty = false;
	records.Clear();
	timeline.Clear();
#ifdef flagTIMING
	Vector<TimingRecord> all_records = GetRecords();
	for(const TimingRecord& r : all_records)
		if(MatchKey(r.key))
			records.Add(r);
	Vector<TimingSample> all_timeline = GetTimeline();
	for(const TimingSample& s : all_timeline)
		if(MatchKey(s.key))
			timeline.Add(s);
#endif
	Normalize();
}

void TimingView::Normalize()
{
	int rows = RowCount();
	if(rows <= 0) {
		top_row = 0;
		selected_row = -1;
		return;
	}
	top_row = minmax(top_row, 0, max(0, rows - 1));
	if(selected_row >= rows)
		selected_row = rows - 1;
	if(selected_row < 0)
		selected_row = 0;
}

void TimingView::EnsureSelectionVisible()
{
	if(selected_row < 0)
		return;
	int rows = RowCount();
	if(rows <= 0)
		return;
	int visible = max(0, (GetSize().cy - (line_height + 2)) / line_height);
	if(visible <= 0)
		return;
	if(selected_row < top_row)
		top_row = selected_row;
	else if(selected_row >= top_row + visible)
		top_row = selected_row - visible + 1;
	Normalize();
}

int TimingView::RowCount() const
{
	return timeline_mode ? timeline.GetCount() : records.GetCount();
}

int TimingView::RowAt(Point p) const
{
	int header_h = line_height + 2;
	if(p.y < header_h)
		return -1;
	int row = top_row + (p.y - header_h) / max(1, line_height);
	return row >= 0 && row < RowCount() ? row : -1;
}

String TimingView::RowText(int row) const
{
	if(timeline_mode) {
		const TimingSample& s = timeline[row];
		String out;
		out << AsString(row) << "  "
		    << s.key.label
		    << "  ctx=" << (s.key.context.IsEmpty() ? String("<none>") : s.key.context)
		    << "  thr=" << Format64Hex(s.key.thread_id)
		    << "  elapsed=" << AsString(s.elapsed)
		    << "  nest=" << AsString(s.nesting)
		    << "  seq=" << AsString(s.sequence);
		if(!s.key.callstack.IsEmpty())
			out << "  cs=" << s.key.callstack;
		return out;
	}

	const TimingRecord& r = records[row];
	String out;
	out << AsString(row) << "  "
	    << r.key.label
	    << "  ctx=" << (r.key.context.IsEmpty() ? String("<none>") : r.key.context)
	    << "  thr=" << Format64Hex(r.key.thread_id)
	    << "  count=" << AsString(r.count)
	    << "  total=" << AsString(r.total_time)
	    << "  min=" << AsString(r.min_time)
	    << "  max=" << AsString(r.max_time)
	    << "  nest=" << AsString(r.max_nesting);
	if(!r.key.callstack.IsEmpty())
		out << "  cs=" << r.key.callstack;
	return out;
}

String TimingView::GetSelectedText() const
{
	if(selected_row < 0 || selected_row >= RowCount())
		return String();
	return RowText(selected_row);
}

void TimingView::Paint(Draw& w)
{
	if(dirty)
		CollectData();

	Size sz = GetSize();
	w.DrawRect(sz, SColorFace());

	Font f = GetStdFont();
	int header_h = line_height + 2;
	String header;
#ifdef flagTIMING
	header << "Timing ";
	header << (timeline_mode ? "[timeline]" : "[summary]");
	header << "  records=" << AsString(records.GetCount());
	header << "  samples=" << AsString(timeline.GetCount());
	header << "  ctx=" << (context_filter.IsEmpty() ? String("<all>") : context_filter);
	header << "  thr=" << (thread_filter ? Format64Hex(thread_filter) : String("<all>"));
#else
	header << "Timing disabled";
#endif
	w.DrawText(4, 2, header, f.Bold(), SColorText());
	w.DrawLine(0, header_h - 1, sz.cx, header_h - 1, 1, SColorShadow());

	int rows = RowCount();
	if(rows <= 0) {
		String msg = "No timing data";
#ifdef flagTIMING
		if(!TimingManager::Global().IsActive())
			msg = "Timing collection is disabled";
#endif
		w.DrawText(4, header_h + 6, msg, f, SColorText());
		return;
	}

	int visible = max(0, (sz.cy - header_h) / line_height);
	int end = min(rows, top_row + visible);
	int y = header_h;
	for(int i = top_row; i < end; ++i) {
		Color bg = (i & 1) ? Blend(SColorFace(), SColorShadow(), 240) : SColorFace();
		if(i == selected_row)
			bg = Blend(SColorHighlight(), SColorFace(), 210);
		w.DrawRect(0, y, sz.cx, line_height, bg);
		String text = RowText(i);
		w.DrawText(4, y + 1, text, f, SColorText());
		y += line_height;
	}
}

bool TimingView::Key(dword key, int)
{
	int rows = RowCount();
	if(!rows)
		return false;
	switch(key) {
	case K_UP:
		SelectRow(max(0, selected_row - 1));
		return true;
	case K_DOWN:
		SelectRow(min(rows - 1, selected_row + 1));
		return true;
	case K_PAGEUP:
		SelectRow(max(0, selected_row - max(1, GetSize().cy / max(1, line_height))));
		return true;
	case K_PAGEDOWN:
		SelectRow(min(rows - 1, selected_row + max(1, GetSize().cy / max(1, line_height))));
		return true;
	case K_HOME:
		SelectRow(0);
		return true;
	case K_END:
		SelectRow(max(0, rows - 1));
		return true;
	case K_SPACE:
		SetTimelineMode(!timeline_mode);
		return true;
	}
	return false;
}

void TimingView::MouseWheel(Point, int zdelta, dword)
{
	if(zdelta == 0)
		return;
	int step = max(1, abs(zdelta) / 120);
	if(zdelta > 0)
		SetTopRow(top_row - step);
	else
		SetTopRow(top_row + step);
}

void TimingView::LeftDown(Point p, dword)
{
	int row = RowAt(p);
	if(row >= 0)
		SelectRow(row);
}

void TimingView::GotFocus()
{
	Refresh();
}

void TimingView::LostFocus()
{
	Refresh();
}

} // namespace Upp
