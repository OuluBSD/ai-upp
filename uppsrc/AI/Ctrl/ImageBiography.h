#ifndef _AI_Ctrl_ImageBiography_h_
#define _AI_Ctrl_ImageBiography_h_

NAMESPACE_UPP


class ImageBiographyCtrl : public ToolAppCtrl {
	Splitter hsplit, vsplit, bsplit;
	ArrayCtrl categories, years, entries;
	WithImageBiography<Ctrl> year;
	ImageViewerCtrl img;
	
public:
	typedef ImageBiographyCtrl CLASSNAME;
	ImageBiographyCtrl();
	
	void Data() override;
	void DataCategory();
	void DataYear();
	void DataEntry();
	void OnCategoryCursor();
	void OnValueChange();
	void Translate();
	void MakeKeywords(int fn);
	void OnTranslate(String s);
	void OnKeywords(int fn, String s);
	void ToolMenu(Bar& bar) override;
	void EntryListMenu(Bar& bar);
	void AddEntry();
	void RemoveEntry();
	void PasteImagePath();
	void AnalyseImage();
	void SetCurrentImage(Image img);
	void Do(int fn);
	
	
};


END_UPP_NAMESPACE

#endif
