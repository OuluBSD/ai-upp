#include "MainWindow.h"

void MainWindow::MainMenu(Bar& bar)
{
	bar.Add("File",    THISBACK(MenuFile));
	bar.Add("View",    THISBACK(MenuView));
	bar.Add("Windows", THISBACK(MenuWindows));
}

void MainWindow::MenuFile(Bar& bar)
{
	bar.Add("Open/Import Session\xE2\x80\xA6",   [=] { OnOpenImportSession(); });
	bar.Separator();
	bar.Add("Run Pipeline",                      [=] { OnRunPipeline(); });
	bar.Add("Compare with Ground Truth\xE2\x80\xA6", [=] { OnCompareGroundTruth(); });
	bar.Add("Clear Pipeline Cache",              [=] { OnClearCache(); });
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
