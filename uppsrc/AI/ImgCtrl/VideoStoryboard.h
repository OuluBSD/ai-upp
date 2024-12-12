#ifndef _AI_ImgCtrl_VideoStoryboard_h_
#define _AI_ImgCtrl_VideoStoryboard_h_

NAMESPACE_UPP


struct VideoStoryboard : Component
{
	
	COMPONENT_CONSTRUCTOR(VideoStoryboard)
	void Serialize(Stream& s) override {TODO}
	void Jsonize(JsonIO& json) override {TODO}
	hash_t GetHashValue() const override {TODO; return 0;}
	static int GetKind() {return METAKIND_ECS_COMPONENT_VIDEO_STORYBOARD;}
	
};

INITIALIZE(VideoStoryboard)

class VideoStoryboardCtrl : public ComponentCtrl {
	Splitter split, vsplit0, vsplit1;
	ArrayCtrl list;
	ImageViewerCtrl img[4];
	
public:
	typedef VideoStoryboardCtrl CLASSNAME;
	VideoStoryboardCtrl();
	
	void Data() override;
	void DataLine();
	void OnValueChange();
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	
	
};

INITIALIZE(VideoStoryboardCtrl)


END_UPP_NAMESPACE

#endif
