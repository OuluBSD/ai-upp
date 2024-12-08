#ifndef _AI_Ctrl_Notepad_h_
#define _AI_Ctrl_Notepad_h_

NAMESPACE_UPP


class SnapIdeas : public ToolAppCtrl {
	Splitter hsplit;
	ArrayCtrl list;
	WithSnapIdeas<Ctrl> idea;
	
	
	
public:
	typedef SnapIdeas CLASSNAME;
	SnapIdeas();
	
	void Data();
	void Clear();
	void IdeaData();
	void OnListMenu(Bar& bar);
	void AddIdea();
	void RemoveIdea();
	void OnValueChange();
	
};


END_UPP_NAMESPACE

#endif
