#ifndef _AI_Ctrl_BiographyPlatform_h_
#define _AI_Ctrl_BiographyPlatform_h_

NAMESPACE_UPP

class BiographyPlatformCtrl : public ComponentCtrl {
	TabCtrl tabs;
	
	
	// Platforms tab
	struct Platforms {
		Splitter hsplit;
		TabCtrl tabs;
		ArrayCtrl platforms;
		
		struct WithOwner {BiographyPlatformCtrl& o; Platforms& p;};
		
		struct Header : WithOwner {
			typedef Header CLASSNAME;
			Splitter vsplit;
			ArrayCtrl entries;
			Splitter entry_split;
			ArrayCtrl attr_keys;
			DocEdit attr_value;
		} header;
		
		struct Messaging : WithOwner {
			typedef Messaging CLASSNAME;
			Splitter vsplit, threadsplit;
			ArrayCtrl threads, entries, comments;
			WithSocialEntry<Ctrl> entry;
		} messaging;
		
		struct EpkPhoto : WithOwner {
			typedef EpkPhoto CLASSNAME;
			Splitter epk_photo_prompt_split, epk_photo_multi_image_split;
			ArrayCtrl  epk_photo_prompts;
			ImageViewerCtrl epk_photo[4];
		} epk_photo;
		
		struct Needs : WithOwner {
			typedef Needs CLASSNAME;
			Splitter vsplit, rolesplit, platsplit, eventsplit;
			ArrayCtrl roles, needs, causes, messages;
			ArrayCtrl actions, action_causes;
			ArrayCtrl events, entries;
			DocEdit event, entry;
		} needs;
		
		struct Marketplace : WithOwner {
			typedef Marketplace CLASSNAME;
			Splitter hsplit, imgsplit;
			ArrayCtrl items, images;
			TabCtrl tabs;
			WithMarketplace<Ctrl> form;
			WithMarketplaceViewer<Ctrl> viewer;
			ImageViewerCtrl img;
			int filter_priority = MARKETPRIORITY_SELL_UPCOMING;
			void Ctor();
			void DataPlatform();
			void ToolMenu(Bar& bar);
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
			void ShowItems(int priority);
			String GetPackageString(int w, int h, int d, double weight);
		} marketplace;
		
	} p;
	
	void Platforms_Ctor();
	void Platforms_EpkPhoto_Ctor();
	void Platforms_Data();
	void Platforms_DataPlatform();
	void Platforms_DataPlatform_Epk();
	void Platforms_ToolMenu(Bar& bar);
	void Platforms_PhotoPromptMenu(Bar& bar);
	void Platforms_OnPhotoPrompt();
	void Platforms_Menu(Bar& bar);
	
	void Platforms_Header_Ctor();
	void Platforms_Header_DataPlatform();
	void Platforms_Header_ToolMenu(Bar& bar);
	void Platforms_Header_EntryListMenu(Bar& bar);
	void Platforms_Header_OnValueChange();
	void Platforms_Header_Do(int fn);
	
	void Platforms_Messaging_Ctor();
	void Platforms_Messaging_DataPlatform();
	void Platforms_Messaging_DataEntry();
	void Platforms_Messaging_DataThread();
	void Platforms_Messaging_DataComment();
	void Platforms_Messaging_Clear();
	void Platforms_Messaging_ClearEntry();
	void Platforms_Messaging_OnValueChange();
	void Platforms_Messaging_AddEntry();
	void Platforms_Messaging_RemoveEntry();
	void Platforms_Messaging_AddThread();
	void Platforms_Messaging_RemoveThread();
	void Platforms_Messaging_AddComment();
	void Platforms_Messaging_RemoveComment();
	void Platforms_Messaging_PasteResponse(int fn);
	void Platforms_Messaging_ToolMenu(Bar& bar);
	void Platforms_Messaging_EntryListMenu(Bar& bar);
	void Platforms_Messaging_ThreadListMenu(Bar& bar);
	void Platforms_Messaging_CommentListMenu(Bar& bar);
	void Platforms_Messaging_Do(int fn);
	
	void Platforms_Needs_Ctor();
	void Platforms_Needs_DataPlatform();
	void Platforms_Needs_DataRole();
	void Platforms_Needs_DataNeed();
	void Platforms_Needs_DataAction();
	void Platforms_Needs_DataEvent();
	void Platforms_Needs_DataEntry();
	void Platforms_Needs_ToolMenu(Bar& bar);
	void Platforms_Needs_Do(int fn);
	
	// Clustered prompts tab
	struct {
		Splitter hsplit, vsplit, bsplit;
		ArrayCtrl image_types, prompts;
		DocEdit final_prompt;
		ImageViewerCtrl epk_photo[4];
	} c;
	
	void Clusters_Ctor();
	void Clusters_Data();
	void Clusters_DataImageType();
	void Clusters_ToolMenu(Bar& bar);
	
	// Audience tab
	struct {
		Splitter menusplit, hsplit, vsplit, bsplit;
		ArrayCtrl roles, profiles, responses, entries;
		WithAudience<Ctrl> entry;
		ImageViewerCtrl img;
	} a;
	
	void Audience_Ctor();
	void Audience_Data();
	void Audience_DataRole();
	void Audience_DataProfile();
	void Audience_DataResponse();
	void Audience_ToolMenu(Bar& bar);
	void Audience_EntryListMenu(Bar& bar);
	void Audience_Do(int fn);
	
public:
	typedef BiographyPlatformCtrl CLASSNAME;
	BiographyPlatformCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	void SetSorting(int col);
	void ImportJson();
};

INITIALIZE(BiographyPlatformCtrl)


END_UPP_NAMESPACE

#endif
