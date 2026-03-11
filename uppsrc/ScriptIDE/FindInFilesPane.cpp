#include "ScriptIDE.h"

namespace Upp {

FindInFilesPane::FindInFilesPane()
{
	Title("Find");
	Icon(TablerIcons::Search());
	
	Add(toolbar.TopPos(0, 24).HSizePos());
	Add(results.VSizePos(24, 0).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
	
	results.AddColumn("File", 30).Sorting();
	results.AddColumn("Line", 10).Sorting();
	results.AddColumn("Text", 60).Sorting();
	
	results.WhenLeftDouble = [=] { OnResultOpen(); };
	
	pattern_lbl.SetLabel("Pattern:");
	files_lbl.SetLabel("Files:");
	regex_toggle.SetLabel("Regex");
	case_toggle.SetLabel("Case");
	search_btn.SetLabel("Search");
	stop_btn.SetLabel("Stop");
	browse_btn.SetLabel("...");
	
	search_btn.WhenAction = [=] { OnSearch(); };
	stop_btn.WhenAction = [=] { Todo("Stop search"); };
	browse_btn.WhenAction = [=] { Todo("Browse for directory"); };
}

void FindInFilesPane::LayoutToolbar(Bar& bar)
{
	bar.Add(pattern_lbl, 60);
	search_pattern.SetRect(0, 0, 150, 20);
	bar.Add(search_pattern);
	
	bar.Add(regex_toggle);
	bar.Add(case_toggle);
	bar.Separator();
	
	bar.Add(files_lbl, 40);
	files_pattern.SetRect(0, 0, 100, 20);
	bar.Add(files_pattern);
	
	browse_btn.SetRect(0, 0, 30, 20);
	bar.Add(browse_btn);
	
	bar.Separator();
	bar.Add(search_btn);
	bar.Add(stop_btn);
	
	bar.Gap(2000);
	bar.Sub("Options", TablerIcons::Settings(), [=](Bar& b) { LayoutPaneMenu(b); });
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
