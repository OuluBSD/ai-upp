#include "Core.h"

NAMESPACE_UPP


AiChatComponent::AiChatComponent(MetaNode& owner) : Component(owner) {
	
}

AiChatComponent::~AiChatComponent() {
	
}

void AiChatComponent::Visit(NodeVisitor& v) {
	v.Ver(0);
}

INITIALIZER_COMPONENT(AiChatComponent);
INITIALIZER_COMPONENT(AiStageExample);


END_UPP_NAMESPACE
