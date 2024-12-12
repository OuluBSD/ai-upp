#ifndef _AI_Ctrl_LeadPublisherCtrl_h_
#define _AI_Ctrl_LeadPublisherCtrl_h_

NAMESPACE_UPP


struct LeadPublisher : Component
{
	
	COMPONENT_CONSTRUCTOR(LeadPublisher)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_LEAD_PUBLISHER;}
	
};

INITIALIZE(LeadPublisher)

class LeadPublisherCtrl : public ComponentCtrl {
	Splitter hsplit, vsplit;
	ArrayCtrl list, artists;
	WithPublisherInfo<Ctrl> info;
	
public:
	typedef LeadPublisherCtrl CLASSNAME;
	LeadPublisherCtrl();
	
	void Data() override;
	void DataItem();
	void ToolMenu(Bar& bar) override;
	void ListMenu(Bar& bar);
	void Do(int fn);
	void AddPublisher();
	void RemovePublisher();
	void ValueChange();
	void PasteArtists();
	
	
};

INITIALIZE(LeadPublisherCtrl)


END_UPP_NAMESPACE

#endif
