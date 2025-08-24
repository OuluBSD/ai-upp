#ifndef _ide_Shell_Agent_h_
#define _ide_Shell_Agent_h_




class IdeAgent : public ParentCtrl {
	Splitter hsplit;
	ArrayCtrl tasklist;
	AgentPromptEdit edit;
	One<::RightTabs> tabs;
	Ctrl placeholder;
	TimeCallback tc;
	
public:
	typedef IdeAgent CLASSNAME;
	IdeAgent();
	void Menu(Bar& bar);
	void OnTab();
	void Data();
	void DataTasks();
};


#endif
