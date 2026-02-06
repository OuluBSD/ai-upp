#include "MaestroHub.h"

NAMESPACE_UPP

IssuesPane::IssuesPane() {
	Add(main_split.SizePos());
	
	issues.AddColumn("ID", 80);
	issues.AddColumn("Title", 300);
	issues.AddColumn("Status", 100);
	issues.AddColumn("Severity", 100);
	issues.WhenBar = THISBACK(OnMenu);
	issues.WhenLeftDouble = THISBACK(OnEdit);
	
	main_split.Vert(issues, detail);
}

void IssuesPane::Load(const String& root) {
	current_root = root;
	IssueManager ism(root);
	Array<MaestroIssue> list = pick(ism.ListIssues("", "", ""));
	issues.Clear();
	for(const auto& i : list)
		issues.Add(i.issue_id, i.title, i.state, i.severity);
		
	if(issues.GetCount() > 0) issues.SetCursor(0);
}

void IssuesPane::OnMenu(Bar& bar) {
	bar.Add("Triage Wizard...", THISBACK(OnTriage));
	bar.Add("Create Issue...", THISBACK(OnResolve)); 
	bar.Separator();
	bar.Add("Edit...", THISBACK(OnEdit));
	bar.Add("Mark Resolved", THISBACK(OnResolve));
	bar.Separator();
	bar.Add("Bulk Status...", THISBACK(OnBulkStatus));
	bar.Add("Bulk Severity...", THISBACK(OnBulkSeverity));
}

void IssuesPane::OnTriage() {
	TriageDialog dlg;
	dlg.Load(current_root);
	if(dlg.Run() == IDOK)
		Load(current_root);
}

void IssuesPane::OnResolve() {
	if(!issues.IsCursor()) return;
	String id = issues.Get(0);
	IssueManager ism(current_root);
	MaestroIssue iss = ism.LoadIssue(id);
	iss.state = "resolved";
	ism.SaveIssue(iss);
	Load(current_root);
}

void IssuesPane::OnEdit() {
	if(!issues.IsCursor()) return;
	String id = issues.Get(0);
	IssueManager ism(current_root);
	MaestroIssue iss = ism.LoadIssue(id);
	
	IssueEditDialog dlg;
	dlg.SyncFromIssue(iss);
	if(dlg.Run() == IDOK) {
		dlg.SyncToIssue(iss);
		ism.SaveIssue(iss);
		Load(current_root);
	}
}

void IssuesPane::OnBulkStatus() {
	PromptOK("Bulk Status (Stub)");
}

void IssuesPane::OnBulkSeverity() {
	PromptOK("Bulk Severity (Stub)");
}

END_UPP_NAMESPACE