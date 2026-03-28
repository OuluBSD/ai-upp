#include "Overviewer.h"

ProjectDashboard OverviewerProject::GetDashboard() const {
	ProjectDashboard db;
	for(int i = 0; i < metadata.GetCount(); i++) {
		const String& path = metadata.GetKey(i);
		const FileMetadata& m = metadata[i];
		
		if(DirectoryExists(AppendFileName(working_dir, path))) db.total_dirs++;
		else db.total_files++;
		
		if(m.flags != 0) db.flagged_entries++;
		if(m.flags & (FLAG_NEEDS_REVIEW | FLAG_CONTENT_NEEDS_REVIEW)) db.needs_review++;
		if(m.priority == 0) db.missing_priority++;
		if(m.completion == 0) db.missing_completion++;
		if(!m.notes.IsEmpty()) db.with_notes++;
		if(!m.problems.IsEmpty()) db.with_problems++;
		if(!m.tasks.IsEmpty()) db.with_tasks++;
		if(!m.leads.IsEmpty()) db.with_leads++;
		
		if(m.priority >= 0 && m.priority <= 5) db.priority_counts[m.priority]++;
		
		for(const String& t : m.reason_tags) db.top_reason_tags.GetAdd(t, 0)++;
		for(const String& t : m.gap_tags) db.top_gap_tags.GetAdd(t, 0)++;
		for(const String& t : m.current_tags) db.top_current_tags.GetAdd(t, 0)++;
	}
	
	for(int i = 0; i < suggestions.GetCount(); i++) {
		const EntrySuggestions& s = suggestions[i];
		int pending = 0;
		auto count_pending = [&](const Vector<Suggestion>& v) {
			for(const Suggestion& sug : v) if(!sug.rejected) pending++;
		};
		count_pending(s.current_tags);
		count_pending(s.reason_tags);
		count_pending(s.gap_tags);
		count_pending(s.problems);
		count_pending(s.tasks);
		if(pending > 0) db.suggestions_pending++;
	}
	
	return db;
}

void OverviewerProject::RunConsistencyCheck() {
	review_queue.Clear();
	
	auto add_review = [&](const String& path, const String& type, const String& msg, int sev, const String& src) {
		String id = path + ":" + msg;
		if(dismissed_review_ids.Find(id) >= 0) return;
		ReviewItem& it = review_queue.Add();
		it.path = path;
		it.type = type;
		it.message = msg;
		it.severity = sev;
		it.source = src;
	};

	for(int i = 0; i < metadata.GetCount(); i++) {
		const String& path = metadata.GetKey(i);
		const FileMetadata& m = metadata[i];
		FileMetadata effective = GetEffectiveMetadata(path);

		// A. Wrong location suspicion (simplified heuristic)
		if(m.flags & FLAG_WRONG_LOCATION) {
			String parent_path = GetFileDirectory(path);
			const FileMetadata* pm = metadata.FindPtr(parent_path);
			if(pm && !pm->reason_tags.IsEmpty()) {
				add_review(path, "Location", "WRONG_LOCATION flag set, but parent has reason tags.", 1, "checker");
			}
		}

		// C. Completion/priority mismatch
		if(m.completion == 5 && m.priority == 5) {
			int active_tasks = 0;
			for(const auto& t : m.tasks) if(!t.done) active_tasks++;
			if(active_tasks > 3)
				add_review(path, "Logic", "High priority/completion but many active tasks.", 1, "checker");
		}

		// E. Missing context
		if((!m.problems.IsEmpty() || !m.tasks.IsEmpty()) && m.notes.IsEmpty() && m.current_tags.IsEmpty()) {
			add_review(path, "Context", "Thin context: has items but no notes or tags.", 0, "checker");
		}

		// G. Review-worthy inherited-only
		if(m.priority == 0 && effective.priority != 0 && !m.problems.IsEmpty()) {
			add_review(path, "Metadata", "Relies on inherited priority while having problems.", 0, "checker");
		}
		
		if(m.flags & FLAG_NEEDS_REVIEW)
			add_review(path, "Flag", "Manual review requested via flag.", 1, "flag");
	}

	// Suggestions as review items
	for(int i = 0; i < suggestions.GetCount(); i++) {
		const String& path = suggestions.GetKey(i);
		const EntrySuggestions& s = suggestions[i];
		int pending = 0;
		auto check = [&](const Vector<Suggestion>& v) {
			for(const auto& sug : v) if(!sug.rejected) pending++;
		};
		check(s.current_tags); check(s.reason_tags); check(s.gap_tags);
		check(s.problems); check(s.tasks);
		if(pending > 0)
			add_review(path, "Suggestion", "Has " + AsString(pending) + " pending suggestions.", 0, "AI");
	}
}
