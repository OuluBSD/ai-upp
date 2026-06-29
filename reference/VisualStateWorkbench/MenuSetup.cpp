#include "MainWindow.h"

void MainWindow::MainMenu(Bar& bar)
{
	bar.Add("File",    THISBACK(MenuFile));
	bar.Add("View",    THISBACK(MenuView));
	bar.Add("Windows", THISBACK(MenuWindows));
}

void MainWindow::MenuFile(Bar& bar)
{
	bar.Add("Load Sample Session", [=] { LoadSampleSession(); });
	bar.Separator();
	bar.Add("Run Pipeline",        [=] { OnRunPipeline(); });
	bar.Add("Clear Pipeline Cache",[=] { OnClearCache(); });
	bar.Separator();
	bar.Add("Exit", [=] { Close(); });
}

void MainWindow::MenuView(Bar& bar)
{
	bar.Add("Reset Dock Layout", [=] { OnResetDockLayout(); });
}

void MainWindow::MenuWindows(Bar& bar)
{
	bar.Add("Dock Manager", [=] { DockManager(); });
}
