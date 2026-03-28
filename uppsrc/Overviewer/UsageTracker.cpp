#include "UsageTracker.h"
#include "Overviewer.h"

UsageSummary UsageTracker::GetSummary(const OverviewerProject& p) {
	UsageSummary s;
	s.total_actions = p.usage_history.GetCount();
	s.sessions_count = p.sessions.GetCount();
	
	Index<String> features;
	features.Add("Scenario");
	features.Add("Decision");
	features.Add("Insight");
	features.Add("Review");
	features.Add("Overview");
	features.Add("Git");
	
	Index<String> used_features;
	
	for(const auto& e : p.usage_history) {
		s.top_actions.GetAdd(e.action_type, 0)++;
		if(!e.target.IsEmpty()) s.top_targets.GetAdd(e.target, 0)++;
		
		for(const String& f : features) {
			if(e.action_type.Find(ToLower(f)) >= 0)
				used_features.FindAdd(f);
		}
	}
	
	for(const String& f : features) {
		if(used_features.Find(f) < 0)
			s.unused_features.Add(f);
	}
	
	auto sort_map = [](VectorMap<String, int>& m) {
		Vector<int> order = GetSortOrder(m.GetValues(), std::greater<int>());
		VectorMap<String, int> sorted;
		for(int i : order) sorted.Add(m.GetKey(i), m[i]);
		m = pick(sorted);
	};
	
	sort_map(s.top_actions);
	sort_map(s.top_targets);
	
	return s;
}

Vector<FrictionSignal> UsageTracker::GetFriction(const OverviewerProject& p) {
	Vector<FrictionSignal> res;
	
	// 1. Rapid re-edit (same target, same action, multiple times in < 60s)
	VectorMap<String, Vector<Time>> edits;
	for(const auto& e : p.usage_history) {
		if(e.action_type.Find("edit") >= 0 || e.action_type.Find("set") >= 0) {
			String key = e.action_type + ":" + e.target;
			edits.GetAdd(key).Add(e.timestamp);
		}
	}
	
	for(int i = 0; i < edits.GetCount(); i++) {
		const auto& v = edits[i];
		int rapid = 0;
		for(int j = 1; j < v.GetCount(); j++) {
			if(v[j] - v[j-1] < 60) rapid++;
		}
		if(rapid > 3) {
			FrictionSignal& f = res.Add();
			f.type = "rapid_reedit";
			f.description = "Repeated modifications to '" + edits.GetKey(i) + "' in short succession. May indicate UI friction or indecision.";
			f.severity = 1;
			f.related_targets.Add(edits.GetKey(i));
		}
	}
	
	// 2. Undo Loop
	int undo_redo_cycles = 0;
	for(int i = 1; i < p.usage_history.GetCount(); i++) {
		if((p.usage_history[i-1].action_type == "undo" && p.usage_history[i].action_type == "redo") ||
		   (p.usage_history[i-1].action_type == "redo" && p.usage_history[i].action_type == "undo")) {
			undo_redo_cycles++;
		}
	}
	if(undo_redo_cycles > 2) {
		FrictionSignal& f = res.Add();
		f.type = "undo_loop";
		f.description = "Multiple undo/redo cycles detected. User might be struggling with a specific change.";
		f.severity = 2;
	}
	
	// 3. Overview Loop (generating overview many times without metadata changes)
	int overview_count = 0;
	int metadata_changes = 0;
	Time last_meta;
	for(const auto& e : p.usage_history) {
		if(e.action_type == "generate_overview") {
			overview_count++;
			if(metadata_changes == 0) { /* first one */ }
			else if(e.timestamp - last_meta > 0) { /* OK */ }
		}
		if(e.action_type.Find("metadata") >= 0) {
			metadata_changes++;
			last_meta = e.timestamp;
		}
	}
	if(overview_count > 5 && metadata_changes < 2) {
		FrictionSignal& f = res.Add();
		f.type = "overview_loop";
		f.description = "Frequent overview generation with very few metadata changes. User might be using the overview as a primary navigation or verification tool.";
		f.severity = 0;
	}

	return res;
}
