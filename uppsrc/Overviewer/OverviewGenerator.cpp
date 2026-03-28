#include "OverviewGenerator.h"
#include "Overviewer.h"

void OverviewGenerator::AddSection(Vector<Section>& sections, const String& title, const String& content) {
	if(content.IsEmpty()) return;
	Section& s = sections.Add();
	s.title = title;
	s.content = content;
}

String OverviewGenerator::Format(const Vector<Section>& sections, bool markdown) {
	String out;
	for(const auto& s : sections) {
		if(markdown) {
			out << "## " << s.title << "\n\n";
			out << s.content << "\n\n";
		} else {
			out << s.title << "\n" << String('=', s.title.GetCount()) << "\n\n";
			out << s.content << "\n\n";
		}
	}
	return out;
}

String OverviewGenerator::Generate(const String& rel_path, const OverviewOptions& opt) {
	Vector<Section> sections;
	String title = rel_path.IsEmpty() ? "Project Overview" : "Overview: " + rel_path;
	
	AddSection(sections, "Summary", GetSummary(rel_path, opt));
	AddSection(sections, "Purpose", GetWhy(rel_path, opt));
	AddSection(sections, "Known Issues", GetIssues(rel_path, opt));
	AddSection(sections, "Active Tasks", GetTasks(rel_path, opt));
	AddSection(sections, "Future Leads", GetLeads(rel_path, opt));
	AddSection(sections, "Important Items", GetImportant(rel_path, opt));
	AddSection(sections, "Action Priorities", GetPriorities(rel_path, opt));
	AddSection(sections, "Review Notes", GetReview(rel_path, opt));

	String result = opt.markdown_output ? "# " + title + "\n\n" : title + "\n" + String('*', title.GetCount()) + "\n\n";
	result << Format(sections, opt.markdown_output);
	return result;
}

String OverviewGenerator::GenerateProject(const OverviewOptions& opt) {
	return Generate("", opt);
}

String OverviewGenerator::GetSummary(const String& path, const OverviewOptions& opt) {
	String out;
	if(path.IsEmpty()) {
		ProjectDashboard db = project.GetDashboard();
		out << "Total Files: " << db.total_files << "\n";
		out << "Total Directories: " << db.total_dirs << "\n";
		out << "Flagged Entries: " << db.flagged_entries << "\n";
		out << "Recent Changes: " << db.recent_changes << "\n";
	} else {
		const FileMetadata* m = project.metadata.FindPtr(path);
		if(m) {
			out << "Priority: " << m->priority << "\n";
			out << "Completion: " << m->completion << " / 5\n";
			if(opt.include_notes_summary && !m->notes.IsEmpty())
				out << "\nNotes:\n" << m->notes << "\n";
		}
	}
	return out;
}

String OverviewGenerator::GetWhy(const String& path, const OverviewOptions& opt) {
	String out;
	auto collect_reasons = [&](const String& p) {
		const FileMetadata* m = project.metadata.FindPtr(p);
		if(m && !m->reason_tags.IsEmpty()) {
			if(!out.IsEmpty()) out << "\n";
			out << "- " << p << ": " << Join(m->reason_tags, ", ");
		}
	};

	if(path.IsEmpty()) {
		for(int i = 0; i < project.metadata.GetCount(); i++)
			collect_reasons(project.metadata.GetKey(i));
	} else {
		collect_reasons(path);
		for(int i = 0; i < project.metadata.GetCount(); i++) {
			String k = project.metadata.GetKey(i);
			if(k.StartsWith(path + "/") || k.StartsWith(path + "\\"))
				collect_reasons(k);
		}
	}
	return out;
}

