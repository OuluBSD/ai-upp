#include "ScriptIDE.h"

namespace Upp {

ProfilerPane::ProfilerPane()
{
	Title("Profiler");
	Icon(Icons::Profiler());
	
	Add(toolbar.TopPos(0, 24).HSizePos());
	Add(list.VSizePos(24, 0).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
}

void ProfilerPane::LayoutToolbar(Bar& bar)
{
	bar.Add("Collapse one level up", Icons::Undo(), [=] { Todo("Collapse"); }).Help("Collapse level up");
	bar.Add("Expand one level down", Icons::Plus(), [=] { Todo("Expand"); }).Help("Expand level down");
	bar.Separator();
	bar.Add("Show items with one large local time", [=] { Todo("Filter local time"); }).Help("Hot items");
	bar.Add("Hide calls to external libraries", [=] { Todo("Filter external"); }).Help("Hide external");
	bar.Add("Show callers/callees", [=] { Todo("Show callers"); }).Help("Callers/callees");
	bar.Add("Search", Icons::Search(), [=] { Todo("Search"); }).Help("Search");
	bar.Separator();
	bar.Add("Stop profiling", Icons::Stop(), [=] { Todo("Stop profiling"); }).Help("Stop profiling");
	bar.Gap(2000);
	bar.Add("Save profiling data", Icons::Save(), [=] { Todo("Save data"); }).Help("Save profiling data");
	bar.Add("Load profiling data comparison", Icons::OpenFile(), [=] { Todo("Load data"); }).Help("Load comparison");
	bar.Add("Clear comparison", [=] { Todo("Clear comparison"); }).Enable(false);
	bar.Sub("Options", Icons::Settings(), [=](Bar& b) { LayoutPaneMenu(b); });
}

void ProfilerPane::LayoutPaneMenu(Bar& bar)
{
	bar.Add("Move", [=] { Todo("Move pane"); });
	bar.Add("Undock", [=] { Todo("Undock pane"); });
	bar.Add("Close", [=] { Todo("Close pane"); });
}

void ProfilerPane::SetData(const VectorMap<String, Value>& data) { Todo("Set profile data"); }
void ProfilerPane::Clear() { list.Clear(); }

}
