#ifndef _AI_Ctrl_BiographySummary_h_
#define _AI_Ctrl_BiographySummary_h_

NAMESPACE_UPP


class BiographySummaryCtrl : public ToolAppCtrl {
	Splitter hsplit, vsplit;
	ArrayCtrl categories, blocks;
	WithBiography<Ctrl> block;
	
public:
	typedef BiographySummaryCtrl CLASSNAME;
	BiographySummaryCtrl();
	
	void Data() override;
	void DataCategory();
	void DataYear();
	void OnValueChange();
	void Translate();
	void MakeKeywords();
	void UpdateElements();
	void Do(int fn);
	void OnTranslate(String s);
	void OnKeywords(String s);
	void ToolMenu(Bar& bar) override;
	void EntryListMenu(Bar& bar);
	
	
};


END_UPP_NAMESPACE

#endif
