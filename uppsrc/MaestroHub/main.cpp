#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

#include "MaestroAssistant.h"
#include "FleetDashboard.h"
#include "IntelligenceHub.h"
#include "Technology.h"
#include "Product.h"
#include "Maintenance.h"
#include "IssuesView.h"
#include "IssueDialogs.h"
#include "TriageDialog.h"
#include "RunbookEditor.h"
#include "StateEditor.h"
#include "SubworkManager.h"
#include "TUBrowser.h"
#include "LogAnalyzer.h"
#include "NewSessionDialog.h"
#include "InitDialog.h"
#include "ConfigurationDialog.h"
#include "OpsRunner.h"
#include "WorkDashboard.h"
#include "SessionManagement.h"
#include "AuditTrail.h"
#include "DebugWorkspace.h"

#include "MaestroHub.h"

NAMESPACE_UPP

// Helper to generate context (moved from AIPlanner/main.cpp logic but adapted)
String GetPlanSummaryText(const Array<Track>& tracks, const String& current_track, const String& current_phase, const String& current_task) {
	return PlanSummarizer::GetPlanSummaryText(tracks, current_track, current_phase, current_task);
}

MaestroHub::MaestroHub() {
	Title("Maestro Main Hub [Refined]");
	SetRect(0, 0, 1280, 800);
	Sizeable().Zoomable();
	
	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	
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
	debug_workspace.Create();
	technology.Create();
	product.Create();
	maintenance.Create();
	issues.Create();
	work.Create();
	sessions.Create();
	
	// Left Tabs: Workspace and Pipeline
	left_tabs.Add(technology->SizePos(), "Workspace"); 
	left_tabs.Add("Pipeline"); 
	
	// Center Tabs: Functional Hubs
	center_tabs.Add(fleet->SizePos(), "Fleet Dashboard");
	center_tabs.Add(intelligence->SizePos(), "Code Intelligence");
	center_tabs.Add("Modeling & Logic"); 
	center_tabs.Add(debug_workspace->SizePos(), "Execution Console"); 
	center_tabs.Add(issues->SizePos(), "Issue Tracker");
	
	// Bottom Tabs: Output and Trace
	bottom_tabs.Add("Automation Output");
	bottom_tabs.Add("AI Trace");
	bottom_tabs.Add(audit_trail.Create().SizePos(), "System Events");
	
	// Right Panel: Global AI Assistant
	assistant.Create();
	assistant->btn_toggle << THISBACK(OnToggleAssistant);
	
	// Compose the layout
	center_split.Vert(center_tabs, bottom_tabs);
	center_split.SetPos(6500); 
	
	main_split.Horz();
	main_split << left_tabs << center_split << *assistant;
	main_split.SetPos(2000, 0);
	main_split.SetPos(8000, 1);
	
	Add(main_split.SizePos());
	
	technology->WhenEnact = THISBACK(OnEnact);
	product->WhenEnactStep = THISBACK(OnEnactStep);
	sessions->WhenSelect = THISBACK(OnSessionSelect);
	assistant->chat.enact_suggested.WhenAction = THISBACK(OnSuggestEnact);
	
	config.Load();
	if(config.recent_dirs.GetCount() > 0)
		current_root = config.recent_dirs[0];
	else
		current_root = GetCurrentDirectory();
	
	PostCallback(THISBACK(LoadData));
	
	SetTimeCallback(-2000, THISBACK(PlanWatcher));
	
	// Automated testing support
	const Vector<String>& cmdline = CommandLine();
	for(const auto& arg : cmdline) {
		if(arg == "--test-enact") {
			SetTimeCallback(1000, [=] {
				PlanParser pp;
				pp.LoadMaestroTracks(current_root);
				if(pp.tracks.GetCount() > 0 && pp.tracks[0].phases.GetCount() > 0 && pp.tracks[0].phases[0].tasks.GetCount() > 0) {
					String t = pp.tracks[0].id;
					String p = pp.tracks[0].phases[0].id;
					String k = pp.tracks[0].phases[0].tasks[0].id;
					OnEnact(t, p, k);
					
					Cout() << "=== ENACT TEST DUMP ===\n";
					Cout() << "Enacted Task: " << k << "\n";
					Cout() << "Prompt Input: " << assistant->chat.input.GetData() << "\n";
					Cout() << "=== END DUMP ===\n";
					Cout().Flush();
					Close();
				} else {
					Cout() << "ERROR: No tasks found to enact.\n";
					Cout().Flush();
					Close();
				}
			});
		}
		else if(arg == "--test-product") {
			SetTimeCallback(1000, [=] {
				center_tabs.Set(0); // Fleet Dashboard
				Cout() << "=== PRODUCT TEST DUMP ===\n";
				if(product->runbooks.GetCount() > 0) {
					product->runbooks.SetCursor(0);
					Ctrl::ProcessEvents();
					Cout() << "Selected Runbook: " << product->runbooks.Get(0) << "\n";
					Cout() << "Detail Ready: " << (!product->rb_detail.Get().IsEmpty() ? "YES" : "NO") << "\n";
				}
				if(product->workflows.GetCount() > 0) {
					product->workflows.SetCursor(0);
					Ctrl::ProcessEvents();
					Cout() << "Selected Workflow: " << product->workflows.Get(0) << "\n";
					Cout() << "Detail Ready: " << (!product->wg_detail.Get().IsEmpty() ? "YES" : "NO") << "\n";
				}
				Cout() << "=== END DUMP ===\n";
				Cout().Flush();
				Close();
			});
		}
		else if(arg == "--test-sessions") {
			SetTimeCallback(1000, [=] {
				Cout() << "=== SESSIONS TEST DUMP ===\n";
				Cout() << "New layout: Sessions managed via global sidebar and dialogs.\n";
				Cout() << "=== END DUMP ===\n";
				Cout().Flush();
				Close();
			});
		}
		else if(arg == "--test-graph") {
			SetTimeCallback(1000, [=] {
				center_tabs.Set(0); // Fleet Dashboard
				Cout() << "=== GRAPH TEST DUMP ===\n";
				if(product->workflows.GetCount() > 0) {
					product->workflows.SetCursor(0);
					Ctrl::ProcessEvents();
					
					const auto& g = product->workflow_graph.GetGraph();
					Cout() << "Graph Node Count: " << g.GetNodeCount() << "\n";
					Cout() << "Graph Edge Count: " << g.GetEdgeCount() << "\n";
					Cout() << "Graph Group Count: " << g.GetGroupCount() << "\n";
					
					if(g.GetNodeCount() > 0) {
						Cout() << "First Node ID: " << g.GetNode(0).id << "\n";
					}
				} else {
					Cout() << "WARNING: No workflows found for graph test.\n";
				}
				Cout() << "=== END DUMP ===\n";
				Cout().Flush();
				Close();
			});
		}
		else if(arg == "--test-demo") {
			Cout() << "Starting visual demo tour...\n";
			
			// 1. Initial view: Technology
			SetTimeCallback(2000, [=] {
				Cout() << "Step 1: Browsing Technology Workspace...\n";
				left_tabs.Set(0);
				if(technology->plan.tree.GetChildCount(0) > 0) 
					technology->plan.tree.SetCursor(technology->plan.tree.GetChild(0, 0));
			});
			
			// 2. Switch to Fleet Dashboard
			SetTimeCallback(4000, [=] {
				Cout() << "Step 2: Viewing Fleet Dashboard...\n";
				center_tabs.Set(0);
				if(product->workflows.GetCount() > 0) {
					product->workflows.SetCursor(0);
					Cout() << "Visualizing Workflow Graph...\n";
				}
			});
			
			// 3. Select Runbook
			SetTimeCallback(6000, [=] {
				Cout() << "Step 3: Checking Runbooks...\n";
				if(product->runbooks.GetCount() > 0) product->runbooks.SetCursor(0);
			});
			
			// 4. Execution Console
			SetTimeCallback(8000, [=] {
				Cout() << "Step 4: Switching to Execution Console...\n";
				center_tabs.Set(3);
			});
			
			// 5. Audit Trail (Bottom)
			SetTimeCallback(10000, [=] {
				Cout() << "Step 5: Inspecting Audit Trail...\n";
				bottom_tabs.Set(2);
			});
			
			// 6. Finish
			SetTimeCallback(12000, [=] {
				Cout() << "Demo complete. Closing.\n";
				Close();
			});
		}
	}
}

