#include "ScriptIDE.h"

namespace Upp {

FilesPane::FilesPane()
{
	Title("Files");
	Icon(CtrlImg::Dir());
	Add(tree.SizePos());
	tree.WhenLeftDouble = [=] { OnOpen(); };
	tree.WhenOpen = [=](int id) { Populate(id); };
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

	tree.SetRoot(TreeCtrl::Node(CtrlImg::Dir(), root_label).Set(root_path).CanOpen());
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
