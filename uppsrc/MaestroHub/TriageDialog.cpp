#include "TriageDialog.h"

NAMESPACE_UPP

TriageDialog::TriageDialog() {
	CtrlLayout(*this, "Issue Triage Wizard");
	
	btn_accept << THISBACK(OnAccept);
	btn_skip << THISBACK(OnSkip);
	btn_ignore << THISBACK(OnIgnore);
}

void TriageDialog::Load(const String& maestro_root) {
	root = maestro_root;
	ism.Create(root);
	pending = ism->ListIssues("", "", "open");
	cursor = 0;
	UpdateUI();
}

void TriageDialog::UpdateUI() {
	if(cursor < 0 || cursor >= pending.GetCount()) {
		PromptOK("Triage complete!");
		Close();
		return;
	}
	
	const MaestroIssue& iss = pending[cursor];
	issue_info.SetQTF("[*@3 Issue " + DeQtf(iss.issue_id) + "]&[* Message:] " + DeQtf(iss.message) + "&[* File:] " + DeQtf(iss.file));
	
	// AI Analysis Placeholder
	ai_analysis.SetQTF("[* AI Suggestion:]&Severity: [! critical]&Reason: Potential memory leak in constructor.&Action: Create fix task.");
	
	progress.Set(cursor, pending.GetCount());
	progress_text = Format("Issue %d of %d", cursor + 1, pending.GetCount());
}

void TriageDialog::OnAccept() {
	if(cursor >= 0 && cursor < pending.GetCount()) {
		ism->Triage(pending[cursor].issue_id);
		cursor++;
		UpdateUI();
	}
}

void TriageDialog::OnSkip() {
	cursor++;
	UpdateUI();
}

void TriageDialog::OnIgnore() {
	if(cursor >= 0 && cursor < pending.GetCount()) {
		MaestroIssue iss = clone(pending[cursor]);
		iss.state = "ignored";
		ism->SaveIssue(iss);
		cursor++;
		UpdateUI();
	}
}

END_UPP_NAMESPACE