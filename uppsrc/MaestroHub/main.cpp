#include "MaestroHub.h"
#include "TriageDialog.h"
#include "RunbookEditor.h"
#include "StateEditor.h"
#include "TUBrowser.h"
#include "LogAnalyzer.h"
#include "ConfigurationDialog.h"

NAMESPACE_UPP

// Helper to generate context (moved from AIPlanner/main.cpp logic but adapted)
String GetPlanSummaryText(const Array<Track>& tracks, const String& current_track, const String& current_phase, const String& current_task) {
	return PlanSummarizer::GetPlanSummaryText(tracks, current_track, current_phase, current_task);
}

MaestroHub::MaestroHub() {
	Title("Maestro Main Hub");
	SetRect(0, 0, 1200, 800);
	Sizeable().Zoomable();
	
	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	
	technology.Create();
	product.Create();
	maintenance.Create();
	issues.Create();
	work.Create();
	sessions.Create();
	
	Add(tabs.SizePos());
	tabs.Add(technology->SizePos(), "Technology");
	tabs.Add(product->SizePos(), "Product");
	tabs.Add(maintenance->SizePos(), "Maintenance");
	tabs.Add(issues->SizePos(), "Issues");
	tabs.Add(work->SizePos(), "Work");
	tabs.Add(sessions->SizePos(), "Sessions");
	
	technology->WhenEnact = THISBACK(OnEnact);
	product->WhenEnactStep = THISBACK(OnEnactStep);
	sessions->WhenSelect = THISBACK(OnSessionSelect);
	maintenance->chat.enact_suggested.WhenAction = THISBACK(OnSuggestEnact);
	
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
					Cout() << "Prompt Input: " << maintenance->chat.input.GetData() << "\n";
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
				tabs.Set(1);
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
				tabs.Set(3); // Sessions
				Cout() << "=== SESSIONS TEST DUMP ===\n";
				if(sessions->dirs.GetCount() > 0) {
					sessions->dirs.SetCursor(0);
					Ctrl::ProcessEvents();
					Cout() << "Selected Dir: " << (sessions->dirs.IsCursor() ? sessions->dirs.Get(0).ToString() : "NONE") << "\n";
					Cout() << "Session Count: " << sessions->sessions.GetCount() << "\n";
					if(sessions->sessions.GetCount() > 0) {
						sessions->sessions.SetCursor(0);
						Cout() << "First Session ID: " << (sessions->sessions.IsCursor() ? sessions->sessions.Get(0).ToString() : "NONE") << "\n";
					}
				} else {
					Cout() << "WARNING: No session directories found.\n";
				}
				Cout() << "=== END DUMP ===\n";
				Cout().Flush();
				Close();
			});
		}
		else if(arg == "--test-graph") {
			SetTimeCallback(1000, [=] {
				tabs.Set(1); // Product
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
				Cout() << "Step 1: Browsing Technology Plan...\n";
				tabs.Set(0);
				if(technology->plan.tree.GetChildCount(0) > 0) 
					technology->plan.tree.SetCursor(technology->plan.tree.GetChild(0, 0));
			});
			
			// 2. Switch to Product
			SetTimeCallback(4000, [=] {
				Cout() << "Step 2: Viewing Product Workflows...\n";
				tabs.Set(1);
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
			
			// 4. Enact Step (simulate click)
			SetTimeCallback(8000, [=] {
				Cout() << "Step 4: Simulating 'Execute Step' enactment...\n";
				if(product->runbook_data.GetCount() > 0) {
					const auto& rb = product->runbook_data[0];
					if(rb.steps.GetCount() > 0)
						OnEnactStep(rb.title, 1, "Demo step execution logic");
				}
			});
			
			// 5. Switch to Sessions
			SetTimeCallback(10000, [=] {
				Cout() << "Step 5: Managing Sessions...\n";
				tabs.Set(3);
				if(sessions->dirs.GetCount() > 0) sessions->dirs.SetCursor(0);
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
		b.Add("List Sessions", [=] { tabs.Set(5); }); // Sessions tab
		b.Add("New Session...", [=] { PromptOK("New Session Dialog Placeholder"); });
		b.Separator();
		b.Add("Active Session HUD", [=] { tabs.Set(4); }); // Work tab
	});

	bar.Sub("Issues", [=](Bar& b) {
		b.Add("Browse Issues", [=] { tabs.Set(3); }); // Issues tab
		b.Add("Triage Wizard...", THISBACK(OnTriageWizard));
		b.Separator();
		b.Add("Create Issue...", [=] { PromptOK("New Issue Dialog Placeholder"); });
	});

	bar.Sub("Runbooks", [=](Bar& b) {
		b.Add("Manage Runbooks", [=] { tabs.Set(1); }); // Product tab
		b.Add("Visual Editor...", THISBACK(OnRunbookEditor));
		b.Separator();
		b.Add("Resolve Freeform...", [=] { PromptOK("Runbook Resolve Placeholder"); });
	});

	bar.Sub("Workflows", [=](Bar& b) {
		b.Add("Browse Workflows", [=] { tabs.Set(1); }); // Product tab
		b.Add("Visual Graph", [=] { tabs.Set(1); });
		b.Separator();
		b.Add("State Machine Editor...", THISBACK(OnStateEditor));
	});

	bar.Sub("Intelligence", [=](Bar& b) {
		b.Add("TU Browser", THISBACK(OnTUBrowser));
		b.Add("Log Analyzer", THISBACK(OnLogAnalyzer));
		b.Separator();
		b.Add("Dependency Graph", [=] { tabs.Set(0); }); // Technology tab
	});

	bar.Sub("System", [=](Bar& b) {
		b.Add("Initialize Maestro...", [=] { PromptOK("Init placeholder"); });
		b.Add("Settings...", THISBACK(OnSettings));
		b.Separator();
		b.Add("Manage Playbooks...", [=] { PromptOK("Playbooks placeholder"); });
		b.Add("Collect Evidence...", [=] { PromptOK("Evidence placeholder"); });
		b.Add("Run Ops Doctor", [=] { PromptOK("Ops Doctor Placeholder"); });
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
	Title("Maestro Main Hub - " + current_root);
	if(technology) technology->Load(current_root);
	if(product) product->Load(current_root);
	if(maintenance) maintenance->Load(current_root);
	if(issues) issues->Load(current_root);
	if(work) work->Load(current_root);
	if(sessions) sessions->Load(current_root);
	
	ScanForUnblockedTasks();
}

void MaestroHub::ScanForUnblockedTasks() {
	if(!maintenance || current_root.IsEmpty()) return;
	
	PlanParser pp;
	pp.LoadMaestroTracks(current_root);
	
	// Index all tasks by ID for dependency checking
	Index<String> done_tasks;
	for(const auto& t : pp.tracks)
		for(const auto& p : t.phases)
			for(const auto& tk : p.tasks)
				if(tk.status == STATUS_DONE)
					done_tasks.Add(tk.id);
					
	// Find first TODO task whose dependencies are all DONE
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
						maintenance->chat.SuggestEnactment(t.id, p.id, tk.id);
						return; // Suggest only the first one found
					}
				}
			}
		}
	}
}

