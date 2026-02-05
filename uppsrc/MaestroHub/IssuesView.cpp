#include "MaestroHub.h"
#include "IssueDialogs.h"

NAMESPACE_UPP

IssuesPane::IssuesPane() {
	Add(list.SizePos());
	
	list.AddColumn("ID");
	list.AddColumn("Severity");
	list.AddColumn("Status");
	list.AddColumn("Message");
	
	list.MultiSelect();
	list.WhenBar = THISBACK(OnMenu);
}

void IssuesPane::Load(const String& root) {
	current_root = root;
	list.Clear();
	
	IssueManager ism(root);
	Array<MaestroIssue> iss_list = ism.ListIssues();
	for(const auto& iss : iss_list) {
		list.Add(iss.issue_id, iss.severity, iss.state, iss.message.IsEmpty() ? iss.title : iss.message);
	}
}

void IssuesPane::OnMenu(Bar& bar) {
	if(!list.IsCursor()) return;
	bar.Add("Triage", THISBACK(OnTriage));
	bar.Add("Edit...", THISBACK(OnEdit));
	bar.Add("Create Task...", THISBACK(OnCreateTask));
	bar.Separator();
	bar.Add("Resolve", THISBACK(OnResolve));
	bar.Add("Ignore", THISBACK(OnIgnore));
	bar.Separator();
	bar.Add("Bulk Set Status...", THISBACK(BulkSetStatus));
	bar.Add("Bulk Set Severity...", THISBACK(BulkSetSeverity));
}

void IssuesPane::OnTriage() {
	if(!list.IsCursor()) return;
	String id = list.Get(0);
	IssueManager ism(current_root);
	if(ism.Triage(id)) {
		Load(current_root);
	}
}

void IssuesPane::OnResolve() {
	if(!list.IsCursor()) return;
	String id = list.Get(0);
	IssueManager ism(current_root);
	MaestroIssue iss = ism.LoadIssue(id);
	iss.state = "resolved";
	if(ism.SaveIssue(iss)) {
		Load(current_root);
	}
}

void IssuesPane::OnEdit() {
	if(!list.IsCursor()) return;
	String id = list.Get(0);
	IssueManager ism(current_root);
	MaestroIssue iss = ism.LoadIssue(id);
	IssueEditDialog dlg;
	if(dlg.RunEdit(iss) && ism.SaveIssue(iss)) {
		Load(current_root);
	}
}

void IssuesPane::OnIgnore() {
	if(!list.IsCursor()) return;
	String id = list.Get(0);
	IssueManager ism(current_root);
	MaestroIssue iss = ism.LoadIssue(id);
	iss.state = "ignored";
	if(ism.SaveIssue(iss)) {
		Load(current_root);
	}
}

void IssuesPane::OnCreateTask() {
	if(!list.IsCursor()) return;
	String id = list.Get(0);
	IssueManager ism(current_root);
	MaestroIssue iss = ism.LoadIssue(id);
	String task_title = "Fix issue " + iss.issue_id;
	String task_path;
	if(CreateIssueTaskFile(current_root, iss, task_title, task_path)) {
		iss.decision = "create_task";
		iss.linked_tasks.Add(task_path);
		ism.SaveIssue(iss);
		PromptOK("Task created: " + task_path);
	} else {
		PromptOK("Failed to create task file.");
	}
}

Vector<String> IssuesPane::GetSelectedIssueIds() const {
	Vector<String> ids;
	if(list.GetCount() == 0)
		return ids;
	for(int i = 0; i < list.GetCount(); i++) {
		if(list.IsSelected(i))
			ids.Add(AsString(list.Get(i, 0)));
	}
	if(ids.IsEmpty() && list.IsCursor())
		ids.Add(AsString(list.Get(0)));
	return ids;
}

void IssuesPane::BulkSetStatus() {
	Vector<String> ids = GetSelectedIssueIds();
	if(ids.IsEmpty()) return;
	
	ListSelectDialog dlg;
	Vector<String> choices;
	choices << "open" << "analyzed" << "ignored" << "resolved" << "task";
	dlg.SetChoices(choices);
	String result;
	if(!dlg.RunSelect("Bulk Status", "Select new status:", result))
		return;
	
	IssueManager ism(current_root);
	for(const auto& id : ids) {
		MaestroIssue iss = ism.LoadIssue(id);
		iss.state = result;
		ism.SaveIssue(iss);
	}
	Load(current_root);
}

void IssuesPane::BulkSetSeverity() {
	Vector<String> ids = GetSelectedIssueIds();
	if(ids.IsEmpty()) return;
	
	ListSelectDialog dlg;
	Vector<String> choices;
	choices << "blocker" << "critical" << "warning" << "info";
	dlg.SetChoices(choices);
	String result;
	if(!dlg.RunSelect("Bulk Severity", "Select new severity:", result))
		return;
	
	IssueManager ism(current_root);
	for(const auto& id : ids) {
		MaestroIssue iss = ism.LoadIssue(id);
		iss.severity = result;
		ism.SaveIssue(iss);
	}
	Load(current_root);
}

END_UPP_NAMESPACE
