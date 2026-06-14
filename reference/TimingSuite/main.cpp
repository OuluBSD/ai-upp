#include <CtrlLib/CtrlLib.h>

using namespace Upp;

namespace {

void Burn(int ms, const char *label)
{
	TimingScope timing(label);
	Sleep(ms);
}

void RunScreen(const char *name, int passes)
{
	TimingContextScope context(name);
	for(int i = 0; i < passes; ++i) {
		TimingScope frame("TimingSuite::Frame");
		Burn(1, "TimingSuite::Layout");
		Burn(2, "TimingSuite::Paint");
	}
}

void RunPipeline()
{
	for(int i = 0; i < 8; ++i) {
		String context;
		context << "pipeline/" << i;
		TimingContextScope timing_context(context);
		Burn(1, "TimingSuite::Pipeline::Fetch");
		Burn(1 + (i & 1), "TimingSuite::Pipeline::Transform");
	}
}

void PrepareTiming()
{
	TimingManager& tm = TimingManager::Global();
	tm.Clear();
	tm.Activate(true);
	tm.SetCaptureCallstack(false);
	tm.SetKeepTimeline(true);
}

void RunScenario()
{
	// Multiple contexts make the record list and timeline list easy to compare.

	RunScreen("main-menu", 3);
	RunScreen("gameplay", 2);
	RunScreen("highscore", 1);
	RunPipeline();
}

bool WantsHeadless()
{
	const Vector<String>& args = CommandLine();
	return FindIndex(args, "--headless") >= 0 || FindIndex(args, "-H") >= 0;
}

struct TimingSuiteWindow : TopWindow {
	typedef TimingSuiteWindow CLASSNAME;

	TimingWidget timing;
	Button       rerun;
	Label        note;

	TimingSuiteWindow()
	{
		Title("Timing Suite Reference");
		Sizeable().Zoomable();

		Add(timing);
		Add(rerun);
		Add(note);

		note.SetLabel("GUI path: inspect the embedded timing widget. Run with --headless for the stdout dump.");
		rerun.SetLabel("Re-run sample");
		rerun <<= THISBACK(RunAgain);

		timing.SetTimelineMode(true);
		RunScenario();
		timing.RefreshData();
	}

	void RunAgain()
	{
		RunScenario();
		timing.RefreshData();
	}

	void Layout() override
	{
		Size sz = GetSize();
		int pad = 6;
		int top = 28;
		note.SetRect(pad, pad, max(0, sz.cx - 140), 20);
		rerun.SetRect(max(0, sz.cx - 132 - pad), pad, 132, 20);
		timing.SetRect(0, top, sz.cx, max(0, sz.cy - top));
	}
};

void RunHeadless()
{
	RunScenario();
	Cout() << "TimingSuite headless dump:\n";
	Cout() << TimingManager::Global().Dump();
}

} // namespace

GUI_APP_MAIN
{
	PrepareTiming();

	if(WantsHeadless()) {
		RunHeadless();
	}
	else {
		TimingSuiteWindow().Run();
	}
}
