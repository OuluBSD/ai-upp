#include "MaestroHub.h"

NAMESPACE_UPP

IssueEditDialog::IssueEditDialog() {
	CtrlLayoutOKCancel(*this, "Edit Issue");
	Sizeable().Zoomable();
	
	severity.Add("blocker");
	severity.Add("critical");
	severity.Add("warning");
	severity.Add("info");
	
	issue_status.Add("open");
	issue_status.Add("analyzed");
	issue_status.Add("ignored");
	issue_status.Add("resolved");
	issue_status.Add("task");
}

void IssueEditDialog::SyncFromIssue(const MaestroIssue& src) {
	title.SetData(src.title);
	description.SetData(src.message);
	severity.SetData(src.severity);
	issue_status.SetData(src.state);
}

void IssueEditDialog::SyncToIssue(MaestroIssue& dst) const {
	dst.title = title.GetData();
	dst.message = description.GetData();
	dst.severity = severity.GetData();
	dst.state = issue_status.GetData();
	dst.modified_at = GetSysTime();
}

IssueCreateDialog::IssueCreateDialog() {
	CtrlLayoutOKCancel(*this, "Create New Issue");
	
	severity.Add("blocker");
	severity.Add("critical");
	severity.Add("warning");
	severity.Add("info");
	severity.SetIndex(2); // warning
}

MaestroIssue IssueCreateDialog::GetIssue() const {
	MaestroIssue iss;
	iss.issue_id = "iss-" + FormatIntHex(Random(), 8);
	iss.title = title.GetData();
	iss.message = description.GetData();
	iss.severity = severity.GetData();
	iss.state = "open";
	iss.created_at = iss.modified_at = GetSysTime();
	return iss;
}

ListSelectDialog::ListSelectDialog() {
	CtrlLayoutOKCancel(*this, "Select");
	Sizeable().Zoomable();
}

void ListSelectDialog::SetChoices(const Vector<String>& choices) {
	list.Clear();
	for(const auto& c : choices)
		list.Add(c);
	if(choices.GetCount())
		list.SetIndex(0);
}

bool ListSelectDialog::RunSelect(const char *title_text, const char *label_text, String& result) {
	Title(title_text);
	label.SetLabel(label_text);
	if(Run() != IDOK)
		return false;
	result = list.GetData();
	return !result.IsEmpty();
}

END_UPP_NAMESPACE
