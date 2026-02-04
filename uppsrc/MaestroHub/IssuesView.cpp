#include "MaestroHub.h"

NAMESPACE_UPP

IssuesPane::IssuesPane() {
	Add(list.SizePos());
	
	list.AddColumn("ID");
	list.AddColumn("Severity");
	list.AddColumn("Status");
	list.AddColumn("Message");
	
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
	bar.Add("Resolve", THISBACK(OnResolve));
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

END_UPP_NAMESPACE
