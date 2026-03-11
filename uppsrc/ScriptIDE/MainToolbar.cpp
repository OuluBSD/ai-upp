#include "ScriptIDE.h"

namespace Upp {

void PythonIDE::MainToolbar(Bar& bar)
{
	bar.Add(ScriptIDEImg::IconNewFile(), [=] { OnNewFile(); }).Help("New File");
	bar.Add(ScriptIDEImg::IconOpenFile(), [=] { OnOpenFile(); }).Help("Open File");
	bar.Add(ScriptIDEImg::IconSave(), [=] { OnSaveFile(); }).Help("Save File");
	bar.Add(ScriptIDEImg::IconSaveAs(), [=] { OnSaveAll(); }).Help("Save All Files");
	bar.Separator();
	
	bar.Add(CtrlImg::plus(), [=] { Todo("Create new cell"); }).Help("Create new cell at the current line");
	bar.Separator();
	
	bar.Add(ScriptIDEImg::IconRun(), [=] { OnRun(); }).Help("Run file (F5)");
	bar.Add(ScriptIDEImg::IconRun(), [=] { OnRunCell(); }).Help("Run cell (Ctrl+Return)");
	bar.Add(ScriptIDEImg::IconRun(), [=] { OnRunCellAndAdvance(); }).Help("Run cell and advance (Shift+Return)");
	bar.Add(ScriptIDEImg::IconRun(), [=] { OnRunSelection(); }).Help("Run current line or selection (F9)");
	bar.Separator();
	
	bar.Add(ScriptIDEImg::IconDebug(), [=] { OnDebug(); }).Help("Debug file (Ctrl+F5)");
	bar.Add(ScriptIDEImg::IconDebug(), [=] { OnDebugCell(); }).Help("Debug cell");
	bar.Add(ScriptIDEImg::IconDebug(), [=] { OnDebugSelection(); }).Help("Debug the current file or selection");
	bar.Separator();
	
	bar.Add(CtrlImg::plus(), [=] { Todo("Profile file"); }).Help("Profile file (F10)");
	bar.Add(CtrlImg::plus(), [=] { Todo("Profile cell"); }).Help("Profile cell (Alt+F10)");
	bar.Add(CtrlImg::plus(), [=] { Todo("Profile selection"); }).Help("Profile current line or selection");
	bar.Separator();
	
	bar.Add(CtrlImg::plus(), [=] { OnMaximizePane(); }).Help("Maximize current pane (Ctrl+Alt+Shift+M)");
	bar.Add(CtrlImg::plus(), [=] { OnSettings(); }).Help("Preferences");
	bar.Add(CtrlImg::plus(), [=] { OnPathManager(); }).Help("PYTHONPATH manager");
	bar.Separator();
	
	bar.Add("Recent Projects", [=] { Todo("Recent projects dropdown"); });
	bar.Add("Working Directory", [=] { Todo("Working directory dropdown"); });
	bar.Add(ScriptIDEImg::IconOpenFile(), [=] { Todo("Browse working directory"); }).Help("Browse working directory");
	bar.Add(CtrlImg::undo(), [=] { Todo("Change to parent directory"); }).Help("Change to parent directory");
}

}
