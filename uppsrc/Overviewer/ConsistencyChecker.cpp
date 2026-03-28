#include "Overviewer.h"

ProjectDashboard OverviewerProject::GetDashboard() const {
	ProjectDashboard db;
	Time now = GetSysTime();
	
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
		
		// Stale check
		bool found = false;
		for(int j = history.GetCount() - 1; j >= 0; j--) {
			if(history[j].path == path) {
				if(now - history[j].time > 30 * 24 * 3600) db.stale_entries++;
				found = true;
				break;
			}
		}
		if(!found && history.GetCount() > 0) db.stale_entries++;
	}
	
	db.recent_changes = history.GetCount();
	Index<String> recent_paths;
	for(int i = history.GetCount() - 1; i >= 0 && i >= history.GetCount() - 100; i--) {
		const auto& e = history[i];
		if(!e.path.IsEmpty()) recent_paths.FindAdd(e.path);
		if(!e.actor_id.IsEmpty()) db.activity_by_actor.GetAdd(e.actor_type + ":" + e.actor_id, 0)++;
	}
	for(const String& p : recent_paths) db.recently_modified.Add(p);

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
	
	for(int i = 0; i < decisions.GetCount(); i++) {
		const Decision& d = decisions[i];
		if(d.status == "proposed") db.proposed_decisions++;
		else if(d.status == "accepted") db.accepted_decisions++;
	}
	
	db.total_comments = comments.GetCount();
	
	VectorMap<String, EntryScore> actions = GetActionView(5);
	for(int i = 0; i < actions.GetCount(); i++)
		db.top_action_items.Add(actions.GetKey(i), actions[i].score);
	
	return db;
}

void OverviewerProject::RunConsistencyCheck() {
	review_queue.Clear();
	Time now = GetSysTime();
	
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

		if(m.flags & FLAG_WRONG_LOCATION) {
			String parent_path = GetFileDirectory(path);
			const FileMetadata* pm = metadata.FindPtr(parent_path);
			if(pm && !pm->reason_tags.IsEmpty()) {
				add_review(path, "Location", "WRONG_LOCATION flag set, but parent has reason tags.", 1, "checker");
			}
		}

		if(m.completion == 5 && m.priority == 5) {
			int active_tasks = 0;
			for(const auto& t : m.tasks) if(!t.done) active_tasks++;
			if(active_tasks > 3)
				add_review(path, "Logic", "High priority/completion but many active tasks.", 1, "checker");
		}

		if((!m.problems.IsEmpty() || !m.tasks.IsEmpty()) && m.notes.IsEmpty() && m.current_tags.IsEmpty()) {
			add_review(path, "Context", "Thin context: has items but no notes or tags.", 0, "checker");
		}

		if(m.priority == 0 && effective.priority != 0 && !m.problems.IsEmpty()) {
			add_review(path, "Metadata", "Relies on inherited priority while having problems.", 0, "checker");
		}
		
		if(m.flags & FLAG_NEEDS_REVIEW)
			add_review(path, "Flag", "Manual review requested via flag.", 1, "flag");

		int path_events = 0;
		Time last_touch;
		String last_actor_id;
		String last_actor_type;
		bool potential_conflict = false;
		for(int j = history.GetCount() - 1; j >= 0; j--) {
			if(history[j].path == path) {
				if(path_events == 0) {
					last_touch = history[j].time;
					last_actor_id = history[j].actor_id;
					last_actor_type = history[j].actor_type;
				} else {
					if(history[j].actor_id != last_actor_id && (last_touch - history[j].time < 3600))
						potential_conflict = true;
				}
				path_events++;
			}
		}
		
		if(potential_conflict)
			add_review(path, "Conflict", "Multiple actors modified this entry within 1h.", 1, "checker");
		
		if(path_events > 0 && (now - last_touch > 30 * 24 * 3600) && (!m.problems.IsEmpty() || !m.tasks.IsEmpty())) {
			add_review(path, "Temporal", "Entry is stale (untouched for 30d) but has active items.", 0, "checker");
		}
		
		if(path_events > 10 && m.completion == 5) {
			add_review(path, "Temporal", "Entry heavily modified recently but marked as complete.", 0, "checker");
		}
		
		if(last_actor_type == "agent" && (now - last_touch < 3600) && !(m.flags & FLAG_NEEDS_REVIEW)) {
			add_review(path, "Attribution", "Recently modified by agent; needs human review.", 0, "checker");
		}
		
		// Decisions check
		if(path_events > 20) {
			bool has_decision = false;
			for(int j = 0; j < decisions.GetCount(); j++) {
				if(FindIndex(decisions[j].related_entries, path) >= 0) {
					has_decision = true;
					break;
				}
			}
			if(!has_decision)
				add_review(path, "Planning", "High activity entry with no linked decisions.", 0, "checker");
		}
		
		// Git awareness
		if(git.repo_detected) {
			int gst = git.GetStatus(path);
			if(gst == GIT_MODIFIED && m.completion == 5)
				add_review(path, "Git", "Modified in Git but marked as complete in Overviewer.", 1, "git");
			if(gst == GIT_UNTRACKED && (m.priority >= 4 || !m.tasks.IsEmpty()))
				add_review(path, "Git", "Untracked in Git but has high priority/tasks.", 0, "git");
		}
	}

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
