#ifndef _Overviewer_McpServer_h_
#define _Overviewer_McpServer_h_

#include <Core/Core.h>
#include "Overviewer.h"

using namespace Upp;

class McpServer {
public:
	McpServer(OverviewerProject& p) : project(p) {}

	void Run();

private:
	OverviewerProject& project;
	Vector<String> current_scan;

	Json ValorizeResponse(bool success, const Value& result, const String& error = "");
	void ProcessRequest(const String& line);

	// Operations
	Value OpenProject(const Value& args);
	Value SaveProject(const Value& args);
	Value GetProjectInfo(const Value& args);
	Value ListEntries(const Value& args);
	Value GetEntry(const Value& args);
	Value SetNote(const Value& args);
	Value SetNumeric(const Value& args);
	Value SetFlags(const Value& args);
	Value AddTag(const Value& args);
	Value RemoveTag(const Value& args);
	Value AddListItem(const Value& args);
	Value UpdateListItem(const Value& args);
	Value RemoveListItem(const Value& args);
	Value MoveEntry(const Value& args);
	Value RefreshScan(const Value& args);
	Value FindEntriesWithFlag(const Value& args);
	Value FindEntriesMissingNumeric(const Value& args);
	Value FindEntriesByTag(const Value& args);
	Value GetRegistryTags(const Value& args);
	Value GetRecoveryInfo(const Value& args);
	Value WriteBackupNow(const Value& args);
	Value GenerateSuggestions(const Value& args);
	Value ApplySuggestion(const Value& args);
	Value RejectSuggestion(const Value& args);
	Value GetDashboard(const Value& args);
	Value RunConsistencyCheck(const Value& args);
	Value ListReviewItems(const Value& args);
	Value DismissReviewItem(const Value& args);
	Value GetHistory(const Value& args);
	Value GetRecentChanges(const Value& args);
	Value ClearHistory(const Value& args);
	Value GetActionView(const Value& args);
	Value GetEntryScore(const Value& args);
	Value GenerateOverview(const Value& args);
	Value ExportOverview(const Value& args);
	Value GetGitInfo(const Value& args);
	Value RefreshGitStatus(const Value& args);
	Value GetEntryGitStatus(const Value& args);
	Value GetEntryCommits(const Value& args);
	Value LinkListItemCommit(const Value& args);
	Value GetSessions(const Value& args);
	Value GetHistoryByActor(const Value& args);
	Value GetActorSummary(const Value& args);
	Value CreateScenario(const Value& args);
	Value ActivateScenario(const Value& args);
	Value DeactivateScenario(const Value& args);
	Value ListScenarios(const Value& args);
	Value DeleteScenario(const Value& args);
	Value CompareScenario(const Value& args);
	Value ApplyScenario(const Value& args);
	Value CreateDecision(const Value& args);
	Value UpdateDecision(const Value& args);
	Value SetDecisionStatus(const Value& args);
	Value ListDecisions(const Value& args);
	Value GetDecision(const Value& args);
	Value LinkDecisionToEntry(const Value& args);
	Value LinkDecisionToScenario(const Value& args);
	Value AddComment(const Value& args);
	Value ListComments(const Value& args);
	Value GetCommentsForEntry(const Value& args);
	Value GetCommentsForDecision(const Value& args);

	// Helpers
	void DoScan();
	bool IsPathValid(const String& path);
	String GetAbsPath(const String& rel_path);
	uint32 StringToFlag(const String& name);
};

#endif
