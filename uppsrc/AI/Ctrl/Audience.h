#ifndef _AI_Ctrl_Audience_h_
#define _AI_Ctrl_Audience_h_

NAMESPACE_UPP


class AudienceCtrl : public ToolAppCtrl {
	Splitter menusplit, hsplit, vsplit, bsplit;
	ArrayCtrl roles, profiles, responses, entries;
	WithAudience<Ctrl> entry;
	ImageViewerCtrl img;
	
public:
	typedef AudienceCtrl CLASSNAME;
	AudienceCtrl();
	
	void Data() override;
	void DataRole();
	void DataProfile();
	void DataResponse();
	void ToolMenu(Bar& bar) override;
	void EntryListMenu(Bar& bar);
	void Do(int fn);
	
	
};


END_UPP_NAMESPACE

#endif
