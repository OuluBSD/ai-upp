#ifndef _AI_Ctrl_Messaging_h_
#define _AI_Ctrl_Messaging_h_

NAMESPACE_UPP


struct SocialContent : Component
{
	
	COMPONENT_CONSTRUCTOR(SocialContent)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_SOCIAL_CONTENT;}
	
};

INITIALIZE(SocialContent)

class SocialContentCtrl : public ComponentCtrl {
	Splitter hsplit, vsplit, menusplit, threadsplit;
	ArrayCtrl platforms, threads, entries, comments;
	WithSocialEntry<Ctrl> entry;
	
public:
	typedef SocialContentCtrl CLASSNAME;
	SocialContentCtrl();
	
	void Data() override;
	void DataPlatform();
	void DataEntry();
	void DataThread();
	void DataComment();
	void Clear();
	void ClearEntry();
	void OnValueChange();
	void AddEntry();
	void RemoveEntry();
	void AddThread();
	void RemoveThread();
	void AddComment();
	void RemoveComment();
	void PasteResponse(int fn);
	void ToolMenu(Bar& bar) override;
	void EntryListMenu(Bar& bar);
	void ThreadListMenu(Bar& bar);
	void CommentListMenu(Bar& bar);
	void Do(int fn);
	
};

INITIALIZE(SocialContentCtrl)


END_UPP_NAMESPACE

#endif
