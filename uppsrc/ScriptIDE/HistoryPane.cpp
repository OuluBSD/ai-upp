#include "ScriptIDE.h"

namespace Upp {

HistoryPane::HistoryPane()
{
	Title("History");
	Icon(CtrlImg::Dir());
	
	Add(editor.SizePos());
	editor.SetReadOnly();
	editor.Highlight("python");
	
	UpdateEditor();
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
