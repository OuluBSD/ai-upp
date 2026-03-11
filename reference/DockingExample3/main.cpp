#include <Docking/Docking.h>

using namespace Upp;

class DockingExample3 : public DockWindow {
public:
	typedef DockingExample3 CLASSNAME;

	DockingExample3();
	virtual void DockInit();
	virtual void Close();

private:
	ArrayCtrl arrayctrl1;
	ArrayCtrl arrayctrl2;
	TreeCtrl treectrl1;
	TreeCtrl treectrl2;
	Button button;

	void FillArray(ArrayCtrl& array);
	void FillTree(TreeCtrl& tree);
};

DockingExample3::DockingExample3()
{
	Title("DockingExample3 : Proportional Side Layout");
	Sizeable().MaximizeBox();

	Add(button.SetLabel("Manager").LeftPosZ(4, 100).TopPosZ(4, 23));
	button << [=] { DockManager(); };

	FillArray(arrayctrl1);
	FillTree(treectrl1);
	FillArray(arrayctrl2);
	FillTree(treectrl2);
}

void DockingExample3::DockInit()
{
	DockableCtrl& array1 = Dockable(arrayctrl1, "ArrayCtrl 1").SizeHint(Size(300, 200));
	DockableCtrl& tree1  = Dockable(treectrl1, "TreeCtrl 1").SizeHint(Size(300, 200));
	DockableCtrl& array2 = Dockable(arrayctrl2, "ArrayCtrl 2").SizeHint(Size(300, 200));
	DockableCtrl& tree2  = Dockable(treectrl2, "TreeCtrl 2").SizeHint(Size(300, 200));

	FileIn in(GetDataFile("docklayout3.dat"));
	bool badfile = !in.IsOpen();
	if (!badfile) {
		SerializeWindow(in);
		badfile = in.IsError();
	}

	if (badfile) {
		DockLeft(array1);
		DockLeft(tree1);
		DockTop(array2);
		DockRight(tree2);

		// Keep left dock width tied to window width (50%).
		SetFrameLayoutHalf(DOCK_LEFT);
	}
}

void DockingExample3::Close()
{
	FileOut out(GetDataFile("docklayout3.dat"));
	if (out.IsOpen())
		SerializeWindow(out);
	TopWindow::Close();
}

void DockingExample3::FillArray(ArrayCtrl& array)
{
	array.AddColumn("Number");
	array.AddColumn("Roman numbers");
	array.MultiSelect();
	for (int i = 0; i < 200; i++)
		array.Add(i, FormatIntRoman(i, true));
}

void DockingExample3::FillTree(TreeCtrl& tree)
{
	Vector<int> parent;
	parent.Add(0);
	tree.SetRoot(Image(), "The Tree");
	for (int i = 1; i < 10000; i++) {
		parent.Add(tree.Add(parent[rand() % parent.GetCount()], Image(),
		            FormatIntRoman(i, true)));
		if ((rand() & 3) == 0)
			tree.Open(parent.Top());
	}
	tree.Open(0);
}

GUI_APP_MAIN
{
	DockingExample3().Run();
}
