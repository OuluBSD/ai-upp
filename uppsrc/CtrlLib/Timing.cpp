#include "CtrlLib.h"

namespace Upp {

TimingWidget::TimingWidget()
{
	Add(context_lbl);
	Add(context);
	Add(thread_lbl);
	Add(thread);
	Add(timeline);
	Add(active);
	Add(capture);
	Add(keep_timeline);
	Add(refresh);
	Add(clear);
	Add(view);
	Add(details);

	context_lbl.SetText("Context");
	thread_lbl.SetText("Thread (0=all)");
	thread.NullText("0 = all, hex thread id");
	timeline.SetLabel("Timeline");
	active.SetLabel("Active");
	capture.SetLabel("Callstack");
	keep_timeline.SetLabel("Keep timeline");
	refresh.SetLabel("Refresh");
	clear.SetLabel("Clear");

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
	active.WhenAction = THISBACK(ApplyFilters);
	capture.WhenAction = THISBACK(ApplyFilters);
	keep_timeline.WhenAction = THISBACK(ApplyFilters);
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

void TimingWidget::ApplyFilters()
{
	TimingManager& tm = TimingManager::Global();
	tm.Activate(active.Get());
	tm.SetCaptureCallstack(capture.Get());
	tm.SetKeepTimeline(keep_timeline.Get());

	uint64 thread_id = thread_filter;
	if(GetThreadValue(thread_id)) {
		thread_filter = thread_id;
		thread.Error(false);
	}
	else
		thread.Error(true);

	view.SetTimelineMode(timeline.Get());
	view.SetContextFilter(context.GetText().ToString());
	view.SetThreadFilter(thread_filter);
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
	details.Set(text);
}

void TimingWidget::Layout()
{
	Size sz = GetSize();
	int pad = 4;
	int top = 28;
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

	int view_top = top;
	int view_h = max(0, sz.cy - view_top - details_h - pad);
	view.SetRect(0, view_top, sz.cx, view_h);
	details.SetRect(0, view_top + view_h + pad, sz.cx, details_h);
}

} // namespace Upp
