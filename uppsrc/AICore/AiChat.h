#ifndef _AI_Core_AiChat_h_
#define _AI_Core_AiChat_h_





struct AiChatComponent : Component {
	OmniThread omni;
	
	void Visit(Vis& v) override;
	String GetName() const override {return "AI: Chat";}
	
	CLASSTYPE(AiChatComponent)
	AiChatComponent(MetaNode& owner);
	~AiChatComponent();
	
	static int GetKind() {return METAKIND_ECS_COMPONENT_AI_CHAT;}
	
};

INITIALIZE(AiChatComponent);



INITIALIZE_VALUECOMPONENT(AiStageExample, METAKIND_ECS_COMPONENT_AI_STAGE_EXAMPLE);





#endif
