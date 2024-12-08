#ifndef _AI_Ctrl_Beliefs_h_
#define _AI_Ctrl_Beliefs_h_

NAMESPACE_UPP


class SocialBeliefsCtrl : public ToolAppCtrl {
	Splitter hsplit;
	ArrayCtrl beliefs;
	ArrayCtrl attrs, user;
	WithSocialBelief<Ctrl> info;
	
public:
	typedef SocialBeliefsCtrl CLASSNAME;
	SocialBeliefsCtrl();
	
	void Data() override;
	void DataBelief();
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	void AddBelief();
	void RemoveBelief();
	void OnValueChange();
};


END_UPP_NAMESPACE

#endif
