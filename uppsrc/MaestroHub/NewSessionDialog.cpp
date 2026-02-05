#include "NewSessionDialog.h"

NAMESPACE_UPP

NewSessionDialog::NewSessionDialog() {
	CtrlLayout(*this, "Start New Maestro Session");
	
	type.Add("work_task");
	type.Add("work_issue");
	type.Add("work_phase");
	type.Add("work_track");
	type.SetIndex(0);
	
	backend.Add("gemini");
	backend.Add("qwen");
	backend.Add("claude");
	backend.SetIndex(0);
	
	ok << THISBACK(OnOK);
	cancel << [=] { Break(IDCANCEL); };
}

void NewSessionDialog::OnOK() {
	if(purpose.GetData().ToString().IsEmpty()) {
		Exclamation("Please enter a purpose for the session.");
		return;
	}
	
	session_id = "ws-" + FormatIntHex(Random(), 8);
	// Logic to actually create session file would go here
	Break(IDOK);
}

END_UPP_NAMESPACE