String OverviewGenerator::GetIssues(const String& path, const OverviewOptions& opt) {
	if(!opt.include_problems) return "";
	String out;
	auto collect_problems = [&](const String& p) {
		const FileMetadata* m = project.metadata.FindPtr(p);
		if(m && !m->problems.IsEmpty()) {
			bool header = false;
			for(const auto& prob : m->problems) {
				if(!prob.done) {
					if(!header) { out << p << ":\n"; header = true; }
					out << "  - " << prob.text << "\n";
				}
			}
		}
	};

	if(path.IsEmpty()) {
		for(int i = 0; i < project.metadata.GetCount(); i++) collect_problems(project.metadata.GetKey(i));
	} else {
		collect_problems(path);
		for(int i = 0; i < project.metadata.GetCount(); i++) {
			String k = project.metadata.GetKey(i);
			if(k.StartsWith(path + "/") || k.StartsWith(path + "\\")) collect_problems(k);
		}
	}
	return out;
}

String OverviewGenerator::GetTasks(const String& path, const OverviewOptions& opt) {
	if(!opt.include_tasks) return "";
	String out;
	auto collect_tasks = [&](const String& p) {
		const FileMetadata* m = project.metadata.FindPtr(p);
		if(m && !m->tasks.IsEmpty()) {
			bool header = false;
			for(const auto& t : m->tasks) {
				if(!t.done) {
					if(!header) { out << p << ":\n"; header = true; }
					out << "  - " << t.text << "\n";
				}
			}
		}
	};

	if(path.IsEmpty()) {
		for(int i = 0; i < project.metadata.GetCount(); i++) collect_tasks(project.metadata.GetKey(i));
	} else {
		collect_tasks(path);
		for(int i = 0; i < project.metadata.GetCount(); i++) {
			String k = project.metadata.GetKey(i);
			if(k.StartsWith(path + "/") || k.StartsWith(path + "\\")) collect_tasks(k);
		}
	}
	return out;
}

String OverviewGenerator::GetLeads(const String& path, const OverviewOptions& opt) {
	if(!opt.include_leads) return "";
	String out;
	auto collect_leads = [&](const String& p) {
		const FileMetadata* m = project.metadata.FindPtr(p);
		if(m && !m->leads.IsEmpty()) {
			bool header = false;
			for(const auto& l : m->leads) {
				if(!l.done) {
					if(!header) { out << p << ":\n"; header = true; }
					out << "  - " << l.text << "\n";
				}
			}
		}
	};

	if(path.IsEmpty()) {
		for(int i = 0; i < project.metadata.GetCount(); i++) collect_leads(project.metadata.GetKey(i));
	} else {
		collect_leads(path);
		for(int i = 0; i < project.metadata.GetCount(); i++) {
			String k = project.metadata.GetKey(i);
			if(k.StartsWith(path + "/") || k.StartsWith(path + "\\")) collect_leads(k);
		}
	}
	return out;
}

String OverviewGenerator::GetImportant(const String& path, const OverviewOptions& opt) {
	String out;
	VectorMap<String, EntryScore> actions = project.GetActionView(10);
	for(int i = 0; i < actions.GetCount(); i++) {
		if(path.IsEmpty() || actions.GetKey(i) == path || actions.GetKey(i).StartsWith(path + "/") || actions.GetKey(i).StartsWith(path + "\\")) {
			out << "- " << actions.GetKey(i) << " (Score: " << FormatDouble(actions[i].score, 1) << ")\n";
			out << "  Factors: " << Join(actions[i].factors, ", ") << "\n";
		}
	}
	return out;
}

String OverviewGenerator::GetReview(const String& path, const OverviewOptions& opt) {
	if(!opt.include_review_items) return "";
	String out;
	for(const auto& it : project.review_queue) {
		if(!it.dismissed && (path.IsEmpty() || it.path == path || it.path.StartsWith(path + "/") || it.path.StartsWith(path + "\\"))) {
			out << "- " << it.path << " [" << it.type << "]: " << it.message << " (Severity: " << it.severity << ")\n";
		}
	}
	return out;
}

String OverviewGenerator::GetPriorities(const String& path, const OverviewOptions& opt) {
	if(!opt.include_action_priorities) return "";
	return "";
}
