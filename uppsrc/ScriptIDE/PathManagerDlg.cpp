#include "ScriptIDE.h"

namespace Upp {

PathManagerDlg::PathManagerDlg()
{
	CtrlLayoutOK(*this, "PYTHONPATH Manager");
	
	list.AddColumn("Path");
	
	add.WhenAction = [=] { OnAdd(); };
	remove.WhenAction = [=] { OnRemove(); };
}

void PathManagerDlg::Set(const PathManager& pm)
{
	list.Clear();
	for(const String& p : pm.GetPaths())
		list.Add(p);
}

void PathManagerDlg::Get(PathManager& pm)
{
	pm.Clear();
	for(int i = 0; i < list.GetCount(); i++)
		pm.AddPath(list.Get(i, 0));
}

void PathManagerDlg::OnAdd()
{
	FileSel fs;
	if(fs.ExecuteSelectDir("Select Directory")) {
		list.Add(fs.Get());
	}
}

void PathManagerDlg::OnRemove()
{
	list.DoRemove();
}

}
