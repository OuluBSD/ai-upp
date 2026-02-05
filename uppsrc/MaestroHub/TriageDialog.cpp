#include "TriageDialog.h"
#include "IssueDialogs.h"

NAMESPACE_UPP

TriageDialog::TriageDialog() {
	CtrlLayout(*this, "Issue Triage Wizard");
	
	btn_accept << THISBACK(OnAccept);
	btn_skip << THISBACK(OnSkip);
	btn_ignore << THISBACK(OnIgnore);
	btn_edit << THISBACK(OnEdit);
	btn_create_task << THISBACK(OnCreateTask);
}

void TriageDialog::Load(const String& maestro_root) {
	root = maestro_root;
	ism.Create(root);
	pending = ism->ListIssues("", "", "open");
	cursor = 0;
	UpdateUI();
}

String TriageDialog::FormatIssueInfo(const MaestroIssue& iss) const {
	String qtf;
	qtf << "[*@3 Issue " << DeQtf(iss.issue_id) << "]&";
	if(!iss.title.IsEmpty())
		qtf << "[* Title:] " << DeQtf(iss.title) << "&";
	if(!iss.message.IsEmpty())
		qtf << "[* Message:] " << DeQtf(iss.message) << "&";
	if(!iss.file.IsEmpty()) {
		qtf << "[* File:] " << DeQtf(iss.file);
		if(iss.line > 0)
			qtf << ":" << iss.line;
		qtf << "&";
	}
	qtf << "[* Severity:] " << DeQtf(iss.severity) << "   [* State:] " << DeQtf(iss.state) << "   [* Priority:] " << iss.priority;
	return qtf;
}

String TriageDialog::FormatAiSuggestion(const MaestroIssue& iss) const {
	String qtf;
	qtf << "[* AI Suggestion]&";
	if(!iss.analysis_summary.IsEmpty())
		qtf << DeQtf(iss.analysis_summary) << "&";
	else
		qtf << "No AI analysis available.&";
	if(iss.analysis_confidence > 0)
		qtf << "[* Confidence:] " << iss.analysis_confidence << "%&";
	if(!iss.decision.IsEmpty())
		qtf << "[* Decision:] " << DeQtf(iss.decision) << "&";
	if(iss.solutions.GetCount()) {
		qtf << "[* Possible Fixes:]&";
		for(const auto& s : iss.solutions)
			qtf << " - " << DeQtf(s) << "&";
	}
	return qtf;
}

void TriageDialog::UpdateUI() {
	if(cursor < 0 || cursor >= pending.GetCount()) {
		PromptOK("Triage complete!");
		Close();
		return;
	}
	
	const MaestroIssue& iss = pending[cursor];
	issue_info.SetQTF(FormatIssueInfo(iss));
	ai_analysis.SetQTF(FormatAiSuggestion(iss));
	
	progress.Set(cursor, pending.GetCount());
	progress_text = Format("Issue %d of %d", cursor + 1, pending.GetCount());
}

void TriageDialog::Advance() {
	cursor++;
	UpdateUI();
}

void TriageDialog::OnAccept() {
	if(cursor >= 0 && cursor < pending.GetCount()) {
		MaestroIssue iss = ism->LoadIssue(pending[cursor].issue_id);
		iss.decision = "accept_ai";
		ism->SaveIssue(iss);
		ism->Triage(pending[cursor].issue_id);
		Advance();
	}
}

void TriageDialog::OnSkip() {
	Advance();
}

void TriageDialog::OnIgnore() {
	if(cursor >= 0 && cursor < pending.GetCount()) {
		MaestroIssue iss = clone(pending[cursor]);
		iss.state = "ignored";
		ism->SaveIssue(iss);
		Advance();
	}
}

void TriageDialog::OnEdit() {
	if(cursor < 0 || cursor >= pending.GetCount()) return;
	MaestroIssue iss = ism->LoadIssue(pending[cursor].issue_id);
	IssueEditDialog dlg;
	if(dlg.RunEdit(iss) && ism->SaveIssue(iss)) {
		pending = ism->ListIssues("", "", "open");
		if(cursor >= pending.GetCount())
			cursor = max(0, pending.GetCount() - 1);
		UpdateUI();
	}
}

void TriageDialog::OnCreateTask() {
	if(cursor < 0 || cursor >= pending.GetCount()) return;
	MaestroIssue iss = ism->LoadIssue(pending[cursor].issue_id);
	String task_title = "Fix issue " + iss.issue_id;
	String task_path;
	if(CreateIssueTaskFile(root, iss, task_title, task_path)) {
		iss.decision = "create_task";
		iss.linked_tasks.Add(task_path);
		ism->SaveIssue(iss);
		PromptOK("Task created: " + task_path);
	} else {
		PromptOK("Failed to create task file.");
	}
}

END_UPP_NAMESPACE
