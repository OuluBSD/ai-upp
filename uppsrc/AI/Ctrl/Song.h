#ifndef _AI_Ctrl_Song_h_
#define _AI_Ctrl_Song_h_

NAMESPACE_UPP


class CompInfoCtrl : public WithComponentInfo<ToolAppCtrl> {
	int focus_lyr = -1;
	
public:
	typedef CompInfoCtrl CLASSNAME;
	CompInfoCtrl();
	
	void Data() override;
	void Clear();
	void DataScript();
	void OnValueChange();
	void SetScript();
	void ToolMenu(Bar& bar) override;
	
	
};


END_UPP_NAMESPACE

#endif
