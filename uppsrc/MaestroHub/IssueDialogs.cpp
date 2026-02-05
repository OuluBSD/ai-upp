#include "IssueDialogs.h"

NAMESPACE_UPP

IssueEditDialog::IssueEditDialog() {
	CtrlLayoutOKCancel(*this, "Edit Issue");
	Sizeable().Zoomable();
	
	severity.Add("blocker");
	severity.Add("critical");
	severity.Add("warning");
	severity.Add("info");
	
	state.Add("open");
	state.Add("analyzed");
	state.Add("ignored");
	state.Add("resolved");
	state.Add("task");
}

void IssueEditDialog::SyncFromIssue() {
	title.SetData(issue.title);
	message.SetData(issue.message);
	file.SetData(issue.file);
	line.SetData(issue.line);
	severity.SetData(issue.severity);
	state.SetData(issue.state);
	priority.SetData(issue.priority);
}

void IssueEditDialog::SyncToIssue() {
	issue.title = title.GetData();
	issue.message = message.GetData();
	issue.file = file.GetData();
	issue.line = line.GetData();
	issue.severity = severity.GetData();
	issue.state = state.GetData();
	issue.priority = priority.GetData();
	issue.modified_at = GetSysTime();
}

void IssueEditDialog::LoadIssue(const MaestroIssue& src) {
	issue = clone(src);
	SyncFromIssue();
}

bool IssueEditDialog::RunEdit(MaestroIssue& dst) {
	LoadIssue(dst);
	if(Run() != IDOK)
		return false;
	SyncToIssue();
	dst = clone(issue);
	return true;
}

ListSelectDialog::ListSelectDialog() {
	CtrlLayoutOKCancel(*this, "Select");
	Sizeable().Zoomable();
}

void ListSelectDialog::SetChoices(const Vector<String>& choices) {
	options.Clear();
	for(const auto& c : choices)
		options.Add(c);
	if(choices.GetCount())
		options.SetIndex(0);
}

bool ListSelectDialog::RunSelect(const String& title_text, const String& prompt_text, String& result) {
	Title(title_text);
	prompt.SetText(prompt_text);
	if(Run() != IDOK)
		return false;
	result = options.GetData();
	return !result.IsEmpty();
}

bool CreateIssueTaskFile(const String& root, const MaestroIssue& issue, const String& title, String& out_path) {
	String tasks_dir = AppendFileName(root, "docs/tasks");
	RealizeDirectory(tasks_dir);
	String safe_id = issue.issue_id.IsEmpty() ? FormatIntHex(Random(), 8) : issue.issue_id;
	String filename = "issue_" + safe_id + ".md";
	out_path = AppendFileName(tasks_dir, filename);
	
	String content;
	content << "# Task: " << (title.IsEmpty() ? ("Fix issue " + safe_id) : title) << "\n";
	content << "# Status: TODO\n\n";
	content << "## Objective\n";
	if(!issue.title.IsEmpty())
		content << "- Title: " << issue.title << "\n";
	if(!issue.message.IsEmpty())
		content << "- Message: " << issue.message << "\n";
	if(!issue.file.IsEmpty()) {
		content << "- File: " << issue.file;
		if(issue.line > 0)
			content << ":" << issue.line;
		content << "\n";
	}
	if(!issue.description.IsEmpty())
		content << "\n" << issue.description << "\n";
	content << "\n## References\n";
	content << "- Issue: " << safe_id << "\n";
	
	return SaveFile(out_path, content);
}

END_UPP_NAMESPACE
