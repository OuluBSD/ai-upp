#ifndef _AI_Core_CoverImage_h_
#define _AI_Core_CoverImage_h_


NAMESPACE_UPP


struct ReleaseCoverImage : Component
{
	
	COMPONENT_CONSTRUCTOR(ReleaseCoverImage)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_RELEASE_COVER_IMAGE;}
	
};

INITIALIZE(ReleaseCoverImage)


END_UPP_NAMESPACE

#endif
