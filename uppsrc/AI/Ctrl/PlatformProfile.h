#ifndef _AI_Ctrl_PlatformProfile_h_
#define _AI_Ctrl_PlatformProfile_h_

NAMESPACE_UPP


class PlatformProfileCtrl : public ComponentCtrl {
	TabCtrl tabs;
	
	// Platform tab
	struct {
		Splitter hsplit, epk_photo_prompt_split, epk_photo_multi_image_split;
		TabCtrl tabs;
		ArrayCtrl platforms, epk_photo_prompts;
		ImageViewerCtrl epk_photo[4];
	} p;
	
	// Clustered prompts tab
	struct {
		Splitter hsplit, vsplit, bsplit;
		ArrayCtrl image_types, prompts;
		DocEdit final_prompt;
		ImageViewerCtrl epk_photo[4];
	} c;
	
public:
	typedef PlatformProfileCtrl CLASSNAME;
	PlatformProfileCtrl();
	
	void Data() override;
	void DataPlatforms();
	void DataPlatform();
	void DataClusters();
	void DataImageType();
	void ToolMenu(Bar& bar) override;
	void PlatformMenu(Bar& bar);
	void PhotoPromptMenu(Bar& bar);
	void Do(int fn);
	void SetSorting(int col);
	void OnPhotoPrompt();
	
};

INITIALIZE(PlatformProfileCtrl)


END_UPP_NAMESPACE

#endif
