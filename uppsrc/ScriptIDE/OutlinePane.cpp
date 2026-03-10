#include "ScriptIDE.h"

namespace Upp {

OutlinePane::OutlinePane()
{
	Title("Outline");
	Icon(CtrlImg::Dir());
	
	Add(toolbar.TopPos(0, 24).HSizePos());
	Add(tree.VSizePos(24, 0).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
	
	tree.WhenLeftDouble = [=] { OnSelect(); };
}

void OutlinePane::LayoutToolbar(Bar& bar)
{
	bar.Add("Search symbols", [=] { Todo("Search symbols"); }).Help("Search");
	bar.Gap(2000);
	bar.Sub("Options", CtrlImg::plus(), [=](Bar& b) { LayoutPaneMenu(b); });
}

void OutlinePane::LayoutPaneMenu(Bar& bar)
{
	bar.Add("Sort alphabetically", [=] { Todo("Sort symbols"); }).Check(false);
	bar.Separator();
	bar.Add("Move", [=] { Todo("Move pane"); });
	bar.Add("Undock", [=] { Todo("Undock pane"); });
	bar.Add("Close", [=] { Todo("Close pane"); });
}

void OutlinePane::UpdateOutline(const String& code)
{
	tree.Clear();
	tree.SetRoot(CtrlImg::Dir(), "Symbols");
	int root = 0;
	
	Vector<String> lines = Split(code, '\n', false);
	for(int i = 0; i < lines.GetCount(); i++) {
		String line = TrimBoth(lines[i]);
		if(line.StartsWith("def ") || line.StartsWith("class ")) {
			tree.Add(root, CtrlImg::File(), i + 1, line);
		}
	}
	tree.OpenDeep(root);
}

void OutlinePane::Clear()
{
	tree.Clear();
}

void OutlinePane::OnSelect()
{
	int id = tree.GetCursor();
	if(id >= 0) {
		Value v = tree.Get(id);
		if(!v.IsVoid())
			WhenSelectLine((int)v);
	}
}

}
