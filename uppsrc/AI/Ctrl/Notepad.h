#ifndef _AI_Ctrl_Notepad_h_
#define _AI_Ctrl_Notepad_h_

NAMESPACE_UPP


class NotepadCtrl : public ComponentCtrl {
	Splitter hsplit;
	ArrayCtrl list;
	WithNotepad<Ctrl> idea;
	
	
	
public:
	typedef NotepadCtrl CLASSNAME;
	NotepadCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override {}
	void Clear();
	void DataNote();
	void OnListMenu(Bar& bar);
	void AddIdea();
	void RemoveIdea();
	void OnValueChange();
	
};

INITIALIZE(NotepadCtrl)


END_UPP_NAMESPACE

#endif
