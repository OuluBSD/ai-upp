#include "MainWindow.h"

void MainWindow::ToolBarTab1(Bar& bar)
{
	bool edit = (mode_.GetMode() == ModeManager::MODE_EDIT);
	bar.Add(edit, "Action A", CtrlImg::open(),  [=] { Log("Tab1: Action A"); });
	bar.Add(edit, "Action B", CtrlImg::save(),  [=] { Log("Tab1: Action B"); });
	bar.Separator();
	bar.Add("Refresh", CtrlImg::undo(), [=] { Log("Tab1: Refresh"); });
}

void MainWindow::ToolBarTab2(Bar& bar)
{
	bool edit = (mode_.GetMode() == ModeManager::MODE_EDIT);
	bar.Add(edit, "Process", CtrlImg::forward(), [=] { Log("Tab2: Process"); });
	bar.Separator();
	bar.Add("Reset", CtrlImg::undo(), [=] { Log("Tab2: Reset"); });
}

void MainWindow::ToolBarTabDebug(Bar& bar)
{
	bar.Add("Clear Log", CtrlImg::remove(), [=] { debug_tab_.Clear(); });
}
