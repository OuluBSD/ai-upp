#include "Shell.h"

NAMESPACE_UPP

namespace Widget {

DraftPad::DraftPad() {
	Add(edit.SizePos());
}

void DraftPad::Data() {
	
}

void DraftPad::ToolMenu(Bar& bar) {
	bar.Add("Clear all", [this]{edit.Clear();});
}

Timer::Timer() {
	CtrlLayout(*this);
	hours.SetData(0);
	mins.SetData(1);
	secs.SetData(0);
	start <<= THISBACK(ToggleStart);
	hours.SetFocus();
}

void Timer::ToggleStart() {
	if (running) {
		start.SetLabel(t_("Start"));
		running = false;
		tc.Kill();
	}
	else {
		int h = hours.GetData();
		int m = mins.GetData();
		int s = secs.GetData();
		h = min(100, max(0, h));
		m = min(60, max(0, m));
		s = min(60, max(0, s));
		int total = s + 60 * (m + 60 * h);
		Time now = GetSysTime();
		ready = now + total;
		start.SetLabel(t_("Stop"));
		running = true;
		tc.Set(-1000, THISBACK(Check));
		Check();
	}
}

void Timer::Check() {
	if (!running) return;
	Time now = GetSysTime();
	int64 remaining = max((int64)0, ready.Get() - now.Get());
	int64 remaining_s = remaining % 60;
	remaining /= 60;
	int64 remaining_m = remaining % 60;
	remaining /= 60;
	int64 remaining_h = remaining;
	String s;
	s << "Remaining: ";
	if (remaining_h)
		s << remaining_h << " hours";
	if (remaining_h || remaining_m)
		s << " " << remaining_m << " minutes";
	if (remaining_h || remaining_m || remaining_s)
		s << " " << remaining_s << " seconds";
	this->remaining.SetLabel(s);
	if (now >= ready) {
		tc.Kill();
		running = false;
		start.SetLabel(t_("Start"));
		this->remaining.SetLabel(t_("Timer is not running"));
		OnReady();
	}
}

void Timer::OnReady() {
	PromptOK("[h(255.42.0)/@(255.42.0)$(28.212.255) $$1,1#E0842503E169BCB89B760F238004336D:Ring][*_@(141.42.0) $$0,0#00000000000000000000000000000000:Default][{_} [s1;= `*ring`* `*ring`*&][s0;= Timer is ready!&][s1;= `*ring`*]]");
}

void Timer::Data() {
	
}
void Timer::ToolMenu(Bar& bar) {
	bar.Add(running ? t_("Stop") : t_("Start"), THISBACK(ToggleStart));
}

}

END_UPP_NAMESPACE
