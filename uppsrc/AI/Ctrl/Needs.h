#ifndef _AI_Ctrl_Needs_h_
#define _AI_Ctrl_Needs_h_

NAMESPACE_UPP


class SocialNeedsCtrl : public ToolAppCtrl {
	Splitter hsplit, rolesplit, platsplit, eventsplit;
	ArrayCtrl roles, needs, causes, messages;
	ArrayCtrl platforms, actions, action_causes;
	ArrayCtrl events, entries;
	DocEdit event, entry;
	
public:
	typedef SocialNeedsCtrl CLASSNAME;
	SocialNeedsCtrl();
	
	void Data() override;
	void DataRole();
	void DataNeed();
	void DataAction();
	void DataEvent();
	void DataEntry();
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	
};


END_UPP_NAMESPACE

#endif
