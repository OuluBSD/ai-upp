#ifndef _Overviewer_OverviewGenerator_h_
#define _Overviewer_OverviewGenerator_h_

#include <Core/Core.h>

using namespace Upp;

struct OverviewOptions {
	bool markdown_output = true;
	bool include_notes_summary = true;
	bool include_problems = true;
	bool include_tasks = true;
	bool include_leads = true;
	bool include_review_items = true;
	bool include_action_priorities = true;
	bool include_git_summary = true;
	bool include_recent_commits = true;
	bool include_attribution_summary = true;
	bool include_decisions = true;
	bool include_insights = true;
	bool include_comments = true;

	void Jsonize(JsonIO& jio) {
		jio("markdown_output", markdown_output)("include_notes_summary", include_notes_summary)
		   ("include_problems", include_problems)("include_tasks", include_tasks)
		   ("include_leads", include_leads)("include_review_items", include_review_items)
		   ("include_action_priorities", include_action_priorities)
		   ("include_git_summary", include_git_summary)("include_recent_commits", include_recent_commits)
		   ("include_attribution_summary", include_attribution_summary)
		   ("include_decisions", include_decisions)("include_insights", include_insights)
		   ("include_comments", include_comments);
	}
};

struct OverviewerProject;

class OverviewGenerator {
public:
	struct Section : Moveable<Section> {
		String title;
		String content;
	};

	OverviewGenerator(const OverviewerProject& p) : project(p) {}

	String Generate(const String& rel_path, const OverviewOptions& opt);
	String GenerateProject(const OverviewOptions& opt);

private:
	const OverviewerProject& project;

	void AddSection(Vector<Section>& sections, const String& title, const String& content);
	String Format(const Vector<Section>& sections, bool markdown);

	String GetSummary(const String& path, const OverviewOptions& opt);
	String GetWhy(const String& path, const OverviewOptions& opt);
	String GetIssues(const String& path, const OverviewOptions& opt);
	String GetTasks(const String& path, const OverviewOptions& opt);
	String GetLeads(const String& path, const OverviewOptions& opt);
	String GetImportant(const String& path, const OverviewOptions& opt);
	String GetAttribution(const String& path, const OverviewOptions& opt);
	String GetDecisions(const String& path, const OverviewOptions& opt);
	String GetInsights(const String& path, const OverviewOptions& opt);
	String GetComments(const String& path, const OverviewOptions& opt);
	String GetReview(const String& path, const OverviewOptions& opt);
	String GetPriorities(const String& path, const OverviewOptions& opt);
};

#endif
