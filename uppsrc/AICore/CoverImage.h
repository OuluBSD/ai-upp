#ifndef _AI_Core_CoverImage_h_
#define _AI_Core_CoverImage_h_





struct ReleaseCoverImage : Component
{
	
	COMPONENT_CONSTRUCTOR(ReleaseCoverImage)
	
	void Visit(Vis& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_RELEASE_COVER_IMAGE;}
	
};

INITIALIZE(ReleaseCoverImage)




#endif
