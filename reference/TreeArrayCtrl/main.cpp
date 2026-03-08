#include "CtrlLib/CtrlLib.h"

using namespace Upp;

struct App : TopWindow {
	Splitter      horz;
	TreeArrayCtrl tree1;
	TreeArrayCtrl tree2;
	StatusBar     info;

	typedef App CLASSNAME;

	void FillColumns(TreeArrayCtrl& tree) {
		tree.Header();
		tree.HeaderTab(0).SetText("Name");
		tree.AddColumn("Size", 80).Edit(Single<EditString>());
		tree.AddColumn("Type", 80).Edit(Single<EditString>());
		tree.AddColumn("Visible", 40).Ctrls<Option>();
		tree.MultiSelect();
	}

	void SetFileRow(TreeArrayCtrl& tree, int id, const FindFile& ff) {
		tree.SetRowValue(id, 1, ff.IsFolder() ? String() : AsString(ff.GetLength()));
		tree.SetRowValue(id, 2, ff.IsFolder() ? String("Dir") : String("File"));
		tree.SetRowValue(id, 3, true);
	}

	void OpenDir(int id) {
		String path = tree1.Get(id);
		int limit = 200;
		for(FindFile ff(AppendFileName(path, "*.*")); ff; ff.Next()) {
			String n = ff.GetName();
			if(n != "." && n != "..") {
				int q = tree1.Add(id,
					ff.IsFolder() ? CtrlImg::Dir() : CtrlImg::File(),
					AppendFileName(path, n), n, ff.IsFolder());
				SetFileRow(tree1, q, ff);
			}
			if(--limit <= 0)
				break;
		}
	}

	void LoadTree(int parent, const String& path, Progress& pi, int& limit)
	{
		pi.SetText(DeFormat(path));
		for(FindFile ff(AppendFileName(path, "*.*")); ff; ff.Next()) {
			if(pi.StepCanceled())
				return;
			String n = ff.GetName();
			if(n != "." && n != "..") {
				int q = tree2.Add(parent,
					ff.IsFolder() ? CtrlImg::Dir() : CtrlImg::File(),
					AppendFileName(path, n), n, ff.IsFolder());
				SetFileRow(tree2, q, ff);
				if(ff.IsFolder())
					LoadTree(q, AppendFileName(path, n), pi, limit);
			}
			if(--limit <= 0)
				break;
		}
	}

	App() {
		FillColumns(tree1);
		FillColumns(tree2);
		horz.Add(tree1);
		horz.Add(tree2);
		Add(horz.Horz().SizePos());

		tree1.WhenOpen = THISBACK(OpenDir);
		tree1.WhenClose = [=] (int id) { tree1.RemoveChildren(id); };

	#ifdef PLATFORM_WIN32
		String dir = String(GetExeFilePath()[0], 1) + ":\\";
	#else
		String dir = "/usr";
	#endif
		tree1.SetRoot(CtrlImg::Dir(), dir);
		tree2.SetRoot(CtrlImg::Dir(), dir);
		if (tree1.GetLineCount() > 0)
			tree1.SetCursor(tree1.GetItemAtLine(0));

		int limit = 400;
		Progress pi;
		pi.AlignText(ALIGN_LEFT);
		LoadTree(0, dir, pi, limit);
		tree2.NoRoot();
		if (tree2.GetLineCount() > 0)
			tree2.SetCursor(tree2.GetItemAtLine(0));
		Sizeable();

		tree1.WhenCursor = [=] { info = AsString(tree1.Get(tree1.GetCursor())); };
		tree1.AddFrame(info);
	}
};

GUI_APP_MAIN
{
	App().Run();
}
