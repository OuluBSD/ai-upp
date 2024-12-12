#ifndef _AI_Ctrl_Header_h_
#define _AI_Ctrl_Header_h_

NAMESPACE_UPP


struct SocialHeader : Component
{
	
	COMPONENT_CONSTRUCTOR(SocialHeader)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_SOCIAL_HEADER;}
	
};

INITIALIZE(SocialHeader)

class SocialHeaderCtrl : public ComponentCtrl {
	Splitter hsplit, vsplit;
	ArrayCtrl platforms, entries;
	//WithSocialHeader<Ctrl> entry;
	Splitter entry_split;
	ArrayCtrl attr_keys;
	DocEdit attr_value;
	
public:
	typedef SocialHeaderCtrl CLASSNAME;
	SocialHeaderCtrl();
	
	void Data() override;
	void DataPlatform();
	void ToolMenu(Bar& bar) override;
	void EntryListMenu(Bar& bar);
	void OnValueChange();
	void Do(int fn);
	
};

INITIALIZE(SocialHeaderCtrl)


END_UPP_NAMESPACE

#endif
