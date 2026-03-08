#include "MaestroHub.h"

NAMESPACE_UPP

TutorialPane::TutorialPane() {
	Add(view.SizePos());
	UpdateContent();
}

void TutorialPane::UpdateContent() {
	String qtf;
	qtf << "[&@6 [* MaestroHub Cockpit Tutorial - Step " << step + 1 << "]]&";
	
	switch(step) {
	case 0:
		qtf << "[C1 Welcome! The **Fleet Dashboard** (center) is your mission control. It shows active projects and automation queues.]";
		break;
	case 1:
		qtf << "[C1 The **Workspace** tab (left) shows your physical repository. The **Pipeline** tab shows current scan/transformation stages.]";
		break;
	case 2:
		qtf << "[C1 Use the **AI Assistant** (right) for natural language interaction. It automatically tracks your current task context.]";
		break;
	case 3:
		qtf << "[C1 The **Evidence Locker** and **Playbook Manager** help you manage verification artifacts and expert strategies.]";
		break;
	default:
		qtf << "[C1 Tutorial complete. You can restart it anytime from the Help menu.]";
		break;
	}
	
	view.SetQTF(qtf);
}

void TutorialPane::OnNext() {
	step++;
	UpdateContent();
}

void TutorialPane::OnPrev() {
	if(step > 0) step--;
	UpdateContent();
}

WelcomeDialog::WelcomeDialog() {
	CtrlLayoutOKCancel(*this, "Welcome");
	content.SetQTF("[&@6 [* Welcome to the Future of Orchestrated Engineering.]]&"
	               "[C1 MaestroHub Cockpit provides a centralized interface for AI-augmented code transformation and repository management.]&"
	               "[C1 This environment is optimized for high-density information display and expert-level control.]");
}

END_UPP_NAMESPACE