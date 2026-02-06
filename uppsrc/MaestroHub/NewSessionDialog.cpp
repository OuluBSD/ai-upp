#include "MaestroHub.h"

NAMESPACE_UPP

NewSessionDialog::NewSessionDialog() {
	CtrlLayoutOKCancel(*this, "Start New Maestro Session");
	
	type.Add("work_task");
	type.Add("work_issue");
	type.Add("work_phase");
	type.Add("work_track");
	type.SetIndex(0);
	
	backend.Add("gemini");
	backend.Add("qwen");
	backend.Add("claude");
	backend.SetIndex(0);
}

END_UPP_NAMESPACE