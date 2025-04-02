#include "Ctrl.h"


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

END_UPP_NAMESPACE
