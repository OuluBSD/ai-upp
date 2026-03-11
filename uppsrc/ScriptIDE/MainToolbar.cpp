#include "ScriptIDE.h"

namespace Upp {

void PythonIDE::MainToolbar(Bar& bar)
{
	bar.Add(Icons::NewFile(), [=] { OnNewFile(); }).Help("New File");
	bar.Add(Icons::OpenFile(), [=] { OnOpenFile(); }).Help("Open File");
	bar.Add(Icons::Save(), [=] { OnSaveFile(); }).Help("Save File");
	bar.Add(Icons::SaveAll(), [=] { OnSaveAll(); }).Help("Save All Files");
	bar.Separator();
	
	bar.Add(Icons::Plus(), [=] { Todo("Create new cell"); }).Help("Create new cell at the current line");
	bar.Separator();
	
	bar.Add(Icons::Run(), [=] { OnRun(); }).Help("Run file (F5)");
	bar.Add(Icons::Run(), [=] { OnRunCell(); }).Help("Run cell (Ctrl+Return)");
	bar.Add(Icons::Run(), [=] { OnRunCellAndAdvance(); }).Help("Run cell and advance (Shift+Return)");
	bar.Add(Icons::Run(), [=] { OnRunSelection(); }).Help("Run current line or selection (F9)");
	bar.Separator();
	
	bar.Add(Icons::Debug(), [=] { OnDebug(); }).Help("Debug file (Ctrl+F5)");
	bar.Add(Icons::Debug(), [=] { OnDebugCell(); }).Help("Debug cell");
	bar.Add(Icons::Debug(), [=] { OnDebugSelection(); }).Help("Debug the current file or selection");
	bar.Separator();
	
	bar.Add(Icons::Profiler(), [=] { Todo("Profile file"); }).Help("Profile file (F10)");
	bar.Add(Icons::Profiler(), [=] { Todo("Profile cell"); }).Help("Profile cell (Alt+F10)");
	bar.Add(Icons::Profiler(), [=] { Todo("Profile selection"); }).Help("Profile current line or selection");
	bar.Separator();
	
	bar.Add(Icons::Maximize(), [=] { OnMaximizePane(); }).Help("Maximize current pane (Ctrl+Alt+Shift+M)");
	bar.Add(Icons::Settings(), [=] { OnSettings(); }).Help("Preferences");
	bar.Add(Icons::Search(), [=] { OnPathManager(); }).Help("PYTHONPATH manager");
	bar.Separator();
	
	bar.Add("Recent Projects", [=] { Todo("Recent projects dropdown"); });
	bar.Add("Working Directory", [=] { Todo("Working directory dropdown"); });
	bar.Add(Icons::OpenFile(), [=] { Todo("Browse working directory"); }).Help("Browse working directory");
	bar.Add(Icons::Undo(), [=] { Todo("Change to parent directory"); }).Help("Change to parent directory");
}

}
