#ifndef _AI_ImgCtrl_VideoPrompt_h_
#define _AI_ImgCtrl_VideoPrompt_h_

NAMESPACE_UPP


class VideoPromptMakerCtrl : public ToolAppCtrl {
	Splitter vsplit, storyboard_prompt_split;
	ArrayCtrl storyboard_parts;
	ArrayCtrl storyboard_prompt_keys, storyboard_prompt_values;
	ArrayCtrl text_storyboard_parts;
	
public:
	typedef VideoPromptMakerCtrl CLASSNAME;
	VideoPromptMakerCtrl();
	
	void Data() override;
	void DataPrompt();
	void OnValueChange();
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	
	
};


END_UPP_NAMESPACE

#endif
