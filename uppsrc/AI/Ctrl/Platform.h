#ifndef _AI_Ctrl_Platform_h_
#define _AI_Ctrl_Platform_h_

NAMESPACE_UPP


class PlatformCtrl : public ComponentCtrl {
	Splitter hsplit, vsplit, bottom, epk_photo_prompt_split;
	TabCtrl plat_tabs, epk_tabs;
	ArrayCtrl roles, platforms, epk_text_fields, epk_photo_types, epk_photo_prompts;
	WithSocialPlatform<Ctrl> plat;
	ImageViewerCtrl epk_photo_prompt_example;
	
public:
	typedef PlatformCtrl CLASSNAME;
	PlatformCtrl();
	
	void Data() override;
	void DataPlatform();
	void ToolMenu(Bar& bar) override;
	void PlatformMenu(Bar& bar);
	void Do(int fn);
	void SetSorting(int col);
	void OnPhotoPrompt();
	
};

INITIALIZE(PlatformCtrl)


END_UPP_NAMESPACE

#endif
