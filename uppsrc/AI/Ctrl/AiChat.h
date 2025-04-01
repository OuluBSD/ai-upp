#ifndef _AI_Ctrl_AiChat_h_
#define _AI_Ctrl_AiChat_h_


NAMESPACE_UPP


class AiChatComponentCtrl : public WithChatAI<ComponentCtrl> {
	
public:
	typedef AiChatComponentCtrl CLASSNAME;
	AiChatComponentCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override {}
	
};

INITIALIZE(AiChatComponentCtrl)


END_UPP_NAMESPACE


#endif
