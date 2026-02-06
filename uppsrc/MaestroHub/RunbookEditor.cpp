#include "MaestroHub.h"

NAMESPACE_UPP

RunbookEditor::RunbookEditor() {
	CtrlLayoutOKCancel(*this, "Runbook Editor");
	Sizeable().Zoomable();
}

void RunbookEditor::New(const String& root) {
	steps.Clear();
}

END_UPP_NAMESPACE