MaestroHub::~MaestroHub() {}

void MaestroHub::MainMenu(Bar& bar) {
	bar.Sub("App", THISBACK(AppMenu));
	
	bar.Sub("Sessions", [=](Bar& b) {
		b.Add("List Sessions", [=] { center_tabs.Set(0); }); 
		b.Add("New Session...", THISBACK(OnNewSession));
		b.Separator();
		b.Add("Active Session HUD", [=] { center_tabs.Set(3); }); 
	});

	bar.Sub("Issues", [=](Bar& b) {
		b.Add("Browse Issues", [=] { center_tabs.Set(4); }); 
		b.Add("Triage Wizard...", THISBACK(OnTriageWizard));
		b.Separator();
		b.Add("Create Issue...", THISBACK(OnCreateIssue));
	});

	bar.Sub("Runbooks", [=](Bar& b) {
		b.Add("Manage Runbooks", [=] { center_tabs.Set(2); }); 
		b.Add("Visual Editor...", THISBACK(OnRunbookEditor));
		b.Separator();
		b.Add("Resolve Freeform...", [=] { PromptOK("Runbook Resolve Placeholder"); });
	});

	bar.Sub("Workflows", [=](Bar& b) {
		b.Add("Browse Workflows", [=] { center_tabs.Set(2); }); 
		b.Add("Visual Graph", [=] { center_tabs.Set(2); });
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

void MaestroHub::AppMenu(Bar& bar) {
	bar.Add("Select Root...", THISBACK(SelectRoot));
	bar.Add("Reload", THISBACK(LoadData));
	bar.Separator();
	bar.Add("Exit", THISBACK(Close));
}

void MaestroHub::SelectRoot() {
	String root = SelectDirectory();
	if(!root.IsEmpty()) {
		current_root = root;
		config.AddDir(current_root);
		LoadData();
	}
}

void MaestroHub::LoadData() {
	SyncStatus();
	if(fleet) {
		fleet->LoadProjects(config.recent_dirs);
		fleet->UpdateQueue();
	}
	if(intelligence) intelligence->Load(current_root);
	if(technology) technology->Load(current_root);
	if(product) product->Load(current_root);
	if(maintenance) maintenance->Load(current_root);
	if(issues) issues->Load(current_root);
	if(work) work->Load(current_root);
	if(sessions) sessions->Load(current_root);
	if(audit_trail) audit_trail->Load(current_root);
	
	ScanForUnblockedTasks();
}

void MaestroHub::ScanForUnblockedTasks() {
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

void MaestroHub::OnToggleAssistant() {
	if(!assistant) return;
	assistant->is_expanded = !assistant->is_expanded;
	assistant->btn_toggle.SetLabel(assistant->is_expanded ? ">" : "<");
	if(assistant->is_expanded)
		main_split.SetPos(8000, 1);
	else
		main_split.SetPos(10000, 1);
}

void MaestroHub::OnEnact(String track, String phase, String task) {
	active_track = track;
	active_phase = phase;
	active_task = task;
	SyncStatus();
	
	center_tabs.Set(3); 
	
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

void MaestroHub::OnEnactStep(String runbook_title, int step_n, String instruction) {
	center_tabs.Set(3); 
	
	if(assistant) {
		assistant->chat.SessionStatus("ENACT STEP", runbook_title + " / " + IntStr(step_n));
		String prompt;
		prompt << "Runbook: **" << runbook_title << "**\n";
		prompt << instruction << "\n";
		prompt << "Please execute this step or provide guidance.";
		
		assistant->chat.input.SetData(prompt);
	}
}

void MaestroHub::OnSessionSelect(String backend, String session_id) {
	if(assistant) {
		assistant->chat.SessionStatus(backend, session_id);
		assistant->chat.SetSession(backend, session_id);
	}
}

void MaestroHub::OnNewSession() {
	NewSessionDialog dlg;
	if(dlg.Run() == IDOK) {
		WorkSession s = WorkSessionManager::CreateSession(current_root, dlg.type.GetData(), dlg.purpose.GetData());
		OnSessionSelect(dlg.backend.GetData(), s.session_id);
		LoadData();
	}
}

void MaestroHub::OnCreateIssue() {
	IssueCreateDialog dlg;
	if(dlg.Run() == IDOK) {
		IssueManager ism(current_root);
		if(ism.SaveIssue(dlg.GetIssue())) {
			PromptOK("Issue created.");
			LoadData();
		}
	}
}

void MaestroHub::OnInitMaestro() {
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

void MaestroHub::OnTriageWizard() {
	TriageDialog dlg;
	dlg.Load(current_root);
	dlg.Run();
	LoadData(); 
}

void MaestroHub::OnRunbookEditor() {
	RunbookEditor dlg;
	dlg.New(current_root);
	if(dlg.Run() == IDOK) {
		LoadData();
	}
}

void MaestroHub::OnStateEditor() {
	StateEditor dlg;
	dlg.Load(current_root, "main");
	dlg.Run();
	LoadData();
}

void MaestroHub::OnTUBrowser() {
	center_tabs.Set(1); 
	if(intelligence) intelligence->tabs.Set(1); 
}

void MaestroHub::OnLogAnalyzer() {
	center_tabs.Set(1); 
	if(intelligence) intelligence->tabs.Set(2); 
}

void MaestroHub::OnSettings() {
	ConfigurationDialog dlg;
	dlg.Load(current_root);
	dlg.Run();
}

void MaestroHub::OnOpsRunner() {
	OpsRunner dlg;
	dlg.Load(current_root);
	dlg.Run();
}

void MaestroHub::OnSuggestEnact() {
	if(assistant) {
		ValueMap vm = assistant->chat.suggested_task;
		assistant->chat.suggestion.Hide();
		assistant->chat.Layout();
		OnEnact(vm["track"], vm["phase"], vm["task"]);
	}
}

void MaestroHub::SyncStatus() {
	String root_name = current_root.IsEmpty() ? "No Project" : GetFileName(current_root);
	Title("Maestro Hub - " + root_name + (active_task.IsEmpty() ? "" : " [" + active_task + "]"));
	
	statusbar.Set(0, root_name, 0);
	
	if(assistant) {
		assistant->UpdateContext(active_track, active_phase, active_task);
	}
}

void MaestroHub::PlanWatcher() {
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

	

	Upp::MaestroHub().Run();
}
