#ifndef _AI_Ctrl_LeadPublishers_h_
#define _AI_Ctrl_LeadPublishers_h_

NAMESPACE_UPP


class LeadPublishers : public ToolAppCtrl {
	Splitter hsplit, vsplit;
	ArrayCtrl list, artists;
	WithPublisherInfo<Ctrl> info;
	
public:
	typedef LeadPublishers CLASSNAME;
	LeadPublishers();
	
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


END_UPP_NAMESPACE

#endif
