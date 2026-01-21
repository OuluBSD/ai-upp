#include "AIPlanner.h"

AIPlanner::AIPlanner() {
	Title("Maestro AI Planner");
	SetRect(0, 0, 1200, 800);
	Sizeable().Zoomable();
	
	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	
	Add(tabs.SizePos());
	
	// Tracks Tab
	tabs.Add(track_split.SizePos(), "Tracks");
	track_split.Horz(track_tree, track_detail);
	track_split.SetPos(3000);
	
	// Runbooks Tab
	tabs.Add(runbook_split.SizePos(), "Runbooks");
	runbook_split.Horz(runbook_list, runbook_detail);
	runbook_split.SetPos(3000);
	runbook_list.AddColumn("ID");
	runbook_list.AddColumn("Title");
	
	// Workflows Tab
	tabs.Add(workflow_split.SizePos(), "Workflows");
	workflow_split.Horz(workflow_list, workflow_detail);
	workflow_split.SetPos(3000);
	workflow_list.AddColumn("ID");
	workflow_list.AddColumn("Title");
	
track_tree.WhenSel = THISBACK(OnTrackSelect);
	runbook_list.WhenCursor = THISBACK(OnRunbookSelect);
	workflow_list.WhenCursor = THISBACK(OnWorkflowSelect);
	
	config.Load();
	
	const Vector<String>& cmdline = CommandLine();
	bool test_mode = false;
	for(const auto& arg : cmdline) {
		if(arg == "--test") test_mode = true;
	}
	
	if(config.recent_dirs.GetCount() > 0)
		current_root = config.recent_dirs[0];
	else
		current_root = "/home/sblo/Dev/Maestro";
	
	if(test_mode) {
		LoadData();
		Cout() << "=== PLANNER TEST DUMP ===\n";
		Cout() << "Current Root: " << current_root << "\n";
		Cout() << "Tracks: " << tracks.GetCount() << "\n";
		Cout() << "Runbooks: " << runbooks.GetCount() << "\n";
		Cout() << "WorkGraphs: " << workgraphs.GetCount() << "\n";
		Cout() << "=== END DUMP ===\n";
		Cout().Flush();
		Exit(0);
	}
	
	PostCallback(THISBACK(LoadData));
}

void AIPlanner::MainMenu(Bar& bar) {
	bar.Sub("File", THISBACK(FileMenu));
}

void AIPlanner::FileMenu(Bar& bar) {
	bar.Add("Select Root...", THISBACK(OnSelectRoot));
	bar.Add("Reload", THISBACK(LoadData));
	bar.Separator();
	bar.Add("Exit", THISBACK(Close));
}

void AIPlanner::OnSelectRoot() {
	String root = SelectDirectory();
	if(!root.IsEmpty()) {
		current_root = root;
		config.AddDir(current_root);
		LoadData();
	}
}

void AIPlanner::LoadData() {
	Title("Maestro AI Planner - " + current_root);
	
	PlanParser pp;
	pp.LoadMaestroTracks(current_root);
	pp.LoadRunbooks(current_root);
	pp.LoadWorkGraphs(current_root);
	
tracks = pick(pp.tracks);
	runbooks = pick(pp.runbooks);
	workgraphs = pick(pp.workgraphs);
	
	// Update Tracks Tree
	track_tree.Clear();
	int root_id = track_tree.Add(0, CtrlImg::Dir(), String("root"), "Tracks");
	for(int i = 0; i < tracks.GetCount(); i++) {
		const auto& track = tracks[i];
		int tid = track_tree.Add(root_id, CtrlImg::Dir(), i, track.name);
		for(int j = 0; j < track.phases.GetCount(); j++) {
			const auto& phase = track.phases[j];
			int pid = track_tree.Add(tid, CtrlImg::Dir(), (i << 16) | (j + 1), phase.name);
			for(int k = 0; k < phase.tasks.GetCount(); k++) {
				const auto& task = phase.tasks[k];
				track_tree.Add(pid, CtrlImg::File(), (i << 20) | ((j + 1) << 10) | (k + 1), task.name);
			}
		}
	}
	track_tree.OpenDeep(root_id);
	
	// Update Runbooks List
	runbook_list.Clear();
	for(int i = 0; i < runbooks.GetCount(); i++) {
		runbook_list.Add(runbooks[i].id, runbooks[i].title);
	}
	
	// Update Workflows List
	workflow_list.Clear();
	for(int i = 0; i < workgraphs.GetCount(); i++) {
		workflow_list.Add(workgraphs[i].id, workgraphs[i].title);
	}
}

