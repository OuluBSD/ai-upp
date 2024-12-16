#ifndef _AI_Core_Needs_h_
#define _AI_Core_Needs_h_


NAMESPACE_UPP


struct SocialNeeds : Component
{
	
	COMPONENT_CONSTRUCTOR(SocialNeeds)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_SOCIAL_NEEDS;}
	
};

INITIALIZE(SocialNeeds)


END_UPP_NAMESPACE

#endif
