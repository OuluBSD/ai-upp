#include "Ctrl.h"


NAMESPACE_UPP

AiChatComponentCtrl::AiChatComponentCtrl() {
	CtrlLayout(*this);
	
	run <<= THISBACK(Run);
	
}

OmniThread& AiChatComponentCtrl::GetThread() {
	return GetExt<AiChatComponent>().omni;
}

void AiChatComponentCtrl::Data() {
	
}

void AiChatComponentCtrl::Run() {
	OmniThread& omni = GetThread();
	ChatThread& chat = omni;
	
	String txt = msg.GetData();
	if (txt.IsEmpty())
		return;
	
	TaskMgr& m = AiTaskManager();
	
}

INITIALIZER_COMPONENT_CTRL(AiChatComponent, AiChatComponentCtrl)

END_UPP_NAMESPACE
