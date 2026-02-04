#include "Maestro.h"

NAMESPACE_UPP

String PlanSummarizer::GetPlanSummaryText(const Array<Track>& tracks, const String& current_track, const String& current_phase, const String& current_task) {
	String res;
	res << "# Maestro Project Plan Summary\n\n";
	
	if(!current_track.IsEmpty()) {
		res << "## Current Context\n";
		res << "- **Track:** " << current_track << "\n";
		if(!current_phase.IsEmpty()) res << "- **Phase:** " << current_phase << "\n";
		if(!current_task.IsEmpty()) res << "- **Task:** " << current_task << "\n";
		res << "\n";
	}
	
	res << "## Project Overview\n";
	for(const auto& t : tracks) {
		res << "### Track: " << t.name << " (" << t.status << ", " << t.completion << "%)\n";
		for(const auto& p : t.phases) {
			bool is_active_phase = (t.id == current_track && p.id == current_phase);
			res << "- " << (is_active_phase ? "**[ACTIVE]** " : "") << "Phase: " << p.name 
			    << " (" << p.status << ", " << p.completion << "%)\n";
			
			for(const auto& tk : p.tasks) {
				bool is_active_task = (tk.id == current_task || tk.path.EndsWith(current_task));
				res << "  - " << (is_active_task ? "**[CURRENT]** " : "") << tk.name 
				    << " [" << StatusToString(tk.status) << "]\n";
			}
		}
		res << "\n";
	}
	return res;
}

String PlanSummarizer::GetRunbookSummary(const Runbook& rb) {
	String res;
	res << "# Runbook: " << rb.title << " (" << rb.id << ")\n";
	res << "**Goal:** " << rb.goal << "\n\n";
	res << "## Steps\n";
	for(const auto& s : rb.steps) {
		res << s.n << ". **" << s.action << "** [" << s.actor << "]\n";
		if(!s.command.IsEmpty()) res << "   Command: `" << s.command << "`\n";
		if(!s.expected.IsEmpty()) res << "   Expected: " << s.expected << "\n";
	}
	return res;
}

String PlanSummarizer::GetWorkGraphSummary(const WorkGraph& wg) {
	String res;
	res << "# WorkGraph: " << wg.title << " (" << wg.id << ")\n";
	res << "**Goal:** " << wg.goal << "\n";
	res << "**Track:** " << wg.track.name << " (" << wg.track.status << ")\n\n";
	
	res << "## Phases\n";
	for(const auto& p : wg.phases) {
		res << "### " << p.name << "\n";
		for(const auto& t : p.tasks) {
			res << "- " << t.title << " (" << t.status << ")\n";
			if(!t.intent.IsEmpty()) res << "  Intent: " << t.intent << "\n";
		}
	}
	return res;
}

END_UPP_NAMESPACE
