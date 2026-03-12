#include "ScriptIDE.h"

namespace Upp {

void PythonIDE::MainToolbarGroup(Bar& bar, int group)
{
	if(group == 0) { // Standard group
		bar.Add(Icons::NewFile(), [this] { OnNewFile(); }).Tip("New File (Ctrl+N)").Help("Create a new Python file");
		bar.Add(Icons::OpenFile(), [this] { OnOpenFile(); }).Tip("Open File (Ctrl+O)").Help("Open an existing Python file");
		bar.Add(Icons::Save(), [this] { OnSaveFile(); }).Tip("Save File (Ctrl+S)").Help("Save current file");
		bar.Add(Icons::SaveAll(), [this] { OnSaveAll(); }).Tip("Save All Files (Ctrl+Alt+S)").Help("Save all open files");
		bar.Separator();
		
		bar.Add(Icons::Plus(), [this] { Todo("Create new cell"); }).Tip("New Cell").Help("Create a new code cell at current line");
		bar.Separator();
		
		bar.Add(Icons::Run(), [this] { OnRun(); }).Tip("Run (F5)").Help("Run current file or configuration");
		bar.Add(Icons::RunCell(), [this] { OnRunCell(); }).Tip("Run Cell (Ctrl+Return)").Help("Run current code cell");
		bar.Add(Icons::RunCellAdvance(), [this] { OnRunCellAndAdvance(); }).Tip("Run Cell and Advance (Shift+Return)").Help("Run current cell and move to next");
		bar.Add(Icons::RunSelection(), [this] { OnRunSelection(); }).Tip("Run Selection (F9)").Help("Run current selection or line");
		bar.Separator();
		
		bar.Add(Icons::Debug(), [this] { OnDebug(); }).Tip("Debug (Ctrl+F5)").Help("Start debugging current file");
		bar.Add(Icons::DebugCell(), [this] { OnDebugCell(); }).Tip("Debug Cell").Help("Debug current code cell");
		bar.Add(Icons::DebugSelection(), [this] { OnDebugSelection(); }).Tip("Debug Selection").Help("Debug current selection or line");
		bar.Separator();
		
		bar.Add(Icons::Profile(), [this] { Todo("Profile file"); }).Tip("Profile (F10)").Help("Profile current file");
		bar.Add(Icons::ProfileCell(), [this] { Todo("Profile cell"); }).Tip("Profile Cell (Alt+F10)").Help("Profile current code cell");
		bar.Add(Icons::ProfileSelection(), [this] { Todo("Profile current line or selection"); }).Tip("Profile Selection").Help("Profile current selection or line");
		bar.Separator();
		
		bar.Add(Icons::Maximize(), [this] { OnMaximizePane(); }).Tip("Maximize Pane (Ctrl+Alt+Shift+M)").Help("Maximize the currently active pane");
		bar.Add(Icons::Settings(), [this] { OnSettings(); }).Tip("Preferences").Help("Configure IDE settings");
		bar.Add(Icons::Search(), [this] { OnPathManager(); }).Tip("PYTHONPATH manager").Help("Manage Python search paths");
	}
	else if(group == 1) { // Project group
		bar.Add("Recent Projects", [this] { Todo("Recent projects dropdown"); }).Tip("Recent projects").Help("Select a recent project");
		bar.Add("Working Directory", [this] { Todo("Working directory dropdown"); }).Tip("Working directory").Help("Select working directory");
		bar.Add(Icons::OpenFile(), [this] { Todo("Browse working directory"); }).Tip("Browse working directory").Help("Choose a new working directory");
		bar.Add(Icons::Undo(), [this] { Todo("Change to parent directory"); }).Tip("Parent directory").Help("Move to parent directory");
	}
}

void PythonIDE::MainToolbar(Bar& bar)
{
	bool first = true;
	for(int id : tstate.left_order) {
		bar.ToolGroup(false, !tstate.hide_handles && !first);
		MainToolbarGroup(bar, id);
		first = false;
	}
	
	first = true;
	for(int id : tstate.right_order) {
		bar.ToolGroup(true, !tstate.hide_handles && !first);
		MainToolbarGroup(bar, id);
		first = false;
	}
}

}
