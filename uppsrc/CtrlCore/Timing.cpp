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

void TimingView::SetContextIntervalMode(bool b)
{
	if(context_interval_mode == b)
		return;
	context_interval_mode = b;
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

void TimingView::SetCategoryMask(uint64 mask)
{
	if(category_mask == mask)
		return;
	category_mask = mask;
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

TimingStorageStats TimingView::GetStorageStats() const
{
#ifdef flagTIMING
	return TimingManager::Global().GetStorageStats();
#else
	return TimingStorageStats();
#endif
}

bool TimingView::MatchCategory(byte category) const
{
	if(category >= 64)
		return false;
	return category_mask & ((uint64)1 << category);
}

bool TimingView::MatchKey(const TimingKey& key) const
{
	if(!MatchCategory(key.category))
		return false;
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
	context_items.Clear();
	stats = TimingStorageStats();
#ifdef flagTIMING
	stats = GetStorageStats();
	Vector<TimingRecord> all_records = GetRecords();
	for(const TimingRecord& r : all_records)
		if(MatchKey(r.key))
			records.Add(r);
	Vector<TimingSample> all_timeline = GetTimeline();
	for(const TimingSample& s : all_timeline)
		if(MatchKey(s.key))
			timeline.Add(s);
#endif
	BuildContextItems();
	Normalize();
}

void TimingView::BuildContextItems()
{
	Vector<int> active;
	for(const TimingSample& s : timeline) {
		bool is_context = s.key.category == TIMING_EVENT_CONTEXT;
		bool is_metadata = s.key.category == TIMING_EVENT_METADATA_UPDATE ||
		                   (is_context && s.value == 0);
		if(!is_context && !is_metadata)
			continue;

		if(is_metadata) {
			ContextItem& item = context_items.Add();
			item.point = s;
			item.has_point = true;
			item.metadata = true;
			continue;
		}

		if(s.value > 0) {
			ContextItem& item = context_items.Add();
			item.begin = s;
			item.has_begin = true;
			Vector<int> used;
			for(int active_i : active) {
				const ContextItem& a = context_items[active_i];
				if(a.has_begin && a.begin.key.thread_id == s.key.thread_id)
					used.Add(a.lane);
			}
			int lane = 0;
			for(;;) {
				bool found = false;
				for(int used_lane : used)
					if(used_lane == lane) {
						found = true;
						break;
					}
				if(!found)
					break;
				lane++;
			}
			item.lane = lane;
			active.Add(context_items.GetCount() - 1);
		}
		else if(s.value < 0) {
			int found = -1;
			for(int i = active.GetCount() - 1; i >= 0; --i) {
				const ContextItem& item = context_items[active[i]];
				if(item.has_begin &&
				   item.begin.key.thread_id == s.key.thread_id &&
				   item.begin.key.label == s.key.label &&
				   item.begin.nesting == s.nesting) {
					found = i;
					break;
				}
			}
			if(found >= 0) {
				ContextItem& item = context_items[active[found]];
				item.end = s;
				item.has_end = true;
				active.Remove(found);
			}
			else {
				ContextItem& item = context_items.Add();
				item.end = s;
				item.has_end = true;
			}
		}
	}

	if(context_items.IsEmpty()) {
		for(const TimingRecord& r : records) {
			if(r.key.category != TIMING_EVENT_CONTEXT &&
			   r.key.category != TIMING_EVENT_METADATA_UPDATE)
				continue;
			ContextItem& item = context_items.Add();
			item.summary = r;
			item.has_summary = true;
			item.metadata = r.key.category == TIMING_EVENT_METADATA_UPDATE ||
			                (r.key.category == TIMING_EVENT_CONTEXT && r.total_value == 0);
		}
	}
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
	if(context_interval_mode)
		return context_items.GetCount();
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
	if(context_interval_mode)
		return ContextItemText(context_items[row], row);

	if(timeline_mode) {
		const TimingSample& s = timeline[row];
		String out;
		out << AsString(row) << "  "
		    << s.key.ToString()
		    << "  elapsed=" << AsString(s.elapsed)
		    << "  value=" << AsString(s.value)
		    << "  nest=" << AsString(s.nesting)
		    << "  seq=" << AsString(s.sequence);
		if(s.dropped_before)
			out << "  dropped_before=" << AsString(s.dropped_before);
		return out;
	}

	const TimingRecord& r = records[row];
	String out;
	out << AsString(row) << "  "
	    << r.key.ToString()
	    << "  count=" << AsString(r.count)
	    << "  total=" << AsString(r.total_time)
	    << "  value=" << AsString(r.total_value)
	    << "  min=" << AsString(r.min_time)
	    << "  max=" << AsString(r.max_time)
	    << "  nest=" << AsString(r.max_nesting);
	if(r.dropped_before)
		out << "  dropped_before=" << AsString(r.dropped_before);
	return out;
}

String TimingView::ContextItemText(const ContextItem& item, int row) const
{
	String out;
	out << AsString(row) << "  ";
	if(item.has_summary) {
		out << (item.metadata ? "metadata_summary" : "context_summary")
		    << "  " << item.summary.key.ToString()
		    << "  count=" << AsString(item.summary.count)
		    << "  value=" << AsString(item.summary.total_value)
		    << "  first=" << AsString(item.summary.first_sequence)
		    << "  last=" << AsString(item.summary.last_sequence);
		if(item.summary.dropped_before)
			out << "  dropped_before=" << AsString(item.summary.dropped_before);
		return out;
	}

	if(item.has_point) {
		const TimingSample& s = item.point;
		out << (s.key.category == TIMING_EVENT_METADATA_UPDATE ? "metadata_update" : "context_point")
		    << "  " << s.key.ToString()
		    << "  seq=" << AsString(s.sequence)
		    << "  value=" << AsString(s.value)
		    << "  nest=" << AsString(s.nesting);
		if(s.dropped_before)
			out << "  dropped_before=" << AsString(s.dropped_before);
		return out;
	}

	const TimingSample& key_sample = item.has_begin ? item.begin : item.end;
	out << "context_interval"
	    << "  state=" << (item.has_begin && item.has_end ? "closed" :
	                      item.has_begin ? "open_end" : "open_begin")
	    << "  lane=" << AsString(item.lane)
	    << "  " << key_sample.key.ToString();
	if(item.has_begin)
		out << "  begin_seq=" << AsString(item.begin.sequence)
		    << " begin_meta=" << (item.begin.key.context.IsEmpty() ? String("<none>") : item.begin.key.context);
	else
		out << "  begin_seq=<before retained>";
	if(item.has_end)
		out << "  end_seq=" << AsString(item.end.sequence)
		    << " end_meta=" << (item.end.key.context.IsEmpty() ? String("<none>") : item.end.key.context);
	else
		out << "  end_seq=<after retained>";
	if(item.has_begin && item.has_end)
		out << "  seq_span=" << AsString(item.end.sequence - item.begin.sequence);
	if(key_sample.dropped_before)
		out << "  dropped_before=" << AsString(key_sample.dropped_before);
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
	header << (context_interval_mode ? "[context intervals]" : timeline_mode ? "[timeline]" : "[summary]");
	header << "  records=" << AsString(records.GetCount());
	header << "  samples=" << AsString(timeline.GetCount());
	if(context_interval_mode)
		header << "  intervals=" << AsString(context_items.GetCount());
	header << "  retained=" << AsString(stats.event_count) << "/" << AsString(stats.event_capacity);
	if(stats.overwritten || stats.dropped)
		header << "  overwritten=" << AsString(stats.overwritten) << " dropped=" << AsString(stats.dropped);
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
		Color bg = RowColor(RowCategory(i), i);
		if(i == selected_row)
			bg = Blend(SColorHighlight(), SColorFace(), 210);
		w.DrawRect(0, y, sz.cx, line_height, bg);
		if(context_interval_mode)
			PaintContextItem(w, context_items[i], y, sz.cx);
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
		if(context_interval_mode)
			SetContextIntervalMode(false);
		else
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

Color TimingView::RowColor(byte category, int row) const
{
	Color base = (row & 1) ? Blend(SColorFace(), SColorShadow(), 242) : SColorFace();
	switch(category) {
	case TIMING_EVENT_MUTEX_WAIT:
	case TIMING_EVENT_MUTEX_HOLD:
	case TIMING_EVENT_SEMAPHORE_WAIT:
	case TIMING_EVENT_SPIN_WAIT:
		return Blend(base, LtRed(), 236);
	case TIMING_EVENT_MEMORY_ALLOC:
	case TIMING_EVENT_MEMORY_FREE:
		return Blend(base, LtGreen(), 236);
	case TIMING_EVENT_CONTEXT:
	case TIMING_EVENT_METADATA_UPDATE:
		return Blend(base, LtBlue(), 236);
	case TIMING_EVENT_MARKER:
		return Blend(base, Yellow(), 236);
	default:
		return base;
	}
}

byte TimingView::RowCategory(int row) const
{
	if(context_interval_mode) {
		const ContextItem& item = context_items[row];
		if(item.has_summary)
			return item.summary.key.category;
		if(item.has_point)
			return item.point.key.category;
		return TIMING_EVENT_CONTEXT;
	}
	return timeline_mode ? timeline[row].key.category : records[row].key.category;
}

void TimingView::PaintContextItem(Draw& w, const ContextItem& item, int y, int width) const
{
	int range_w = min(260, width / 3);
	if(range_w < 48)
		return;
	int range_x = width - range_w - 6;
	uint64 first = stats.retained_from_sequence;
	uint64 last = stats.retained_to_sequence;
	if(!first && !timeline.IsEmpty())
		first = timeline[0].sequence;
	if(!last)
		last = stats.sequence;
	if(last <= first)
		last = first + 1;

	if(item.has_point) {
		uint64 seq = minmax(item.point.sequence, first, last);
		int x = range_x + (int)((seq - first) * (uint64)range_w / (last - first));
		w.DrawRect(x, y + 3, 2, line_height - 6, LtBlue());
		return;
	}
	if(item.has_summary)
		return;

	uint64 begin = item.has_begin ? item.begin.sequence : first;
	uint64 end = item.has_end ? item.end.sequence : last;
	begin = minmax(begin, first, last);
	end = minmax(end, first, last);
	if(end < begin)
		Swap(begin, end);
	int x1 = range_x + (int)((begin - first) * (uint64)range_w / (last - first));
	int x2 = range_x + (int)((end - first) * (uint64)range_w / (last - first));
	x2 = max(x2, x1 + 2);
	int lane_y = y + 4 + min(item.lane, 4) * 2;
	w.DrawRect(x1, lane_y, x2 - x1, 3, LtBlue());
	if(!item.has_begin)
		w.DrawRect(range_x, lane_y, 2, 3, Blue());
	if(!item.has_end)
		w.DrawRect(range_x + range_w - 2, lane_y, 2, 3, Blue());
}

} // namespace Upp
