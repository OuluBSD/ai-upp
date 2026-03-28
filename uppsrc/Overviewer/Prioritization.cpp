#include "Overviewer.h"

EntryScore OverviewerProject::ComputeScore(const String& path) const {
	EntryScore res;
	const FileMetadata* m = metadata.FindPtr(path);
	if(!m) return res;

	auto add_factor = [&](double val, const String& desc) {
		if(val != 0) {
			res.score += val;
			res.factors.Add(desc + " (" + (val > 0 ? "+" : "") + FormatDouble(val, 1) + ")");
		}
	};

	// 1. Explicit Priority (Weight: 10 per point)
	add_factor(m->priority * 10.0, "Priority=" + AsString(m->priority));

	// 2. Completion (Weight: 5 per missing point)
	if(m->completion > 0)
		add_factor((5 - m->completion) * 5.0, "Completion=" + AsString(m->completion));
	else
		add_factor(25.0, "No completion set");

	// 3. Problems (Weight: 8 per problem)
	if(!m->problems.IsEmpty()) {
		int count = 0;
		for(const auto& p : m->problems) if(!p.done) count++;
		add_factor(count * 8.0, AsString(count) + " open problems");
	}

	// 4. Tasks (Weight: 4 per task)
	if(!m->tasks.IsEmpty()) {
		int count = 0;
		for(const auto& t : m->tasks) if(!t.done) count++;
		add_factor(count * 4.0, AsString(count) + " open tasks");
	}

	// 5. Flags (Weight: 15 per review flag)
	if(m->flags & FLAG_NEEDS_REVIEW) add_factor(15.0, "Flag: NEEDS_REVIEW");
	if(m->flags & FLAG_CONTENT_NEEDS_REVIEW) add_factor(15.0, "Flag: CONTENT_NEEDS_REVIEW");

	// 6. Gap Tags (Weight: 5 per tag)
	if(!m->gap_tags.IsEmpty())
		add_factor(m->gap_tags.GetCount() * 5.0, AsString(m->gap_tags.GetCount()) + " gap tags");

	// 7. Suggestions (Weight: 3 per pending suggestion)
	const EntrySuggestions* sug = suggestions.FindPtr(path);
	if(sug) {
		int pending = 0;
		auto count = [&](const Vector<Suggestion>& v) { for(const auto& s : v) if(!s.rejected) pending++; };
		count(sug->current_tags); count(sug->reason_tags); count(sug->gap_tags);
		count(sug->problems); count(sug->tasks);
		if(pending > 0) add_factor(pending * 3.0, AsString(pending) + " pending suggestions");
	}

	// 8. Stale entries (Weight: 10 if stale)
	Time now = GetSysTime();
	Time last_touch;
	bool found = false;
	for(int i = history.GetCount() - 1; i >= 0; i--) {
		if(history[i].path == path) {
			last_touch = history[i].time;
			found = true;
			break;
		}
	}
	if(found && (now - last_touch > 30 * 24 * 3600))
		add_factor(10.0, "Stale entry (untouched 30d)");

	return res;
}

static bool EntryScoreCompare(const EntryScore& a, const EntryScore& b) {
	return a.score > b.score;
}

VectorMap<String, EntryScore> OverviewerProject::GetActionView(int limit) const {
	VectorMap<String, EntryScore> res;
	for(int i = 0; i < metadata.GetCount(); i++) {
		String p = metadata.GetKey(i);
		EntryScore s = ComputeScore(p);
		if(s.score > 0)
			res.Add(p, s);
	}
	
	// Sort by score descending
	Vector<int> p;
	for(int i = 0; i < res.GetCount(); i++) p.Add(i);
	Sort(p, [&](int a, int b) { return res[a].score > res[b].score; });
	
	VectorMap<String, EntryScore> sorted;
	for(int i = 0; i < p.GetCount() && (limit <= 0 || i < limit); i++)
		sorted.Add(res.GetKey(p[i]), res[p[i]]);
	
	return sorted;
}
