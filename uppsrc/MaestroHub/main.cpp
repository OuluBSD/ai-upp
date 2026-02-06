#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

#include "MaestroHub.h"

NAMESPACE_UPP

// Helper to generate context (moved from AIPlanner/main.cpp logic but adapted)
String GetPlanSummaryText(const Array<Track>& tracks, const String& current_track, const String& current_phase, const String& current_task) {
	return PlanSummarizer::GetPlanSummaryText(tracks, current_track, current_phase, current_task);
}

MaestroHubCockpit::MaestroHubCockpit() {
	Title("MaestroHub Cockpit - Professional Software Engineering Orchestrator");
	Icon(CtrlImg::Network());
	
	// Try to limit icon size
	toolbar.MaxIconSize(Size(20, 20));
	
	AddFrame(toolbar);

	toolbar.Set([=](Bar& bar) {
		bar.Add(CtrlImg::Dir(), THISBACK(SelectRoot)).Tip("Open Project Directory");
		bar.Separator();
		bar.Add(CtrlImg::save(), []{ PromptOK("Save All (Stub)"); }).Tip("Save All Changes");
		bar.Separator();
		bar.Add(CtrlImg::reporticon(), THISBACK(OnOpsRunner)).Tip("Run Operations Doctor");
		bar.Add(CtrlImg::question(), THISBACK(OnLogAnalyzer)).Tip("Scan Logs for Findings");
	});
	
	AddFrame(statusbar);
	statusbar.Set(0, "Ready.", 0);
	statusbar.Set(1, "Model: Gemini 1.5 Pro", 200);
	statusbar.Set(2, "Quota: 85% [====--]", 150);
	statusbar.Set(3, "Backend: OK", 100);

	fleet.Create();
	intelligence.Create();
	evidence.Create();
	playbook.Create();
	debug_workspace.Create();
	technology.Create();
	pipeline.Create();
	product.Create();
	maintenance.Create();
	issues.Create();
	work.Create();
	sessions.Create();
	audit_trail.Create();
	tutorial.Create();
	
	automation_output.SetQTF("[&@6 [* Automation System Active]]&[C1 This view displays logs and output from orchestrated transformation tasks.]");
	ai_trace.SetQTF("[&@2 [* AI Reasoning Trace]]&[C1 This view displays real-time insights into the AI's decision-making process.]");

	// Left Tabs: Workspace and Pipeline
	left_tabs.Add(technology->SizePos(), "Workspace"); 
	left_tabs.Add(pipeline->SizePos(), "Pipeline"); 
	
	// Center Tabs: Functional Hubs
	center_tabs.Add(fleet->SizePos(), "Fleet Dashboard");
	center_tabs.Add(intelligence->SizePos(), "Code Intelligence");
	center_tabs.Add(evidence->SizePos(), "Evidence Locker");
	center_tabs.Add(product->SizePos(), "Modeling & Logic"); 
	center_tabs.Add(playbook->SizePos(), "Strategy Playbooks");
	center_tabs.Add(debug_workspace->SizePos(), "Execution Console"); 
	center_tabs.Add(issues->SizePos(), "Issue Tracker");
	center_tabs.Add(maintenance->SizePos(), "Maintenance Hub");
	
	// Bottom Tabs: Output and Trace
	bottom_tabs.Add(automation_output.SizePos(), "Automation Output");
	bottom_tabs.Add(ai_trace.SizePos(), "AI Trace");
	bottom_tabs.Add(audit_trail->SizePos(), "System Events");
	bottom_tabs.Add(tutorial->SizePos(), "Interactive Guide");
	
	// Right Panel: Global AI Assistant
	assistant.Create();
	
	// Compose the layout
	center_split.Vert(center_tabs, bottom_tabs);
	center_split.SetPos(6500); 
	
	main_split.Horz();
	main_split << left_tabs << center_split << *assistant;
	main_split.SetPos(2000, 0);
	main_split.SetPos(8000, 1);
	
	Add(main_split.SizePos());
	
	// Use operator<< for CallbackN support with lambdas
	technology->WhenEnact << [=](String t, String p, String k) { OnEnact(t, p, k); };
	product->WhenEnactStep << [=](String t, int s, String i) { OnEnactStep(t, s, i); };
	
	assistant->chat.WhenEvent = [=](const MaestroEvent& e) { OnAssistantEvent(e); };
	
	config.Load();
	if(config.recent_dirs.GetCount() > 0)
		current_root = config.recent_dirs[0];
	else
		current_root = GetCurrentDirectory();
	
	PostCallback(THISBACK(LoadData));
}

