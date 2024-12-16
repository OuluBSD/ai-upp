#ifndef _AI_Core_PlatformProfile_h_
#define _AI_Core_PlatformProfile_h_


NAMESPACE_UPP


struct PlatformProfile : Component
{
	
	COMPONENT_CONSTRUCTOR(PlatformProfile)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_PLATFORM_PROFILE;}
	
};

INITIALIZE(PlatformProfile)


END_UPP_NAMESPACE

#endif
