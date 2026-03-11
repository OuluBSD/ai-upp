#include "ScriptIDE.h"

namespace Upp {

FilesPane::FilesPane()
{
	Title("Files");
	Icon(CtrlImg::Dir());
	
	Add(location_bar.TopPos(0, 24).HSizePos());
	Add(pane_toolbar.TopPos(24, 24).HSizePos());
	Add(tree.VSizePos(48, 0).HSizePos());
	
	location_bar.Set([=](Bar& bar) { LayoutLocationBar(bar); });
	pane_toolbar.Set([=](Bar& bar) { LayoutPaneToolbar(bar); });
	
	tree.WhenLeftDouble = [=] { OnOpen(); };
	tree.WhenOpen = [=](int id) { Populate(id); };
	
	path_field.WhenAction = [=] { SetRoot(~path_field); };
}

void FilesPane::LayoutLocationBar(Bar& bar)
{
	bar.Add(CtrlImg::plus(), [=] { WhenPathManager(); }).Help("PYTHONPATH manager button");
	path_field.SetRect(0, 0, 300, 20);
	bar.Add(path_field);
	bar.Add(CtrlImg::open(), [=] { WhenBrowse(); }).Help("Browse directory");
	bar.Add(CtrlImg::undo(), [=] { WhenParent(); }).Help("Parent directory");
}

void FilesPane::LayoutPaneToolbar(Bar& bar)
{
	bar.Add(CtrlImg::left_arrow(), [=] { Todo("Previous"); }).Help("Previous");
	bar.Add(CtrlImg::right_arrow(), [=] { Todo("Next"); }).Help("Next");
	bar.Add(CtrlImg::undo(), [=] { WhenParent(); }).Help("Parent");
	bar.Gap(2000); // Align right
	bar.Add("Filter", [=] { Todo("Filter filenames"); }).Help("Filter filenames");
	bar.Sub("Pane menu", CtrlImg::plus(), [=](Bar& b) { LayoutPaneMenu(b); });
}

void FilesPane::LayoutPaneMenu(Bar& bar)
{
	bar.Add("Show hidden files", [=] { ShowHidden(!show_hidden); }).Check(show_hidden);
	bar.Add("Edit filter settings...", [=] { Todo("Edit filter settings..."); });
	bar.Separator();
	bar.Add("Size", [=] { Todo("Size"); });
	bar.Add("Type", [=] { Todo("Type"); });
	bar.Add("Date modified", [=] { Todo("Date modified"); }).Check(true);
	bar.Separator();
	bar.Add("Single click to open", [=] { Todo("Single click to open"); }).Check(false);
	bar.Separator();
	bar.Add("Move", [=] { Todo("Move"); });
	bar.Add("Undock", [=] { Todo("Undock"); });
	bar.Add("Close", [=] { Todo("Close"); });
}

void FilesPane::SetRoot(const String& path)
{
	root_path = path;
	path_field <<= path;
	Refresh();
}

void FilesPane::Refresh()
{
	tree.Clear();
	if(root_path.IsEmpty()) return;

	String root_label = GetFileName(root_path);
	if(root_label.IsEmpty()) root_label = root_path; // Fallback for root directories

	tree.SetRoot(CtrlImg::Dir(), root_label);
	tree.Set(0, root_path);
	Populate(0);
}

void FilesPane::Populate(int id)
{
	if(tree.GetChildCount(id)) return;
	
	String path = tree.Get(id);
	FindFile ff(AppendFileName(path, "*"));
	while(ff) {
		String name = ff.GetName();
		if(!show_hidden && name.GetCount() > 0 && name[0] == '.') {
			ff.Next();
			continue;
		}
		
		if(ff.IsFile()) {
			if(GetFileExt(ff.GetName()) == ".py" || GetFileExt(ff.GetName()) == ".gamestate" || GetFileExt(ff.GetName()) == ".form")
				tree.Add(id, CtrlImg::File(), ff.GetPath(), ff.GetName());
		}
		else if(ff.IsFolder()) {
			if(ff.GetName() != "." && ff.GetName() != "..") {
				tree.Add(id, CtrlImg::Dir(), ff.GetPath(), ff.GetName(), true);
			}
		}
		ff.Next();
	}
}

void FilesPane::OnOpen()
{
	int id = tree.GetCursor();
	if(id >= 0) {
		String path = tree.Get(id);
		if(FileExists(path)) {
			WhenOpen(path);
		}
	}
}

}
