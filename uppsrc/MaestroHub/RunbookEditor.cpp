#include "MaestroHub.h"

NAMESPACE_UPP

RunbookEditor::RunbookEditor() {
	CtrlLayoutOKCancel(*this, "Runbook Editor");
	Sizeable().Zoomable();
	
	steps.AddColumn("Action");
	steps.AddColumn("Command");
	steps.WhenLeftDouble = THISBACK(OnEditStep);
	
	// add_step << THISBACK(OnEditStep); // Assuming add_step button exists
}

void RunbookEditor::New(const String& root) {
	steps.Clear();
}

void RunbookEditor::OnEditStep() {
	StepWizard wiz;
	wiz.WhenAssist = WhenAssist;
	
	if(wiz.Run() == IDOK) {
		// Stub: Add/Update step
		RunbookStep s = wiz.GetStep();
		steps.Add(s.action, s.command);
	}
}

END_UPP_NAMESPACE