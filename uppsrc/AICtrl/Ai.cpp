#include "AICtrl.h"


NAMESPACE_UPP

AiCompletionComponentCtrl::AiCompletionComponentCtrl() {
	Add(ctrl.SizePos());
	
}

OmniThread& AiCompletionComponentCtrl::GetThread() {
	return GetExt<AiCompletionComponent>().omni;
}

void AiCompletionComponentCtrl::Data() {
	ctrl.SetThread(GetThread());
}


INITIALIZER_COMPONENT_CTRL(AiCompletionComponent, AiCompletionComponentCtrl)




AiChatComponentCtrl::AiChatComponentCtrl() {
	Add(ctrl.SizePos());
	
}

OmniThread& AiChatComponentCtrl::GetThread() {
	return GetExt<AiChatComponent>().omni;
}

void AiChatComponentCtrl::Data() {
	ctrl.SetThread(GetThread());
}


INITIALIZER_COMPONENT_CTRL(AiChatComponent, AiChatComponentCtrl)

END_UPP_NAMESPACE
