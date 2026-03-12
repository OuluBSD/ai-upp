#include "ScriptIDE.h"

namespace Upp {

HistoryPane::HistoryPane()
{
	Title("History");
	Icon(Icons::History());
	
	Add(toolbar.TopPos(0, 36).HSizePos());
	Add(editor.VSizePos(36, 0).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
	
	editor.SetReadOnly();
	editor.Highlight("python");
	
	UpdateEditor();
}

void HistoryPane::LayoutToolbar(Bar& bar)
{
	bar.Add(Icons::ClearConsole(), [=] { Todo("Clear history"); }).Tip("Clear history").Help("Clear history");
	bar.ToolGapRight();
	bar.Sub("Options", Icons::Settings(), [=](Bar& b) { LayoutPaneMenu(b); }).Tip("Pane menu");
}

void HistoryPane::LayoutPaneMenu(Bar& bar)
{
	bar.Add("Wrap lines", [=] { wrap_lines = !wrap_lines; UpdateEditor(); }).Check(wrap_lines);
	bar.Add("Show line numbers", [=] { show_line_numbers = !show_line_numbers; UpdateEditor(); }).Check(show_line_numbers);
	bar.Separator();
	bar.Add("Move", [=] { Todo("Move pane"); });
	bar.Add("Undock", [=] { Todo("Undock pane"); });
	bar.Add("Close", [=] { Todo("Close pane"); });
}

void HistoryPane::UpdateEditor()
{
	editor.LineNumbers(show_line_numbers);
	editor.WordWrap(wrap_lines);
}

}