MaestroHubCockpit::~MaestroHubCockpit() {}

void MaestroHubCockpit::MainMenu(Bar& bar) {
	bar.Sub("App", THISBACK(AppMenu));
	
	bar.Sub("Sessions", [=](Bar& b) {
		b.Add("List Sessions", [=] { center_tabs.Set(0); }); 
		b.Add("New Session...", THISBACK(OnNewSession));
		b.Separator();
		b.Add("Active Session HUD", [=] { center_tabs.Set(4); }); 
	});

	bar.Sub("Issues", [=](Bar& b) {
		b.Add("Browse Issues", [=] { center_tabs.Set(5); }); 
		b.Add("Triage Wizard...", THISBACK(OnTriageWizard));
		b.Separator();
		b.Add("Create Issue...", THISBACK(OnCreateIssue));
	});

	bar.Sub("Runbooks", [=](Bar& b) {
		b.Add("Manage Runbooks", [=] { center_tabs.Set(3); }); 
		b.Add("Visual Editor...", THISBACK(OnRunbookEditor));
		b.Separator();
		b.Add("Resolve Freeform...", [=] { PromptOK("Runbook Resolve Placeholder"); });
	});

	bar.Sub("Workflows", [=](Bar& b) {
		b.Add("Browse Workflows", [=] { center_tabs.Set(3); }); 
		b.Add("Visual Graph", [=] { center_tabs.Set(3); });
		b.Separator();
		b.Add("State Machine Editor...", THISBACK(OnStateEditor));
	});

	bar.Sub("Intelligence", [=](Bar& b) {
		b.Add("TU Browser", THISBACK(OnTUBrowser));
		b.Add("Log Analyzer", THISBACK(OnLogAnalyzer));
		b.Separator();
		b.Add("Dependency Graph", [=] { center_tabs.Set(1); }); 
	});

	bar.Sub("System", [=](Bar& b) {
		b.Add("Initialize Maestro...", THISBACK(OnInitMaestro));
		b.Add("Settings...", THISBACK(OnSettings));
		b.Separator();
		b.Add("Manage Playbooks...", [=] { PromptOK("Playbooks placeholder"); });
		b.Add("Collect Evidence...", [=] { PromptOK("Evidence placeholder"); });
		b.Add("Run Ops Doctor", THISBACK(OnOpsRunner));
	});
}

void MaestroHubCockpit::AppMenu(Bar& bar) {
	bar.Add("New Session...", THISBACK(OnNewSession));
	bar.Add("Project Init...", THISBACK(OnInitMaestro));
	bar.Separator();
	bar.Add("Configuration", THISBACK(OnSettings));
	bar.Separator();
	bar.Sub("Help", [=](Bar& bar) {
		bar.Add("Welcome Tutorial", [=] { WelcomeDialog dlg; dlg.Run(); });
		bar.Separator();
		bar.Add("About", [=] { PromptOK("MaestroHub Cockpit v1.0&[C1 (c) 2026 AI-UPP Team]"); });
	});
	bar.Separator();
	bar.Add("Exit", THISBACK(Close));
}

void MaestroHubCockpit::SelectRoot() {
	String root = SelectDirectory();
	if(!root.IsEmpty()) {
		current_root = root;
		config.AddDir(current_root);
		LoadData();
	}
}

