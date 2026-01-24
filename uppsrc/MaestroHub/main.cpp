#include "MaestroHub.h"
#include "Technology.h"
#include "Product.h"
#include "Maintenance.h"
#include "SessionManagement.h"
#include <AI/Engine/PlanSummarizer.h>

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
	sessions.Create();
	
	Add(tabs.SizePos());
	tabs.Add(technology->SizePos(), "Technology");
	tabs.Add(product->SizePos(), "Product");
	tabs.Add(maintenance->SizePos(), "Maintenance");
	tabs.Add(sessions->SizePos(), "Sessions");
	
	technology->WhenEnact = THISBACK(OnEnact);
	product->WhenEnactStep = THISBACK(OnEnactStep);
	
	config.Load();
	if(config.recent_dirs.GetCount() > 0)
		current_root = config.recent_dirs[0];
	else
		current_root = GetHomeDirectory() + "/Dev/Maestro";
	
	PostCallback(THISBACK(LoadData));
	
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
	}
}

void MaestroHub::MainMenu(Bar& bar) {
	bar.Sub("App", THISBACK(AppMenu));
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
	if(sessions) sessions->Load(current_root);
}

void MaestroHub::OnEnact(String track, String phase, String task) {
	int maint_idx = 2;
	tabs.Set(maint_idx);
	
	PlanParser pp;
	pp.LoadMaestroTracks(current_root);
	
	String context = PlanSummarizer::GetPlanSummaryText(pp.tracks, track, phase, task);
	
	if(maintenance) {
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
		String prompt;
		prompt << "Runbook: **" << runbook_title << "**\n";
		prompt << instruction << "\n";
		prompt << "Please execute this step or provide guidance.";
		
		maintenance->chat.input.SetData(prompt);
	}
}

END_UPP_NAMESPACE

GUI_APP_MAIN {
	Upp::MaestroHub().Run();
}