void AIPlanner::OnTrackSelect() {
	int id = track_tree.GetCursor();
	if(id < 0) return;
	
	Value key = track_tree.Get(id);
	if (!key.Is<int>()) {
		track_detail.SetQTF("");
		return;
	}
	
	int val = (int)key;
	String qtf;
	
	if (val < 1024) { // Track
		const auto& t = tracks[val];
		qtf << "[*@3 " << DeQtf(t.name) << "]" << "&";
		qtf << "[* Status:] " << DeQtf(t.status) << "&";
		qtf << "[* Completion:] " << t.completion << "%&";
	}
	else if (val < (1 << 20)) { // Phase
		int ti = val >> 16;
		int pi = (val & 0xFFFF) - 1;
		if (ti >= 0 && ti < tracks.GetCount() && pi >= 0 && pi < tracks[ti].phases.GetCount()) {
			const auto& p = tracks[ti].phases[pi];
			qtf << "[*@3 " << DeQtf(p.name) << "]" << "&";
			qtf << "[* Status:] " << DeQtf(p.status) << "&";
			qtf << "[* Completion:] " << p.completion << "%&";
		}
	}
	else { // Task
		int ti = val >> 20;
		int pi = ((val >> 10) & 0x3FF) - 1;
		int tai = (val & 0x3FF) - 1;
		if (ti >= 0 && ti < tracks.GetCount() && 
		    pi >= 0 && pi < tracks[ti].phases.GetCount() &&
		    tai >= 0 && tai < tracks[ti].phases[pi].tasks.GetCount()) {
			const auto& t = tracks[ti].phases[pi].tasks[tai];
			qtf << "[*@3 " << DeQtf(t.name) << "]" << "&";
			qtf << "[* Status:] " << StatusToString(t.status) << "&";
			qtf << "[* Priority:] " << DeQtf(t.priority) << "&";
			qtf << "[* Description:]&" << DeQtf(t.description) << "&";
		}
	}
	
track_detail.SetQTF(qtf);
}

void AIPlanner::OnRunbookSelect() {
	if(!runbook_list.IsCursor()) return;
	const auto& rb = runbooks[runbook_list.GetCursor()];
	
	String qtf;
	qtf << "[*@3 " << DeQtf(rb.title) << "]" << "&";
	qtf << "[* ID:] " << DeQtf(rb.id) << "&";
	qtf << "[* Goal:]&" << DeQtf(rb.goal) << "&";
	
	qtf << "[* Steps:]&";
	for(const auto& s : rb.steps) {
		qtf << "  " << s.n << ". [" << DeQtf(s.actor) << "] " << DeQtf(s.action) << "&";
		if(!s.command.IsEmpty())
			qtf << "     [* Command:] [C " << DeQtf(s.command) << "]" << "&";
		if(!s.expected.IsEmpty())
			qtf << "     [* Expected:] " << DeQtf(s.expected) << "&";
	}
	
	runbook_detail.SetQTF(qtf);
}

void AIPlanner::OnWorkflowSelect() {
	if(!workflow_list.IsCursor()) return;
	const auto& wg = workgraphs[workflow_list.GetCursor()];
	
	String qtf;
	qtf << "[*@3 " << DeQtf(wg.title) << "]" << "&";
	qtf << "[* ID:] " << DeQtf(wg.id) << "&";
	qtf << "[* Goal:]&" << DeQtf(wg.goal) << "&";
	
	qtf << "[* Domain:] " << DeQtf(wg.domain) << " [* Profile:] " << DeQtf(wg.profile) << "&";
	
	qtf << "[* Track Summary:]&";
	qtf << "  [* Name:] " << DeQtf(wg.track.name) << "&";
	qtf << "  [* Goal:] " << DeQtf(wg.track.goal) << "&";
	
	qtf << "[* Execution Graph:]&";
	for(const auto& p : wg.phases) {
		qtf << "  [* Phase:] " << DeQtf(p.name) << "&";
		for(const auto& tk : p.tasks) {
			qtf << "    - [* " << DeQtf(tk.title) << "*]&";
			qtf << "      [* Intent:] " << DeQtf(tk.intent) << "&";
			if(tk.definition_of_done.GetCount() > 0) {
				qtf << "      [* DoD:]&";
				for(const auto& dod : tk.definition_of_done)
					qtf << "        - " << DeQtf(dod.kind) << ": " << DeQtf(dod.cmd) << DeQtf(dod.path) << "&";
			}
		}
	}
	
	workflow_detail.SetQTF(qtf);
}

GUI_APP_MAIN {
	AIPlanner().Run();
}
