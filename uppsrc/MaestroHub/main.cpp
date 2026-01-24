#include "MaestroHub.h"
#include "Technology.h"
#include "Product.h"
#include "Maintenance.h"

NAMESPACE_UPP

// Helper to generate context (moved from AIPlanner/main.cpp logic but adapted)
String GetPlanSummaryText(const Array<Track>& tracks, const String& current_track, const String& current_phase, const String& current_task) {
	String res;
	res << "# Maestro Project Plan Summary\n\n";
	res << "## Current Context\n";
	res << "- **Track:** " << current_track << "\n";
	res << "- **Phase:** " << current_phase << "\n";
	res << "- **Task:** " << current_task << "\n\n";
		res << "## Project Overview\n";
	for(const auto& t : tracks) {
		res << "### Track: " << t.name << " (" << t.status << ", " << t.completion << " %)\n";
		for(const auto& p : t.phases) {
			bool is_active_phase = (t.id == current_track && p.id == current_phase);
			res << "- " << (is_active_phase ? "**[ACTIVE]** " : "") << "Phase: " << p.name 
			    << " (" << p.status << ", " << p.completion << " %)\n";
			
			if(is_active_phase) {
				for(const auto& tk : p.tasks) {
					bool is_active_task = (tk.id == current_task || tk.path.EndsWith(current_task));
					res << "  - " << (is_active_task ? "**[CURRENT]** " : "") << tk.name 
					    << " [" << StatusToString(tk.status) << "]\n";
				}
			}
		}
		res << "\n";
	}
	return res;
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
	
	Add(tabs.SizePos());
	tabs.Add(technology->SizePos(), "Technology");
	tabs.Add(product->SizePos(), "Product");
	tabs.Add(maintenance->SizePos(), "Maintenance");
	
	technology->WhenEnact = THISBACK(OnEnact);
	
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
			// Delay slightly to allow layout and data load
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

END_UPP_NAMESPACE

GUI_APP_MAIN {
	Upp::MaestroHub().Run();
}
