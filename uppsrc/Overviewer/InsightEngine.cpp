#include "InsightEngine.h"
#include "Overviewer.h"

Vector<Insight> InsightEngine::Generate(const OverviewerProject& project) {
	Vector<Insight> res;
	CheckChurn(project, res);
	CheckStagnation(project, res);
	CheckDecisionDrift(project, res);
	CheckReviewAccumulation(project, res);
	CheckScenarioAbandonment(project, res);
	CheckCommentHotspots(project, res);
	
	for(auto& i : res) {
		i.timestamp_generated = GetSysTime();
		if(i.id.IsEmpty()) i.id = i.type + "_" + AsString(Uuid::Create());
	}
	
	return res;
}

void InsightEngine::CheckChurn(const OverviewerProject& p, Vector<Insight>& res) {
	Time now = GetSysTime();
	VectorMap<String, Index<String>> actor_churn; // path -> set of actor_ids
	VectorMap<String, int> event_count;
	
	for(const auto& e : p.history) {
		if(now - e.time < 7 * 24 * 3600) { // last 7 days
			if(!e.path.IsEmpty()) {
				actor_churn.GetAdd(e.path).FindAdd(e.actor_id);
				event_count.GetAdd(e.path, 0)++;
			}
		}
	}
	
	for(int i = 0; i < actor_churn.GetCount(); i++) {
		if(event_count[i] > 10 || actor_churn[i].GetCount() > 2) {
			Insight& ins = res.Add();
			ins.type = "churn";
			ins.title = "High Churn Detected";
			ins.description = "Entry " + actor_churn.GetKey(i) + " is being modified frequently by multiple actors.";
			ins.severity = actor_churn[i].GetCount() > 3 ? 2 : 1;
			ins.related_entries.Add(actor_churn.GetKey(i));
			ins.supporting_evidence.Add(AsString(event_count[i]) + " events in 7 days");
			ins.supporting_evidence.Add(AsString(actor_churn[i].GetCount()) + " distinct actors");
		}
	}
}

void InsightEngine::CheckStagnation(const OverviewerProject& p, Vector<Insight>& res) {
	Time now = GetSysTime();
	for(int i = 0; i < p.metadata.GetCount(); i++) {
		const String& path = p.metadata.GetKey(i);
		const FileMetadata& m = p.metadata[i];
		if(m.priority >= 4 && m.completion <= 1) {
			// check history
			bool recent = false;
			for(int j = p.history.GetCount()-1; j >= 0; j--) {
				if(p.history[j].path == path) {
					if(now - p.history[j].time < 14 * 24 * 3600) recent = true;
					break;
				}
			}
			if(!recent) {
				Insight& ins = res.Add();
				ins.type = "stagnation";
				ins.title = "High Priority Stagnation";
				ins.description = "Critical entry " + path + " has high priority but no recent progress.";
				ins.severity = 2;
				ins.related_entries.Add(path);
				ins.supporting_evidence.Add("Priority: " + AsString(m.priority));
				ins.supporting_evidence.Add("Completion: " + AsString(m.completion));
				ins.supporting_evidence.Add("No changes in 14 days");
			}
		}
	}
}

void InsightEngine::CheckDecisionDrift(const OverviewerProject& p, Vector<Insight>& res) {
	Time now = GetSysTime();
	for(int i = 0; i < p.decisions.GetCount(); i++) {
		const Decision& d = p.decisions[i];
		if(d.status == "accepted") {
			bool action_seen = false;
			for(const auto& entry : d.related_entries) {
				for(int j = p.history.GetCount()-1; j >= 0; j--) {
					if(p.history[j].path == entry && p.history[j].time > d.timestamp) {
						action_seen = true;
						break;
					}
				}
				if(action_seen) break;
			}
			if(!action_seen && (now - d.timestamp > 7 * 24 * 3600)) {
				Insight& ins = res.Add();
				ins.type = "drift";
				ins.title = "Decision Drift / Inaction";
				ins.description = "Decision '" + d.title + "' was accepted but no related entries have been modified since.";
				ins.severity = 1;
				for(const auto& e : d.related_entries) ins.related_entries.Add(e);
				ins.supporting_evidence.Add("Accepted on " + Format(d.timestamp));
			}
		}
	}
}

void InsightEngine::CheckReviewAccumulation(const OverviewerProject& p, Vector<Insight>& res) {
	if(p.review_queue.GetCount() > 20) {
		Insight& ins = res.Add();
		ins.type = "review_debt";
		ins.title = "Review Debt Accumulating";
		ins.description = "There are many pending review items that may indicate a bottleneck in the understanding process.";
		ins.severity = p.review_queue.GetCount() > 50 ? 2 : 1;
		ins.supporting_evidence.Add(AsString(p.review_queue.GetCount()) + " items in queue");
	}
}

void InsightEngine::CheckScenarioAbandonment(const OverviewerProject& p, Vector<Insight>& res) {
	Time now = GetSysTime();
	for(int i = 0; i < p.scenarios.GetCount(); i++) {
		// Scenarios currently don't store timestamp, but we could infer from history if we wanted.
		// For now, let's just flag if there are more than 5 scenarios.
	}
	if(p.scenarios.GetCount() > 5) {
		Insight& ins = res.Add();
		ins.type = "scenario_bloat";
		ins.title = "Scenario Proliferation";
		ins.description = "Many active scenarios exist. Consider applying or deleting stale ones.";
		ins.severity = 0;
		ins.supporting_evidence.Add(AsString(p.scenarios.GetCount()) + " active scenarios");
	}
}

void InsightEngine::CheckCommentHotspots(const OverviewerProject& p, Vector<Insight>& res) {
	VectorMap<String, int> counts;
	for(const auto& c : p.comments) {
		if(!c.related_entry.IsEmpty())
			counts.GetAdd(c.related_entry, 0)++;
	}
	
	for(int i = 0; i < counts.GetCount(); i++) {
		if(counts[i] > 5) {
			Insight& ins = res.Add();
			ins.type = "hotspot";
			ins.title = "Comment Hotspot";
			ins.description = "Entry " + counts.GetKey(i) + " has significant discussion activity.";
			ins.severity = 1;
			ins.related_entries.Add(counts.GetKey(i));
			ins.supporting_evidence.Add(AsString(counts[i]) + " comments");
		}
	}
}
