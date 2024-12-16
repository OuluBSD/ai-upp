#ifndef _AI_Core_VideoPrompt_h_
#define _AI_Core_VideoPrompt_h_


NAMESPACE_UPP


struct VideoPromptMaker : Component
{
	
	COMPONENT_CONSTRUCTOR(VideoPromptMaker)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_VIDEO_PROMPT_MAKER;}
	
};

INITIALIZE(VideoPromptMaker)


END_UPP_NAMESPACE

#endif
