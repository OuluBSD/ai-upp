#ifndef _AI_Ctrl_Ai_h_
#define _AI_Ctrl_Ai_h_


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


class AiChatComponentCtrl : public ComponentCtrl {
	ChatAiCtrl ctrl;
	
public:
	typedef AiChatComponentCtrl CLASSNAME;
	AiChatComponentCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override {}
	
	OmniThread& GetThread();
};

INITIALIZE(AiChatComponentCtrl)



END_UPP_NAMESPACE


#endif
