#ifndef _AI_Ctrl_AiCompletion_h_
#define _AI_Ctrl_AiCompletion_h_


NAMESPACE_UPP


class AiCompletionComponentCtrl : public ComponentCtrl {
	CompletionCtrl ctrl;
	
public:
	typedef AiCompletionComponentCtrl CLASSNAME;
	AiCompletionComponentCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override {}
	
	OmniThread& GetThread();
};

INITIALIZE(AiCompletionComponentCtrl)


END_UPP_NAMESPACE


#endif
