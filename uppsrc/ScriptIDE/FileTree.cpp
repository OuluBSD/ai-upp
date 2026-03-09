#include "ScriptIDE.h"

namespace Upp {

FileTree::FileTree()
{
	Add(tree.SizePos());
	tree.WhenLeftDouble = [=] { OnOpen(); };
	tree.WhenOpen = [=](int id) { Populate(id); };
}

void FileTree::SetRoot(const String& path)
{
	root_path = path;
	Refresh();
}

void FileTree::Refresh()
{
	tree.Clear();
	if(root_path.IsEmpty()) return;

	String root_label = GetFileName(root_path);
	if(root_label.IsEmpty()) root_label = root_path; // Fallback for root directories

	tree.SetRoot(TreeCtrl::Node(CtrlImg::Dir(), root_label).Set(root_path).CanOpen());
	Populate(0);
}

void FileTree::Populate(int id)
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
				tree.Add(id, TreeCtrl::Node(CtrlImg::Dir(), ff.GetName()).Set(ff.GetPath()).CanOpen());
			}
		}
		ff.Next();
	}
}

void FileTree::OnOpen()
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
