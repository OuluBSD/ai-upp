#ifndef _AI_Core_Header_h_
#define _AI_Core_Header_h_


NAMESPACE_UPP


struct SocialHeader : Component
{
	
	COMPONENT_CONSTRUCTOR(SocialHeader)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_SOCIAL_HEADER;}
	
};

INITIALIZE(SocialHeader)


END_UPP_NAMESPACE

#endif
