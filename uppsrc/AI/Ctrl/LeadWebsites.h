#ifndef _AI_Ctrl_LeadWebsites_h_
#define _AI_Ctrl_LeadWebsites_h_

NAMESPACE_UPP


class LeadWebsites : public ToolAppCtrl {
	Splitter vsplit, hsplit, mainsplit, bsplit, bvsplit, bssplit;
	ArrayCtrl websites, list, payouts, prices, attrs;
	ArrayCtrl bools, strings, list_names, list_values;
	ArrayCtrl song_typecasts, lyrics_ideas, music_styles;
	
public:
	typedef LeadWebsites CLASSNAME;
	LeadWebsites();
	
	void Data() override;
	void DataWebsite();
	void DataPayout();
	void DataPrice();
	void DataOpportunity();
	void DataAnalyzedList();
	void CreateScript();
	void CopyHeaderClipboard();
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	
	
};


END_UPP_NAMESPACE

#endif
