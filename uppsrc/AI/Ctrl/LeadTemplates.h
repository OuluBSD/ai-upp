#ifndef _AI_Ctrl_LeadTemplates_h_
#define _AI_Ctrl_LeadTemplates_h_

NAMESPACE_UPP


class LeadTemplateCtrl : public ToolAppCtrl {
	ArrayCtrl templates;
	
public:
	typedef LeadTemplateCtrl CLASSNAME;
	LeadTemplateCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	
	
};


END_UPP_NAMESPACE

#endif
