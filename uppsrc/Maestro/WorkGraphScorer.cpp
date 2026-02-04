#include "WorkGraphScorer.h"

namespace Upp {

RankedWorkGraph WorkGraphScorer::Rank(const WorkGraph& wg, const String& profile) {
	RankedWorkGraph rwg;
	rwg.workgraph_id = wg.id;
	rwg.profile = profile;
	
	for(const auto& p : wg.phases) {
		for(const auto& t : p.tasks) {
			rwg.ranked_tasks.Add(ScoreTask(t, profile, wg.domain));
		}
	}
	
	Sort(rwg.ranked_tasks, [](const ScoreResult& a, const ScoreResult& b) { return a.score > b.score; });
	
	int quick_wins = 0;
	int risky_bets = 0;
	int purpose_wins = 0;
	double total_score = 0;
	
	for(const auto& t : rwg.ranked_tasks) {
		if(t.score >= 5 && t.effort_bucket <= 2) quick_wins++;
		if(t.risk >= 4) risky_bets++;
		if(t.purpose >= 4) purpose_wins++;
		total_score += t.score;
	}
	
	rwg.summary.Add("total_tasks", rwg.ranked_tasks.GetCount());
	rwg.summary.Add("quick_wins", quick_wins);
	rwg.summary.Add("risky_bets", risky_bets);
	rwg.summary.Add("purpose_wins", purpose_wins);
	rwg.summary.Add("avg_score", rwg.ranked_tasks.IsEmpty() ? 0 : total_score / rwg.ranked_tasks.GetCount());
	
	return rwg;
}

ScoreResult WorkGraphScorer::ScoreTask(const WorkGraphTask& t, const String& profile, const String& domain) {
	ScoreResult sr;
	sr.task_id = t.id;
	sr.task_title = t.title;
	
	if(!t.effort.IsEmpty()) {
		int avg = (int)t.effort["min"] + (int)t.effort["max"] / 2;
		if(avg <= 5) sr.effort_bucket = 1;
		else if(avg <= 15) sr.effort_bucket = 2;
		else if(avg <= 60) sr.effort_bucket = 3;
		else if(avg <= 240) sr.effort_bucket = 4;
		else sr.effort_bucket = 5;
	} else {
		sr.effort_bucket = InferEffort(t);
		sr.inferred_fields.Add("effort");
	}
	
	if(!t.impact.IsVoid()) sr.impact = (int)t.impact;
	else { sr.impact = InferImpact(t, domain); sr.inferred_fields.Add("impact"); }
	
	if(!t.risk_score.IsVoid()) sr.risk = (int)t.risk_score;
	else { sr.risk = InferRisk(t); sr.inferred_fields.Add("risk"); }
	
	if(!t.purpose.IsVoid()) sr.purpose = (int)t.purpose;
	else { sr.purpose = InferPurpose(t); sr.inferred_fields.Add("purpose"); }
	
	if(profile == "investor")
		sr.score = (sr.impact * 3 + sr.purpose) - (sr.effort_bucket * 2 + sr.risk * 2);
	else if(profile == "purpose")
		sr.score = (sr.purpose * 3 + sr.impact) - (sr.effort_bucket + sr.risk);
	else // default
		sr.score = (sr.impact * 2 + sr.purpose) - (sr.effort_bucket + sr.risk);
		
	return sr;
}

int WorkGraphScorer::InferEffort(const WorkGraphTask& t) {
	int cmd_count = 0;
	for(const auto& dod : t.definition_of_done) if(dod.kind == "command") cmd_count++;
	for(const auto& v : t.verification) if(v.kind == "command") cmd_count++;
	
	int effort = 3;
	if(cmd_count == 0) effort = 2;
	else if(cmd_count == 1) effort = 2;
	else if(cmd_count <= 3) effort = 3;
	else if(cmd_count <= 6) effort = 4;
	else effort = 5;
	
	if(!t.safe_to_execute) effort = min(5, effort + 1);
	
	return effort;
}

int WorkGraphScorer::InferImpact(const WorkGraphTask& t, const String& domain) {
	int impact = 2;
	String text = ToLower(t.title + " " + t.intent);
	
	if(domain == "issues") {
		if(text.Find("blocker") >= 0 || text.Find("critical") >= 0) impact = 5;
		else if(text.Find("fix") >= 0 || text.Find("bug") >= 0) impact = 4;
	}
	
	for(const auto& tag : t.tags) {
		String tg = ToLower(tag);
		if(tg == "build" || tg == "fix" || tg == "critical") impact = max(impact, 4);
		if(tg == "feature" || tg == "improvement") impact = max(impact, 3);
	}
	
	if(!t.outputs.IsEmpty()) impact = min(5, impact + 1);
	
	return impact;
}

int WorkGraphScorer::InferRisk(const WorkGraphTask& t) {
	int risk = 2;
	if(!t.safe_to_execute) risk = 4;
	if(t.outputs.GetCount() > 5) risk = min(5, risk + 2);
	
	for(const auto& tag : t.tags) {
		String tg = ToLower(tag);
		if(tg == "unsafe" || tg == "experimental" || tg == "destructive") risk = 5;
		if(tg == "readonly" || tg == "docs") risk = max(0, risk - 2);
	}
	return risk;
}

int WorkGraphScorer::InferPurpose(const WorkGraphTask& t) {
	int purpose = 2;
	for(const auto& tag : t.tags) {
		String tg = ToLower(tag);
		if(tg == "docs" || tg == "user-facing" || tg == "ux") purpose = 5;
		if(tg == "build" || tg == "internal" || tg == "cleanup") purpose = 1;
	}
	return purpose;
}

}
