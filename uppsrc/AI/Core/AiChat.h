#ifndef _AI_Core_AiChat_h_
#define _AI_Core_AiChat_h_


NAMESPACE_UPP


struct AiChatComponent : Component {
	void Visit(NodeVisitor& v) override;
	String GetName() const override {return "AI: Chat";}
	
	AiChatComponent(MetaNode& owner);
	~AiChatComponent();
	
	static int GetKind() {return METAKIND_ECS_COMPONENT_AI_CHAT;}
	
};

INITIALIZE(AiChatComponent);


END_UPP_NAMESPACE


#endif
