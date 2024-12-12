#ifndef _AI_ImgCtrl_VideoStoryboard_h_
#define _AI_ImgCtrl_VideoStoryboard_h_

NAMESPACE_UPP


struct VideoStoryboard : Component
{
	
	COMPONENT_CONSTRUCTOR(VideoStoryboard)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
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
