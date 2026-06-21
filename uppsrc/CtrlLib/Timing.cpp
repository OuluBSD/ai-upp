#include "CtrlLib.h"

namespace Upp {

TimingWidget::TimingWidget()
{
	Add(context_lbl);
	Add(context);
	Add(thread_lbl);
	Add(thread);
	Add(timeline);
	Add(intervals);
	Add(active);
	Add(capture);
	Add(keep_timeline);
	Add(scopes);
	Add(contexts);
	Add(locks);
	Add(memory);
	Add(markers);
	Add(refresh);
	Add(clear);
	Add(view);
	Add(details);

	context_lbl.SetText("Context");
	thread_lbl.SetText("Thread (0=all)");
	thread.NullText("0 = all, hex thread id");
	timeline.SetLabel("Timeline");
	intervals.SetLabel("Intervals");
	active.SetLabel("Active");
	capture.SetLabel("Callstack");
	keep_timeline.SetLabel("Keep timeline");
	scopes.SetLabel("Scopes");
	contexts.SetLabel("Contexts");
	locks.SetLabel("Locks");
	memory.SetLabel("Memory");
	markers.SetLabel("Markers");
	refresh.SetLabel("Refresh");
	clear.SetLabel("Clear");

	scopes = true;
	contexts = true;
	locks = true;
	memory = true;
	markers = true;

	details.SetFont(Courier());
	details.SetReadOnly();
	details.NoEofLine();
	details.NoUpDownLeave();
	details.NoBackground();
	details.ProcessTab(false);
	details.ProcessEnter(false);

	view.WhenSelect = THISBACK(OnViewSelect);
	context.WhenAction = THISBACK(ApplyFilters);
	thread.WhenAction = THISBACK(ApplyFilters);
	timeline.WhenAction = THISBACK(ApplyFilters);
	intervals.WhenAction = THISBACK(ApplyFilters);
	active.WhenAction = THISBACK(ApplyFilters);
	capture.WhenAction = THISBACK(ApplyFilters);
	keep_timeline.WhenAction = THISBACK(ApplyFilters);
	scopes.WhenAction = THISBACK(ApplyFilters);
	contexts.WhenAction = THISBACK(ApplyFilters);
	locks.WhenAction = THISBACK(ApplyFilters);
	memory.WhenAction = THISBACK(ApplyFilters);
	markers.WhenAction = THISBACK(ApplyFilters);
	refresh <<= THISBACK(RefreshData);
	clear <<= THISBACK(ClearData);

	SyncFromManager();
	ApplyFilters();
}

void TimingWidget::SyncFromManager()
{
	TimingManager& tm = TimingManager::Global();
	active = tm.IsActive();
	capture = tm.IsCaptureCallstack();
	keep_timeline = tm.IsKeepTimeline();
	locks = tm.IsLockEvents();
	memory = tm.IsMemoryEvents();
}

bool TimingWidget::GetThreadValue(uint64& id) const
{
	String s = TrimBoth(thread.GetText().ToString());
	if(s.IsEmpty() || s == "0") {
		id = 0;
		return true;
	}

	const char *p = ~s;
	if(p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
		p += 2;

	const char *end = p;
	uint64 value = stou64(p, (void *)&end, 16);
	while(*end && (byte)*end <= ' ')
		++end;
	if(end == p || *end)
		return false;

	id = value;
	return true;
}

void TimingWidget::SetTimelineMode(bool b)
{
	timeline = b;
	if(b)
		intervals = false;
	ApplyFilters();
}

void TimingWidget::SetContextIntervalMode(bool b)
{
	intervals = b;
	if(b)
		timeline = false;
	ApplyFilters();
}

void TimingWidget::SetContextFilter(const String& s)
{
	context.SetText(s);
	ApplyFilters();
}

String TimingWidget::GetContextFilter() const
{
	return context.GetText().ToString();
}

void TimingWidget::SetThreadFilter(uint64 id)
{
	thread_filter = id;
	thread.SetText(id ? Format64Hex(id) : String());
	ApplyFilters();
}

uint64 TimingWidget::GetThreadFilter() const
{
	return thread_filter;
}

void TimingWidget::SetActive(bool b)
{
	active = b;
	ApplyFilters();
}

void TimingWidget::SetCaptureCallstack(bool b)
{
	capture = b;
	ApplyFilters();
}

void TimingWidget::SetKeepTimeline(bool b)
{
	keep_timeline = b;
	ApplyFilters();
}

void TimingWidget::SetLockEvents(bool b)
{
	locks = b;
	ApplyFilters();
}

void TimingWidget::SetMemoryEvents(bool b)
{
	memory = b;
	ApplyFilters();
}

uint64 TimingWidget::BuildCategoryMask() const
{
	uint64 mask = 0;
	if(scopes.Get())
		mask |= ((uint64)1 << TIMING_EVENT_SCOPE);
	if(contexts.Get()) {
		mask |= ((uint64)1 << TIMING_EVENT_CONTEXT);
		mask |= ((uint64)1 << TIMING_EVENT_METADATA_UPDATE);
	}
	if(locks.Get()) {
		mask |= ((uint64)1 << TIMING_EVENT_MUTEX_WAIT);
		mask |= ((uint64)1 << TIMING_EVENT_MUTEX_HOLD);
		mask |= ((uint64)1 << TIMING_EVENT_SEMAPHORE_WAIT);
		mask |= ((uint64)1 << TIMING_EVENT_SPIN_WAIT);
	}
	if(memory.Get()) {
		mask |= ((uint64)1 << TIMING_EVENT_MEMORY_ALLOC);
		mask |= ((uint64)1 << TIMING_EVENT_MEMORY_FREE);
	}
	if(markers.Get())
		mask |= ((uint64)1 << TIMING_EVENT_MARKER);
	return mask;
}

void TimingWidget::ApplyFilters()
{
	TimingManager& tm = TimingManager::Global();
	tm.Activate(active.Get());
	tm.SetCaptureCallstack(capture.Get());
	tm.SetKeepTimeline(keep_timeline.Get());
	tm.SetLockEvents(locks.Get());
	tm.SetMemoryEvents(memory.Get());

	uint64 thread_id = thread_filter;
	if(GetThreadValue(thread_id)) {
		thread_filter = thread_id;
		thread.Error(false);
	}
	else
		thread.Error(true);

	view.SetTimelineMode(timeline.Get());
	view.SetContextIntervalMode(intervals.Get());
	view.SetContextFilter(context.GetText().ToString());
	view.SetThreadFilter(thread_filter);
	view.SetCategoryMask(BuildCategoryMask());
	view.RefreshData();
	UpdateDetails();
	Refresh();
}

void TimingWidget::RefreshData()
{
	view.RefreshData();
	UpdateDetails();
	Refresh();
}

void TimingWidget::ClearData()
{
	TimingManager::Global().Clear();
	RefreshData();
}

void TimingWidget::OnViewSelect(int)
{
	UpdateDetails();
}

void TimingWidget::UpdateDetails()
{
	String text = view.GetSelectedText();
	if(text.IsEmpty())
		text = "Select a row to inspect its details.";
	String stats = FormatStorageStats();
	if(!stats.IsEmpty())
		text << "\n\n" << stats;
	details.Set(text);
}

String TimingWidget::FormatStorageStats() const
{
#ifdef flagTIMING
	TimingManager& tm = TimingManager::Global();
	TimingStorageStats stats = tm.GetStorageStats();
	String text;
	text << "Storage retained=" << stats.event_count << "/" << stats.event_capacity
	     << " sequence=" << stats.sequence
	     << " overwritten=" << stats.overwritten
	     << " dropped=" << stats.dropped
	     << " active=" << tm.IsActive()
	     << " locks=" << tm.IsLockEvents()
	     << " memory=" << tm.IsMemoryEvents();
	return text;
#else
	return String();
#endif
}

void TimingWidget::Layout()
{
	Size sz = GetSize();
	int pad = 4;
	int top = 54;
	int details_h = max(120, sz.cy / 4);
	int filter_y = 4;
	int h = 20;

	context_lbl.SetRect(pad, filter_y, 56, h);
	context.SetRect(pad + 60, filter_y, max(120, sz.cx / 3), h);

	int thread_x = context.GetRect().right + pad;
	thread_lbl.SetRect(thread_x, filter_y, 88, h);
	thread.SetRect(thread_x + 92, filter_y, 110, h);

	int right = sz.cx - pad;
	int btn_w = 80;
	clear.SetRect(right - btn_w, filter_y, btn_w, h);
	refresh.SetRect(right - btn_w * 2 - pad, filter_y, btn_w, h);
	keep_timeline.SetRect(right - btn_w * 2 - pad * 2 - 95, filter_y, 90, h);
	capture.SetRect(right - btn_w * 2 - pad * 2 - 185, filter_y, 82, h);
	active.SetRect(right - btn_w * 2 - pad * 2 - 265, filter_y, 70, h);
	timeline.SetRect(right - btn_w * 2 - pad * 2 - 335, filter_y, 75, h);
	intervals.SetRect(right - btn_w * 2 - pad * 2 - 420, filter_y, 82, h);

	int category_y = filter_y + h + pad;
	int x = pad;
	scopes.SetRect(x, category_y, 72, h); x += 76;
	contexts.SetRect(x, category_y, 82, h); x += 86;
	locks.SetRect(x, category_y, 64, h); x += 68;
	memory.SetRect(x, category_y, 76, h); x += 80;
	markers.SetRect(x, category_y, 76, h);

	int view_top = top;
	int view_h = max(0, sz.cy - view_top - details_h - pad);
	view.SetRect(0, view_top, sz.cx, view_h);
	details.SetRect(0, view_top + view_h + pad, sz.cx, details_h);
}

} // namespace Upp