void MaestroHub::OnEnact(String track, String phase, String task) {
	int maint_idx = 2;
	tabs.Set(maint_idx);
	
	PlanParser pp;
	pp.LoadMaestroTracks(current_root);
	
	String context = PlanSummarizer::GetPlanSummaryText(pp.tracks, track, phase, task);
	
	if(maintenance) {
		maintenance->SessionStatus("ENACT TASK", task);
		String prompt;
		prompt << context << "\n\n";
		prompt << "I am starting work on **Task: " << task << "**.\n";
		prompt << "Please analyze the requirements and provide a plan or begin execution.";
		
		maintenance->chat.input.SetData(prompt);
	}
}

void MaestroHub::OnEnactStep(String runbook_title, int step_n, String instruction) {
	int maint_idx = 2;
	tabs.Set(maint_idx);
	
	if(maintenance) {
		maintenance->SessionStatus("ENACT STEP", runbook_title + " / " + IntStr(step_n));
		String prompt;
		prompt << "Runbook: **" << runbook_title << "**\n";
		prompt << instruction << "\n";
		prompt << "Please execute this step or provide guidance.";
		
		maintenance->chat.input.SetData(prompt);
	}
}

void MaestroHub::OnSessionSelect(String backend, String session_id) {
	tabs.Set(2); // Maintenance
	if(maintenance) {
		maintenance->SessionStatus(backend, session_id);
		maintenance->chat.SetSession(backend, session_id);
	}
}

void MaestroHub::OnSuggestEnact() {
	if(maintenance) {
		ValueMap vm = maintenance->chat.suggested_task;
		maintenance->chat.suggestion.Hide();
		maintenance->chat.Layout();
		OnEnact(vm["track"], vm["phase"], vm["task"]);
	}
}

void MaestroHub::OnTriageWizard() {
	TriageDialog dlg;
	dlg.Load(current_root);
	dlg.Run();
	LoadData(); // Refresh views
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
	// Need a way to select which workflow to edit, or just load first one for now
	dlg.Load(current_root, "main");
	dlg.Run();
	LoadData();
}

void MaestroHub::OnTUBrowser() {
	TUBrowser dlg;
	dlg.Load(current_root);
	dlg.Run();
}

void MaestroHub::OnLogAnalyzer() {
	LogAnalyzer dlg;
	dlg.Load(current_root);
	dlg.Run();
}

void MaestroHub::OnSettings() {
	ConfigurationDialog dlg;
	dlg.Load(current_root);
	dlg.Run();
}

void MaestroHub::PlanWatcher() {
	if(current_root.IsEmpty()) return;
	
	// Simple check: iterate all .md files in uppsrc/AI/plan
	// For efficiency in large repos, this should be optimized, but fine for now
	bool changed = false;
	
	FindFile ff(AppendFileName(current_root, "uppsrc/AI/plan/*"));
	while(ff) {
		// Only check directories for simplicity of recursion or assume simplified structure
		// Let's check a known marker file or just reload every few seconds?
		// Better: check recursivley but just 1 level deep for now or use a "last_modified" marker?
		// Actually, let's just check the "phases" directory as that's where status updates happen
		if(ff.IsDirectory()) {
			// Deep scan omitted for brevity, checking specific known path
		}
		ff.Next();
	}
	
	// Check specific legacy location used by PlanParser
	String phases_dir = AppendFileName(current_root, "docs/phases");
	FindFile fp(AppendFileName(phases_dir, "*.md"));
	Time max_time = Time::Low();
	
	while(fp) {
		if(fp.GetLastWriteTime() > max_time) max_time = fp.GetLastWriteTime();
		fp.Next();
	}
	
	// Also check new location
	// Note: Ideally PlanParser would expose a "GetLastModified()" method
	
	if(max_time > last_plan_check) {
		if(last_plan_check != Time::Low()) { // Don't reload on first check
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
