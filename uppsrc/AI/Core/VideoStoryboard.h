#ifndef _AI_Core_VideoStoryboard_h_
#define _AI_Core_VideoStoryboard_h_


NAMESPACE_UPP


struct VideoStoryboard : Component
{
	
	COMPONENT_CONSTRUCTOR(VideoStoryboard)
	
	void Visit(Vis& v) override {
		v.Ver(1)
		(1);
	}
	static int GetKind() {return METAKIND_ECS_COMPONENT_VIDEO_STORYBOARD;}
	
};

INITIALIZE(VideoStoryboard)


END_UPP_NAMESPACE

#endif
