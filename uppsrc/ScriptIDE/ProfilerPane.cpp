#include "ScriptIDE.h"

namespace Upp {

ProfilerPane::ProfilerPane()
{
	Title("Profiler");
	Icon(Icons::Profiler());
	
	Add(toolbar.TopPos(0, 36).HSizePos());
	Add(list.VSizePos(36, 0).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
}

void ProfilerPane::LayoutToolbar(Bar& bar)
{
	bar.Add(Icons::Undo(), [=] { Todo("Collapse"); }).Tip("Collapse one level up").Help("Collapse level up");
	bar.Add(Icons::Plus(), [=] { Todo("Expand"); }).Tip("Expand one level down").Help("Expand level down");
	bar.Separator();
	bar.Add(Icons::Run(), [=] { Todo("Filter local time"); }).Tip("Show items with one large local time").Help("Hot items");
	bar.Add(Icons::Stop(), [=] { Todo("Filter external"); }).Tip("Hide calls to external libraries").Help("Hide external");
	bar.Add(Icons::Undo(), [=] { Todo("Show callers"); }).Tip("Show callers/callees").Help("Callers/callees");
	bar.Add(Icons::Search(), [=] { Todo("Search"); }).Tip("Search").Help("Search");
	bar.Separator();
	bar.Add(Icons::Stop(), [=] { Todo("Stop profiling"); }).Tip("Stop profiling").Help("Stop profiling");
	bar.ToolGapRight();
	bar.Add(Icons::Save(), [=] { Todo("Save data"); }).Tip("Save profiling data").Help("Save profiling data");
	bar.Add(Icons::OpenFile(), [=] { Todo("Load data"); }).Tip("Load profiling data comparison").Help("Load comparison");
	bar.Add(Icons::ClearConsole(), [=] { Todo("Clear comparison"); }).Tip("Clear comparison").Enable(false);
	bar.Sub("Options", Icons::Settings(), [=](Bar& b) { LayoutPaneMenu(b); }).Tip("Pane menu");
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