void MaestroHubCockpit::LoadData() {
	SyncStatus();
	if(fleet) {
		fleet->LoadProjects(config.recent_dirs);
		fleet->UpdateQueue();
	}
	if(intelligence) intelligence->Load(current_root);
	if(evidence) evidence->Load(current_root);
	if(playbook) playbook->Load(current_root);
	if(technology) technology->Load(current_root);
	if(pipeline) pipeline->Load(current_root);
	if(product) product->Load(current_root);
	if(maintenance) maintenance->Load(current_root);
	if(issues) issues->Load(current_root);
	if(work) work->Load(current_root);
	if(sessions) sessions->Load(current_root);
	if(audit_trail) audit_trail->Load(current_root);
	
	ScanForUnblockedTasks();
}

void MaestroHubCockpit::ScanForUnblockedTasks() {
	if(!assistant || current_root.IsEmpty()) return;
	
	PlanParser pp;
	pp.LoadMaestroTracks(current_root);
	
	Index<String> done_tasks;
	for(const auto& t : pp.tracks)
		for(const auto& p : t.phases)
			for(const auto& tk : p.tasks)
				if(tk.status == STATUS_DONE)
					done_tasks.Add(tk.id);
					
	for(const auto& t : pp.tracks) {
		for(const auto& p : t.phases) {
			for(const auto& tk : p.tasks) {
				if(tk.status == STATUS_TODO) {
					bool unblocked = true;
					for(const String& dep : tk.depends_on) {
						if(done_tasks.Find(dep) < 0) {
							unblocked = false;
							break;
						}
					}
					if(unblocked) {
						assistant->chat.SuggestEnactment(t.id, p.id, tk.id);
						return; 
					}
				}
			}
		}
	}
}

void MaestroHubCockpit::OnToggleAssistant() {
	if(!assistant) return;
	assistant->is_expanded = !assistant->is_expanded;
	if(assistant->is_expanded)
		main_split.SetPos(8000, 1);
	else
		main_split.SetPos(10000, 1);
}

void MaestroHubCockpit::OnAssistantEvent(const MaestroEvent& e) {
	if(e.type == "tool_use" && e.tool_name == "update_task_status") {
		Value v = ParseJSON(e.tool_input);
		if(v.Is<ValueMap>()) {
			ValueMap params = v;
			if(ToLower(params["status"].ToString()) == "done") {
				// Trigger evidence collection automatically when a task is finished
				if(evidence) {
					evidence->OnCollect();
					statusbar.Set(0, "Evidence collected for finished task: " + params["task"].ToString(), 0);
				}
			}
		}
	}
}

void MaestroHubCockpit::OnEnact(String track, String phase, String task) {
	active_track = track;
	active_phase = phase;
	active_task = task;
	SyncStatus();
	
	center_tabs.Set(4); 
	
	PlanParser pp;
	pp.LoadMaestroTracks(current_root);
	
	String context = PlanSummarizer::GetPlanSummaryText(pp.tracks, track, phase, task);
	
	if(assistant) {
		assistant->chat.SessionStatus("ENACT TASK", task);
		String prompt;
		prompt << context << "\n\n";
		prompt << "I am starting work on **Task: " << task << "**.\n";
		prompt << "Please analyze the requirements and provide a plan or begin execution.";
		
		assistant->chat.input.SetData(prompt);
	}
}

void MaestroHubCockpit::OnEnactStep(String runbook_title, int step_n, String instruction) {
	center_tabs.Set(4); 
	
	if(assistant) {
		assistant->chat.SessionStatus("ENACT STEP", runbook_title + " / " + IntStr(step_n));
		String prompt;
		prompt << "Runbook: **" << runbook_title << "**\n";
		prompt << instruction << "\n";
		prompt << "Please execute this step or provide guidance.";
		
		assistant->chat.input.SetData(prompt);
	}
}

void MaestroHubCockpit::OnSessionSelect(String backend, String session_id) {
	if(assistant) {
		assistant->chat.SessionStatus(backend, session_id);
		assistant->chat.SetSession(backend, session_id);
	}
}

