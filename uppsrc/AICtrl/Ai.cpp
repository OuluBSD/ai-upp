#include "AICtrl.h"


NAMESPACE_UPP

AiCompletionComponentCtrl::AiCompletionComponentCtrl() {
	Add(ctrl.SizePos());
	
}

void AiCompletionComponentCtrl::Data() {
	TODO //ctrl.SetThread(GetThread());
}


INITIALIZER_COMPONENT_CTRL(AiCompletionComponent, AiCompletionComponentCtrl)




AiChatComponentCtrl::AiChatComponentCtrl() {
	Add(ctrl.SizePos());
	
}

void AiChatComponentCtrl::Data() {
	TODO //ctrl.SetThread(GetThread());
}


INITIALIZER_COMPONENT_CTRL(AiChatComponent, AiChatComponentCtrl)

END_UPP_NAMESPACE
