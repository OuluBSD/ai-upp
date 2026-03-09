#include "ScriptIDE.h"

namespace Upp {

FindInFilesPane::FindInFilesPane()
{
	Title("Find in Files");
	Icon(CtrlImg::plus()); // Use plus as placeholder for search
	
	Add(search_pattern.TopPos(2, 20).LeftPos(2, 200));
	Add(search_btn.SetLabel("Search").TopPos(2, 20).LeftPos(205, 80));
	Add(results.VSizePos(24, 0).HSizePos());
	
	search_btn.WhenAction = [=] { OnSearch(); };
	search_pattern.WhenEnter = [=] { OnSearch(); };
	
	results.AddIndex("PATH");
	results.AddColumn("File", 30).Sorting();
	results.AddColumn("Line", 10).Sorting();
	results.AddColumn("Text", 60).Sorting();
	results.AllSorting();
	results.EvenRowColor();
	results.SetLineCy(20);
	
	results.WhenLeftDouble = [=] { OnResultOpen(); };
}

void FindInFilesPane::OnSearch()
{
	String pattern = search_pattern.GetData();
	if(pattern.IsEmpty() || root_path.IsEmpty()) return;
	
	results.Clear();
	
	Vector<String> files;
	auto Scan = [&](const String& dir, auto& self) -> void {
		FindFile ff(AppendFileName(dir, "*"));
		while(ff) {
			if(ff.IsFile()) {
				if(GetFileExt(ff.GetName()) == ".py")
					files.Add(ff.GetPath());
			}
			else if(ff.IsFolder()) {
				if(ff.GetName() != "." && ff.GetName() != "..")
					self(ff.GetPath(), self);
			}
			ff.Next();
		}
	};
	
	Scan(root_path, Scan);
	
	for(const String& path : files) {
		String content = Upp::LoadFile(path);
		Vector<String> lines = Split(content, '\n', false);
		for(int i = 0; i < lines.GetCount(); i++) {
			int q = lines[i].Find(pattern);
			if(q >= 0) {
				results.Add(path, GetFileName(path), i + 1, TrimBoth(lines[i]));
			}
		}
	}
}

void FindInFilesPane::OnResultOpen()
{
	if(!results.IsCursor()) return;
	String path = results.Get(results.GetCursor(), "PATH");
	int line = results.Get(results.GetCursor(), "Line");
	WhenOpenMatch(path, line);
}

}
