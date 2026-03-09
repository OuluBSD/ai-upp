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
}

void FilesPane::LayoutLocationBar(Bar& bar)
{
	bar.Add(CtrlImg::plus(), WhenPathManager).Help("PYTHONPATH manager");
	bar.Add("Path", [=] { /* TODO: Path dropdown */ }).Help("Active directory");
	bar.Add(CtrlImg::open(), WhenBrowse).Help("Browse directory");
	bar.Add(CtrlImg::undo(), WhenParent).Help("Parent directory");
}

void FilesPane::LayoutPaneToolbar(Bar& bar)
{
	bar.Add(CtrlImg::left_arrow(), [=] { /* TODO */ }).Help("Previous");
	bar.Add(CtrlImg::right_arrow(), [=] { /* TODO */ }).Help("Next");
	bar.Add(CtrlImg::undo(), WhenParent).Help("Parent");
	bar.Gap(2000); // Align right
	bar.Add("Filter", [=] { /* TODO */ });
	bar.Sub("Options", CtrlImg::plus(), [=](Bar& b) { LayoutPaneMenu(b); });
}

void FilesPane::LayoutPaneMenu(Bar& bar)
{
	bar.Add("Show hidden files", [=] {}).Check(false);
	bar.Add("Edit filter settings...", [=] {});
	bar.Separator();
	bar.Add("Size", [=] {});
	bar.Add("Type", [=] {});
	bar.Add("Date modified", [=] {}).Check(true);
	bar.Separator();
	bar.Add("Single click to open", [=] {}).Check(false);
}

void FilesPane::SetRoot(const String& path)
{
	root_path = path;
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
		if(ff.IsFile()) {
			if(GetFileExt(ff.GetName()) == ".py")
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
