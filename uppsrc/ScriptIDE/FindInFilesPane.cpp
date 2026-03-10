#include "ScriptIDE.h"

namespace Upp {

FindInFilesPane::FindInFilesPane()
{
	Title("Find");
	Icon(CtrlImg::plus());
	
	Add(toolbar.TopPos(0, 24).HSizePos());
	Add(results.VSizePos(24, 0).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
	
	results.AddColumn("File", 30).Sorting();
	results.AddColumn("Line", 10).Sorting();
	results.AddColumn("Text", 60).Sorting();
	
	results.WhenLeftDouble = [=] { OnResultOpen(); };
}

void FindInFilesPane::LayoutToolbar(Bar& bar)
{
	search_pattern.SetRect(0, 0, 200, 20);
	bar.Add(search_pattern);
	bar.Add("Search", CtrlImg::plus(), [=] { OnSearch(); }).Help("Search");
	bar.Add(regex_toggle);
	bar.Add(case_toggle);
	bar.Add("Advanced search", [=] { Todo("Advanced search"); }).Help("Advanced search toggle");
	bar.Separator();
	bar.Sub("Options", CtrlImg::plus(), [=](Bar& b) { LayoutPaneMenu(b); });
}

void FindInFilesPane::LayoutPaneMenu(Bar& bar)
{
	bar.Add("Set maximum number of results...", [=] { Todo("Max results"); });
	bar.Separator();
	bar.Add("Move", [=] { Todo("Move pane"); });
	bar.Add("Undock", [=] { Todo("Undock pane"); });
	bar.Add("Close", [=] { Todo("Close pane"); });
}

void FindInFilesPane::OnSearch()
{
	String pattern = ~search_pattern;
	if(pattern.IsEmpty()) return;
	
	results.Clear();
	if(root_path.IsEmpty()) return;
	
	// Very simple recursive search
	Vector<String> files;
	FindFile ff(AppendFileName(root_path, "*"));
	while(ff) {
		if(ff.IsFile() && GetFileExt(ff.GetName()) == ".py")
			files.Add(ff.GetPath());
		ff.Next();
	}
	
	for(const String& f : files) {
		String content = LoadFile(f);
		Vector<String> lines = Split(content, '\n', false);
		for(int i = 0; i < lines.GetCount(); i++) {
			if(lines[i].Find(pattern) >= 0) {
				results.Add(GetFileName(f), i + 1, TrimBoth(lines[i]), f);
			}
		}
	}
}

void FindInFilesPane::OnResultOpen()
{
	if(results.IsCursor()) {
		String path = results.Get(results.GetCursor(), 3);
		int line = results.Get(results.GetCursor(), 1);
		WhenOpenMatch(path, line);
	}
}

}
