#ifndef _AI_Core_AiCompletion_h_
#define _AI_Core_AiCompletion_h_


NAMESPACE_UPP


struct AiCompletionComponent : Component {
	OmniThread omni;
	
	void Visit(Vis& v) override;
	String GetName() const override {return "AI: Completion";}
	
	AiCompletionComponent(MetaNode& owner);
	~AiCompletionComponent();
	
	static int GetKind() {return METAKIND_ECS_COMPONENT_AI_COMPLETION;}
	
};

INITIALIZE(AiCompletionComponent);


END_UPP_NAMESPACE


#endif
