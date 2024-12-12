#ifndef _AI_Ctrl_Notepad_h_
#define _AI_Ctrl_Notepad_h_

NAMESPACE_UPP


struct Notepad : Component
{
	
	COMPONENT_CONSTRUCTOR(Notepad)
	void Serialize(Stream& s) override {TODO}
	void Jsonize(JsonIO& json) override {TODO}
	hash_t GetHashValue() const override {TODO; return 0;}
	static int GetKind() {return METAKIND_ECS_COMPONENT_NOTEPAD;}
	
};

INITIALIZE(Notepad)

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
	void IdeaData();
	void OnListMenu(Bar& bar);
	void AddIdea();
	void RemoveIdea();
	void OnValueChange();
	
};

INITIALIZE(NotepadCtrl)


END_UPP_NAMESPACE

#endif
