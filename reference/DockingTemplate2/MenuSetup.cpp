#include "MainWindow.h"

void MainWindow::MainMenu(Bar& bar)
{
	bar.Add("App",     THISBACK(MenuApp));
	bar.Add("Edit",    THISBACK(MenuEdit));
	bar.Add("View",    THISBACK(MenuView));
	bar.Add("Windows", THISBACK(MenuWindows));
	bar.Add("Help",    THISBACK(MenuHelp));
}

void MainWindow::MenuApp(Bar& bar)
{
	bar.Add("New",     [=] { PromptOK("New not implemented"); });
	bar.Add("Open",    [=] { PromptOK("Open not implemented"); });
	bar.Add("Save",    [=] { PromptOK("Save not implemented"); });
	bar.Add("Save As", [=] { PromptOK("Save As not implemented"); });
	bar.Separator();
	bar.Add("Preferences", [=] { PromptOK("Preferences not implemented"); });
	bar.Separator();
	bar.Add("Exit", [=] { Break(); });
}

void MainWindow::MenuEdit(Bar& bar)
{
	// Mirror active-tab toolbar commands via the same Bar& builder functions
	int tab = main_tabs_.Get();
	if(tab == 0)      ToolBarTab1(bar);
	else if(tab == 1) ToolBarTab2(bar);
	else              ToolBarTabDebug(bar);
}

void MainWindow::MenuView(Bar& bar)
{
	bar.Add("Reset to Default Layout", [=] { OnResetDockLayout(); });
	bar.Separator();
	bar.Add("Manage Layouts…", [=] { DockManager(); });
	bar.Add("Save Layout As…", [=] { OnSaveLayoutAs(); });
	bar.Add("Load Layout…",    [=] { OnLoadLayout(); });
	bar.Separator();
	bar.Add("Show Dock A", [=] { SetDockVisible(dock_a_, true); });
	bar.Add("Show Dock B", [=] { SetDockVisible(dock_b_, true); });
	bar.Add("Show Dock C", [=] { SetDockVisible(dock_c_, true); });
}

void MainWindow::MenuWindows(Bar& bar)
{
	bar.Sub("Dock Windows", [=](Bar& sub) { DockWindowMenu(sub); });
	bar.Separator();
	bar.Sub("Modes", THISBACK(MenuModes));
}

void MainWindow::MenuModes(Bar& bar)
{
	bool is_edit = (mode_.GetMode() == ModeManager::MODE_EDIT);
	bar.Add(!is_edit, "View Mode", [=] { SetViewMode(); });
	bar.Add(is_edit,  "Edit Mode", [=] { SetEditMode(); });
}

void MainWindow::MenuHelp(Bar& bar)
{
	bar.Add("About", [=] {
		PromptOK("DockingTemplate2\n\n"
		         "Reusable U++ docking GUI template.\n"
		         "Demonstrates per-tab toolbar, full MenuBar,\n"
		         "edit modes, cursor linking, AppRegistry persistence,\n"
		         "and the nide layout-init pattern.");
	});
}
