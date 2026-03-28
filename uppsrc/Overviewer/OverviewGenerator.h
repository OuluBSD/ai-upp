#ifndef _Overviewer_OverviewGenerator_h_
#define _Overviewer_OverviewGenerator_h_

#include <Core/Core.h>

using namespace Upp;

struct OverviewerProject;

struct OverviewOptions {
	bool include_notes_summary = true;
	bool include_tags = true;
	bool include_problems = true;
	bool include_tasks = true;
	bool include_leads = true;
	bool include_review_items = true;
	bool include_suggestions = false;
	bool include_action_priorities = true;
	bool markdown_output = true;
	int max_entries = 50;
	int minimum_score_threshold = 10;

	void Jsonize(JsonIO& jio) {
		jio("include_notes_summary", include_notes_summary)
		   ("include_tags", include_tags)
		   ("include_problems", include_problems)
		   ("include_tasks", include_tasks)
		   ("include_leads", include_leads)
		   ("include_review_items", include_review_items)
		   ("include_suggestions", include_suggestions)
		   ("include_action_priorities", include_action_priorities)
		   ("markdown_output", markdown_output)
		   ("max_entries", max_entries)
		   ("minimum_score_threshold", minimum_score_threshold);
	}
};

class OverviewGenerator {
public:
	OverviewGenerator(const OverviewerProject& p) : project(p) {}

	String Generate(const String& rel_path, const OverviewOptions& opt);
	String GenerateProject(const OverviewOptions& opt);

private:
	const OverviewerProject& project;

	struct Section : Moveable<Section> {
		String title;
		String content;
	};

	void AddSection(Vector<Section>& sections, const String& title, const String& content);
	String Format(const Vector<Section>& sections, bool markdown);
	
	String GetSummary(const String& path, const OverviewOptions& opt);
	String GetWhy(const String& path, const OverviewOptions& opt);
	String GetIssues(const String& path, const OverviewOptions& opt);
	String GetTasks(const String& path, const OverviewOptions& opt);
	String GetLeads(const String& path, const OverviewOptions& opt);
	String GetImportant(const String& path, const OverviewOptions& opt);
	String GetAttribution(const String& path, const OverviewOptions& opt);
	String GetReview(const String& path, const OverviewOptions& opt);
	String GetPriorities(const String& path, const OverviewOptions& opt);
};

#endif
