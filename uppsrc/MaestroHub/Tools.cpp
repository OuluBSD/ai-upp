#include "MaestroHub.h"

NAMESPACE_UPP

bool CreateIssueTaskFile(const String& root, const MaestroIssue& iss, const String& title, String& task_path) {
	// Logic to create a new .md task file based on the issue
	String tasks_dir = AppendFileName(AppendFileName(root, "uppsrc/AI/plan"), "issues");
	RealizeDirectory(tasks_dir);
	
	String filename = iss.issue_id + ".md";
	task_path = AppendFileName(tasks_dir, filename);
	
	String content;
	content << "# Task: " << title << "\n";
	content << "# Status: TODO\n\n";
	content << "## Objective\n";
	content << "Resolve issue " << iss.issue_id << ": " << iss.message << "\n\n";
	content << "## Requirements\n";
	content << "- Analyze file: " << iss.file << "\n";
	content << "- Fix root cause.\n";
	
	return SaveFile(task_path, content);
}

END_UPP_NAMESPACE

