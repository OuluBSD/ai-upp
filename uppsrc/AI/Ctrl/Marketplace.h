#ifndef _AI_Ctrl_Marketplace_h_
#define _AI_Ctrl_Marketplace_h_

NAMESPACE_UPP


class MarketplaceCtrl : public ComponentCtrl {
	Splitter hsplit, imgsplit;
	ArrayCtrl items, images;
	TabCtrl tabs;
	WithMarketplace<Ctrl> form;
	WithMarketplaceViewer<Ctrl> viewer;
	ImageViewerCtrl img;
	int filter_priority = MARKETPRIORITY_SELL_UPCOMING;
	
public:
	typedef MarketplaceCtrl CLASSNAME;
	MarketplaceCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	void DataItem();
	void DataImage();
	void DataCategory();
	void DataSubCategory();
	void OnValueChange();
	void OnDimensionChange();
	void ClearForm();
	void PasteImagePath();
	void MakeTempImages();
	void OnCategory();
	void OnSubCategory();
	void SetCategoryShorcut(int i);
	void SetCurrentImage(Image img);
	void LoadImagePath(String path);
	void ShowItems(int priority) {filter_priority = priority; PostCallback(THISBACK(Data));}
	
	String GetPackageString(int w, int h, int d, double weight);
	
};

INITIALIZE(MarketplaceCtrl)


END_UPP_NAMESPACE

#endif
