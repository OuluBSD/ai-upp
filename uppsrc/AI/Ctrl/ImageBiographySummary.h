#ifndef _AI_Ctrl_ImageBiographySummary_h_
#define _AI_Ctrl_ImageBiographySummary_h_

NAMESPACE_UPP


class ImageBiographySummaryCtrl : public ToolAppCtrl {
	Splitter hsplit, vsplit;
	ArrayCtrl categories, blocks;
	WithBiography<Ctrl> block;
	
public:
	typedef ImageBiographySummaryCtrl CLASSNAME;
	ImageBiographySummaryCtrl();
	
	void Data() override;
	void DataCategory();
	void DataYear();
	void OnValueChange();
	void Translate();
	void MakeKeywords();
	void Do(int fn);
	void OnTranslate(String s);
	void OnKeywords(String s);
	void ToolMenu(Bar& bar) override;
	void EntryListMenu(Bar& bar);
	
	
};


END_UPP_NAMESPACE

#endif
