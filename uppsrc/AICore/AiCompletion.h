#ifndef _AI_Core_AiCompletion_h_
#define _AI_Core_AiCompletion_h_





struct AiCompletionComponent : Component {
	OmniThread omni;
	
	void Visit(Vis& v) override;
	String GetName() const override {return "AI: Completion";}
	
	CLASSTYPE(AiCompletionComponent)
	AiCompletionComponent(VfsValue& owner);
	~AiCompletionComponent();
	
	static int GetKind() {return METAKIND_ECS_COMPONENT_AI_COMPLETION;}
	
};

INITIALIZE(AiCompletionComponent);





#endif
