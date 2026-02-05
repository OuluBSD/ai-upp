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

void IssueEditDialog::SyncFromIssue(const MaestroIssue& src) {
	title.SetData(src.title);
	message.SetData(src.message);
	file.SetData(src.file);
	line.SetData(src.line);
	severity.SetData(src.severity);
	state.SetData(src.state);
	priority.SetData(src.priority);
}

void IssueEditDialog::SyncToIssue(MaestroIssue& dst) {
	dst.title = title.GetData();
	dst.message = message.GetData();
	dst.file = file.GetData();
	dst.line = line.GetData();
	dst.severity = severity.GetData();
	dst.state = state.GetData();
	dst.priority = priority.GetData();
	dst.modified_at = GetSysTime();
}

void IssueEditDialog::LoadIssue(const MaestroIssue& src) {
	SyncFromIssue(src);
}

bool IssueEditDialog::RunEdit(MaestroIssue& dst) {
	LoadIssue(dst);
	if(Run() != IDOK)
		return false;
	SyncToIssue(dst);
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

END_UPP_NAMESPACE
