#include "ScriptIDE.h"

namespace Upp {

void PythonIDE::MainToolbarGroup(Bar& bar, int group)
{
	bool doc_running = false;
	bool doc_paused = false;
	bool doc_can_pause = false;
	if(active_file >= 0 && active_file < open_files.GetCount()) {
		if(IDocumentHost* h = open_files[active_file].editor) {
			doc_running = h->IsRunning();
			doc_paused = h->IsPaused();
			doc_can_pause = h->CanPause();
		}
	}
	bool stop_enabled = doc_running || vm.IsRunning();
	bool run_enabled = !stop_enabled;
	bool pause_enabled = doc_can_pause && doc_running;

	if(group == 0) { // Standard group
		bar.Add(Icons::NewFile(), [this] { OnNewFile(); }).Tip("New File (Ctrl+N)").Help("Create a new Python file");
		bar.Add(Icons::OpenFile(), [this] { OnOpenFile(); }).Tip("Open File (Ctrl+O)").Help("Open an existing Python file");
		bar.Add(Icons::Save(), [this] { OnSaveFile(); }).Tip("Save File (Ctrl+S)").Help("Save current file");
		bar.Add(Icons::SaveAll(), [this] { OnSaveAll(); }).Tip("Save All Files (Ctrl+Alt+S)").Help("Save all open files");
		bar.Separator();
		
		bar.Add(Icons::Plus(), [this] { Todo("Create new cell"); }).Tip("New Cell").Help("Create a new code cell at current line");
		bar.Separator();
		
		bar.Add(Icons::Run(), [this] { OnRun(); }).Tip("Run (Ctrl+F5)").Help("Run current file without stopping at breakpoints").Enable(run_enabled);
		bar.Add(Icons::RunCell(), [this] { OnRunCell(); }).Tip("Run Cell (Ctrl+Return)").Help("Run current code cell").Enable(run_enabled);
		bar.Add(Icons::RunCellAdvance(), [this] { OnRunCellAndAdvance(); }).Tip("Run Cell and Advance (Shift+Return)").Help("Run current cell and move to next").Enable(run_enabled);
		bar.Add(Icons::RunSelection(), [this] { OnRunSelection(); }).Tip("Run Selection (F9)").Help("Run current selection or line").Enable(run_enabled);
		bar.Separator();
		
		bar.Add(Icons::Debug(), [this] { OnDebug(); }).Tip("Debug (F5)").Help("Start debugging current file with breakpoints enabled").Enable(run_enabled);
		bar.Add(Icons::DebugCell(), [this] { OnDebugCell(); }).Tip("Debug Cell").Help("Debug current code cell").Enable(run_enabled);
		bar.Add(Icons::DebugSelection(), [this] { OnDebugSelection(); }).Tip("Debug Selection").Help("Debug current selection or line").Enable(run_enabled);
		bar.Separator();
		
		bar.Add(Icons::Profile(), [this] { OnProfile(); }).Tip("Profile (F10)").Help("Profile current file").Enable(active_file >= 0 && run_enabled);
		bar.Add(Icons::ProfileCell(), [this] { Todo("Profile cell"); }).Tip("Profile Cell (Alt+F10)").Help("Profile current code cell").Enable(run_enabled);
		bar.Add(Icons::ProfileSelection(), [this] { Todo("Profile current line or selection"); }).Tip("Profile Selection").Help("Profile current selection or line").Enable(run_enabled);
		bar.Separator();
		
		bar.Add(doc_paused ? Icons::Run() : Icons::StepOver(), [this] { OnPause(); }).Tip(doc_paused ? "Resume" : "Pause").Help(doc_paused ? "Resume current run" : "Pause current run").Enable(pause_enabled);
		bar.Add(Icons::Stop(), [this] { OnStop(); }).Tip("Stop").Help("Stop current run").Enable(stop_enabled);
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
