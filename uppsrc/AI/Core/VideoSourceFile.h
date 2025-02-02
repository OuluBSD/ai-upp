#ifndef _AI_Core_VideoSourceFile_h_
#define _AI_Core_VideoSourceFile_h_


NAMESPACE_UPP


struct VideoSourceFile : Component
{
	
	COMPONENT_CONSTRUCTOR(VideoSourceFile)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);
	}
	static int GetKind() {return METAKIND_ECS_COMPONENT_VIDEO_SOURCE_FILE;}
	
};

INITIALIZE(VideoSourceFile)



struct VideoSourceFileRange : Component
{
	
	COMPONENT_CONSTRUCTOR(VideoSourceFileRange)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);
	}
	static int GetKind() {return METAKIND_ECS_COMPONENT_VIDEO_SOURCE_FILE_RANGE;}
	
};

INITIALIZE(VideoSourceFileRange)




struct VideoSourceFileAudio : Component
{
	
	COMPONENT_CONSTRUCTOR(VideoSourceFileAudio)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);
	}
	static int GetKind() {return METAKIND_ECS_COMPONENT_VIDEO_SOURCE_FILE_AUDIO;}
	
};

INITIALIZE(VideoSourceFileAudio)


END_UPP_NAMESPACE


#endif
