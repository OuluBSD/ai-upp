#include "Maestro.h"

namespace Upp {

IssueManager::IssueManager(const String& maestro_root)
{
	base_path = NormalizePath(maestro_root);
	docs_issues_dir = AppendFileName(base_path, "docs/issues");
	json_issues_dir = AppendFileName(base_path, "docs/maestro/issues");
	
	RealizeDirectory(docs_issues_dir);
	RealizeDirectory(json_issues_dir);
}

Array<MaestroIssue> IssueManager::ListIssues(const String& type, const String& severity, const String& status)
{
	Array<MaestroIssue> list;
	
	// Scan JSON storage
	FindFile ff(AppendFileName(json_issues_dir, "*.json"));
	while(ff) {
		MaestroIssue& issue = list.Add();
		if(!LoadFromJsonFile(issue, ff.GetPath()))
			list.Drop();
		else {
			// Apply filters
			if(!type.IsEmpty() && issue.issue_type != type) list.Drop();
			else if(!severity.IsEmpty() && issue.severity != severity) list.Drop();
			else if(!status.IsEmpty() && issue.state != status) list.Drop();
		}
		ff.Next();
	}
	
	// Scan Legacy Markdown (limited support)
	ff.Search(AppendFileName(docs_issues_dir, "*.md"));
	while(ff) {
		String id = ff.GetName().Left(ff.GetName().GetCount() - 3);
		bool exists = false;
		for(const auto& i : list) if(i.issue_id == id) { exists = true; break; }
		
		if(!exists) {
			MaestroIssue& issue = list.Add();
			issue.issue_id = id;
			issue.title = id;
			issue.state = "open";
			issue.issue_type = "legacy";
		}
		ff.Next();
	}
	
	return list;
}

MaestroIssue IssueManager::LoadIssue(const String& id)
{
	MaestroIssue issue;
	if(!LoadFromJsonFile(issue, AppendFileName(json_issues_dir, id + ".json"))) {
		// Fallback to legacy path discovery if JSON missing
		issue.issue_id = id;
		issue.state = "open";
	}
	return issue;
}

bool IssueManager::SaveIssue(const MaestroIssue& issue)
{
	if(issue.issue_id.IsEmpty()) return false;
	return StoreAsJsonFile(issue, AppendFileName(json_issues_dir, issue.issue_id + ".json"), true);
}

bool IssueManager::DeleteIssue(const String& id)
{
	String path = AppendFileName(json_issues_dir, id + ".json");
	if(FileExists(path)) return DeleteFile(path);
	return false;
}

String IssueManager::CreateFromLogFinding(const ValueMap& finding, const String& scan_id)
{
	MaestroIssue issue;
	issue.issue_id = "iss-" + FormatIntHex(Random(), 8);
	issue.message = AsString(finding["message"]);
	issue.severity = AsString(finding["severity"]);
	issue.file = AsString(finding["file"]);
	issue.line = finding["line"];
	issue.created_at = GetSysTime();
	issue.state = "open";
	
	if(SaveIssue(issue)) return issue.issue_id;
	return "";
}

bool IssueManager::Triage(const String& id, bool auto_mode)
{
	MaestroIssue issue = LoadIssue(id);
	if(issue.issue_id.IsEmpty()) return false;
	
	// AI triage logic placeholder
	issue.state = "analyzed";
	issue.priority = 10;
	return SaveIssue(issue);
}

}
