#include "AICore.h"

NAMESPACE_UPP


AiCompletionComponent::AiCompletionComponent(VfsValue& owner) : Component(owner) {
	
}

AiCompletionComponent::~AiCompletionComponent() {
	
}

void AiCompletionComponent::Visit(Vis& v) {
	v.Ver(0);
}

INITIALIZER_COMPONENT(AiCompletionComponent);


END_UPP_NAMESPACE
