#include "Maestro.h"

namespace Upp {

WorkManager::WorkManager(const String& maestro_root)
{
	base_path = NormalizePath(maestro_root);
}

Array<WorkManager::WorkItem> WorkManager::LoadAvailableWork()
{
	Array<WorkItem> items;
	
	// 1. Load Tracks/Phases from PlanParser
	PlanParser parser;
	parser.Load(FindPlanRoot());
	
	for(const auto& track : parser.tracks) {
		if(track.status != "done") {
			WorkItem& item = items.Add();
			item.id = track.id.IsEmpty() ? track.name : track.id; // Fallback
			item.type = "track";
			item.name = track.name;
			item.status = track.status;
			item.description = "Track: " + track.name;
		}
		
		for(const auto& phase : track.phases) {
			if(phase.status != "done") {
				WorkItem& item = items.Add();
				item.id = phase.id.IsEmpty() ? phase.name : phase.id;
				item.type = "phase";
				item.name = phase.name;
				item.status = phase.status;
				item.description = "Phase in " + track.name;
			}
		}
	}
	
	// 2. Load Open Issues
	IssueManager im;
	Array<MaestroIssue> issues = im.ListIssues("", "", "open");
	for(const auto& iss : issues) {
		WorkItem& item = items.Add();
		item.id = iss.issue_id;
		item.type = "issue";
		item.name = iss.title;
		item.status = iss.state;
		item.description = iss.severity + ": " + iss.message;
	}
	
	return items;
}

WorkManager::WorkItem WorkManager::SelectBestWorkItem(const Array<WorkItem>& items)
{
	if(items.IsEmpty()) return WorkItem();
	
	// Use AI to select
	CliMaestroEngine engine;
	ConfigureGemini(engine);
	
	String prompt = "Select the best work item from the following list to work on next.\n";
	prompt << "Consider priority, dependencies, and impact.\n\n";
	prompt << StoreAsJson(items) << "\n\n";
	prompt << "Return ONLY a JSON object with the selected item ID and a reason: {\"id\": \"...\", \"reason\": \"...\"}";
	
	String response;
	bool done = false;
	engine.Send(prompt, [&](const MaestroEvent& ev) {
		if(ev.type == "message") response << ev.text;
		else if(ev.type == "done") done = true;
	});
	
	while(!done && engine.Do()) Sleep(10);
	
	ValueMap result = ParseJSON(response);
	String selected_id = result["id"];
	String reason = result["reason"];
	
	for(const auto& item : items) {
		if(item.id == selected_id) {
			WorkItem selected = item;
			selected.reason = reason;
			return selected;
		}
	}
	
	// Fallback: Return first item
	if(items.GetCount() > 0) {
		WorkItem selected = items[0];
		selected.reason = "Fallback selection (AI failed or invalid ID)";
		return selected;
	}
	
	return WorkItem();
}

Array<WorkManager::WorkItem> WorkManager::SelectTopWorkItems(const Array<WorkItem>& items, int count)
{
	// Stub: Just return first N items for now to save tokens/time
	Array<WorkItem> top;
	for(int i = 0; i < min(items.GetCount(), count); i++) {
		top.Add(items[i]);
	}
	return top;
}

bool WorkManager::StartWorkSession(const WorkItem& item)
{
	Cout() << "Starting work session for " << item.type << ": " << item.name << "\n";
	Cout() << "Reason: " << item.reason << "\n";
	
	String docs_root = GetDocsRoot(FindPlanRoot());
	WorkSession s = WorkSessionManager::CreateSession(docs_root, "work_" + item.type, "Working on " + item.name);
	s.related_entity.Add(item.type + "_id", item.id);
	
	Cout() << "Session ID: " << s.session_id << "\n";
	
	// Refined Execution Logic
	CliMaestroEngine engine;
	ConfigureGemini(engine);
	
	String prompt = "You are an autonomous worker agent. Your task is to: " + item.description + "\n";
	prompt << "Context: " + item.name + " (" + item.type + ")\n\n";
	prompt << "Please provide a step-by-step plan to complete this task.\n";
	prompt << "For each step, describe the action and the expected outcome.\n";
	prompt << "Do not execute code yet, just plan.";
	
	Cout() << "Generating plan with AI...\n";
	String plan;
	engine.Send(prompt, [&](const MaestroEvent& ev) {
		if(ev.type == "message") {
			plan << ev.text;
			Cout() << ev.text;
		}
	});
	
	while(engine.Do()) Sleep(10);
	Cout() << "\n";
	
	// Save plan to breadcrumb
	Breadcrumb bc;
	bc.timestamp_id = Format(GetSysTime());
	bc.prompt = prompt;
	bc.response = plan;
	bc.model_used = engine.model;
	BreadcrumbManager::SaveBreadcrumb(bc, docs_root, s.session_id);
	
	Cout() << "Plan saved to session " << s.session_id << ".\n";
	
	// Update session state
	s.status = WorkSessionStatus::RUNNING;
	s.state = "active";
	WorkSessionManager::SaveSession(s, WorkSessionManager::FindSessionPath(docs_root, s.session_id));
	
	return true;
}

bool WorkManager::AnalyzeTarget(const String& target, bool simulate)
{
	Cout() << (simulate ? "[SIMULATE] " : "") << "Analyzing target: " << target << "\n";
	if(simulate) return true;
	
	String docs_root = GetDocsRoot(FindPlanRoot());
	WorkSession s = WorkSessionManager::CreateSession(docs_root, "analyze", "Analysis of " + target);
	
	// AI Analysis Logic Stub
	CliMaestroEngine engine;
	ConfigureGemini(engine);
	
	String prompt = "Analyze the following target: " + target + "\nProvide insights and recommendations.";
	engine.Send(prompt, [&](const MaestroEvent& ev) {
		if(ev.type == "message") Cout() << ev.text;
	});
	
	while(engine.Do()) Sleep(10);
	Cout() << "\n";
	
	return true;
}

bool WorkManager::FixTarget(const String& target, const String& issue_id, bool simulate)
{
	Cout() << (simulate ? "[SIMULATE] " : "") << "Fixing target: " << target << (issue_id.IsEmpty() ? "" : " (Issue: " + issue_id + ")") << "\n";
	if(simulate) return true;
	
	String docs_root = GetDocsRoot(FindPlanRoot());
	WorkSession s = WorkSessionManager::CreateSession(docs_root, "fix", "Fixing " + target);
	
	// AI Fix Logic Stub
	Cout() << "AI is generating a fix...\n";
	return true;
}

}