void MaestroHubCockpit::OnNewSession() {
	NewSessionDialog dlg;
	if(dlg.Run() == IDOK) {
		WorkSession s = WorkSessionManager::CreateSession(current_root, dlg.backend.GetData(), dlg.type.GetData());
		OnSessionSelect(dlg.backend.GetData(), s.session_id);
		LoadData();
	}
}

void MaestroHubCockpit::OnCreateIssue() {
	IssueCreateDialog dlg;
	if(dlg.Run() == IDOK) {
		IssueManager ism(current_root);
		if(ism.SaveIssue(dlg.GetIssue())) {
			PromptOK("Issue created.");
			LoadData();
		}
	}
}

void MaestroHubCockpit::OnInitMaestro() {
	InitDialog dlg;
	if(dlg.Run() == IDOK) {
		String target = dlg.dir.GetData();
		if(!target.IsEmpty()) {
			RealizeDirectory(AppendFileName(target, ".maestro"));
			RealizeDirectory(AppendFileName(target, "docs/runbooks"));
			RealizeDirectory(AppendFileName(target, "docs/workflows"));
			RealizeDirectory(AppendFileName(target, "docs/issues"));
			RealizeDirectory(AppendFileName(target, "uppsrc/AI/plan"));
			
			current_root = target;
			config.AddDir(current_root);
			LoadData();
			PromptOK("Maestro initialized in " + target);
		}
	}
}

void MaestroHubCockpit::OnTriageWizard() {
	TriageDialog dlg;
	dlg.Load(current_root);
	dlg.Run();
	LoadData(); 
}

void MaestroHubCockpit::OnRunbookEditor() {
	RunbookEditor dlg;
	if(dlg.Run() == IDOK) {
		LoadData();
	}
}

void MaestroHubCockpit::OnStateEditor() {
	StateEditor dlg;
	dlg.Load(current_root, "main");
	dlg.Run();
	LoadData();
}

void MaestroHubCockpit::OnTUBrowser() {
	center_tabs.Set(1); 
	if(intelligence) intelligence->tabs.Set(0); 
}

void MaestroHubCockpit::OnLogAnalyzer() {
	center_tabs.Set(1); 
	if(intelligence) intelligence->tabs.Set(1); 
}

void MaestroHubCockpit::OnSettings() {
	ConfigurationDialog dlg;
	dlg.Load(current_root);
	dlg.Run();
}

void MaestroHubCockpit::OnOpsRunner() {
	OpsRunner dlg;
	dlg.Load(current_root);
	dlg.Run();
}

void MaestroHubCockpit::OnSuggestEnact() {
	if(assistant) {
		ValueMap vm = assistant->chat.suggested_task;
		assistant->chat.suggestion.Hide();
		assistant->chat.Layout();
		OnEnact(vm["track"], vm["phase"], vm["task"]);
	}
}

void MaestroHubCockpit::SyncStatus() {
	String root_name = current_root.IsEmpty() ? "No Project" : GetFileName(current_root);
	Title("Maestro Hub - " + root_name + (active_task.IsEmpty() ? "" : " [" + active_task + "]"));
	
	statusbar.Set(0, root_name, 0);
	
	if(assistant) {
		assistant->UpdateContext(active_track, active_phase, active_task);
	}
}

void MaestroHubCockpit::PlanWatcher() {
	if(current_root.IsEmpty()) return;
	
	String phases_dir = AppendFileName(current_root, "docs/phases");
	FindFile fp(AppendFileName(phases_dir, "*.md"));
	Time max_time = Time::Low();
	
	while(fp) {
		if(fp.GetLastWriteTime() > max_time) max_time = fp.GetLastWriteTime();
		fp.Next();
	}
	
	if(max_time > last_plan_check) {
		if(last_plan_check != Time::Low()) { 
			LoadData();
		}
		last_plan_check = max_time;
	}
}

END_UPP_NAMESPACE

GUI_APP_MAIN {
	using namespace Upp;
	
	MaestroToolRegistry tool_reg;

	RegisterMaestroTools(tool_reg);

	
	Upp::MaestroHubCockpit().Run();
}
