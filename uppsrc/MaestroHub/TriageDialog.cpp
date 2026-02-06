#include "MaestroHub.h"

NAMESPACE_UPP

bool CreateIssueTaskFile(const String& root, const MaestroIssue& iss, const String& title, String& task_path) {
	// Simple task creation logic
	String track_id = "maintenance";
	String phase_id = "issue_fixing";
	String task_id = iss.issue_id;
	
	String dir = AppendFileName(root, "docs/phases");
	RealizeDirectory(dir);
	
	task_path = AppendFileName(dir, task_id + ".md");
	
	String content;
	content << "# Task: " << title << "\n\n";
	content << "## Description\n" << iss.description << "\n\n";
	content << "## Message\n" << iss.message << "\n\n";
	content << "## Context\n";
	content << "- Issue ID: " << iss.issue_id << "\n";
	content << "- File: " << iss.file << "\n";
	content << "- Line: " << iss.line << "\n";
	
	return SaveFile(task_path, content);
}

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
	pending = pick(ism->ListIssues("", "", "open"));
	cursor = 0;
	UpdateUI();
}

void TriageDialog::UpdateUI() {
	if(cursor < 0 || cursor >= pending.GetCount()) {
		PromptOK("Triage complete!");
		Break(IDOK);
		return;
	}
	
	const MaestroIssue& iss = pending[cursor];
	issue_info.SetQTF(FormatIssueInfo(iss));
	ai_analysis.SetQTF(FormatAiSuggestion(iss));
	
	progress.Set(cursor, pending.GetCount());
	progress_text = Format("Issue %d of %d", cursor + 1, pending.GetCount());
}

String TriageDialog::FormatIssueInfo(const MaestroIssue& iss) const {
	String qtf;
	qtf << "[*@3 Issue " << DeQtf(iss.issue_id) << "]&";
	qtf << "[* Title:] " << DeQtf(iss.title) << "&";
	qtf << "[* Message:] " << DeQtf(iss.message) << "&";
	qtf << "[* Severity:] " << DeQtf(iss.severity) << "   [* State:] " << DeQtf(iss.state);
	return qtf;
}

String TriageDialog::FormatAiSuggestion(const MaestroIssue& iss) const {
	String qtf;
	if(!iss.analysis_summary.IsEmpty()) {
		qtf << "[* AI Analysis (Confidence " << iss.analysis_confidence << "%):]&";
		qtf << DeQtf(iss.analysis_summary);
	} else {
		qtf << "[* AI Analysis:]&No automatic analysis available.";
	}
	return qtf;
}

void TriageDialog::OnAccept() {
	if(cursor >= 0 && cursor < pending.GetCount()) {
		if(ism->Triage(pending[cursor].issue_id)) {
			Advance();
		}
	}
}

void TriageDialog::OnSkip() {
	Advance();
}

void TriageDialog::OnIgnore() {
	if(cursor >= 0 && cursor < pending.GetCount()) {
		MaestroIssue iss = pending[cursor];
		iss.state = "ignored";
		if(ism->SaveIssue(iss)) Advance();
	}
}

void TriageDialog::OnEdit() {
	if(cursor < 0 || cursor >= pending.GetCount()) return;
	IssueEditDialog dlg;
	MaestroIssue iss = pending[cursor];
	dlg.SyncFromIssue(iss);
	if(dlg.Run() == IDOK) {
		dlg.SyncToIssue(iss);
		if(ism->SaveIssue(iss)) {
			pending[cursor] = iss;
			UpdateUI();
		}
	}
}

void TriageDialog::OnCreateTask() {
	if(cursor < 0 || cursor >= pending.GetCount()) return;
	MaestroIssue iss = pending[cursor];
	String task_path;
	if(CreateIssueTaskFile(root, iss, "Fix " + iss.issue_id, task_path)) {
		iss.state = "task";
		ism->SaveIssue(iss);
		Advance();
	}
}

void TriageDialog::Advance() {
	cursor++;
	UpdateUI();
}

END_UPP_NAMESPACE
