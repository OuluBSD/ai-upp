#ifndef _AI_Core_Messaging_h_
#define _AI_Core_Messaging_h_


NAMESPACE_UPP


struct SocialContent : Component
{
	
	COMPONENT_CONSTRUCTOR(SocialContent)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_SOCIAL_CONTENT;}
	
};

INITIALIZE(SocialContent)


END_UPP_NAMESPACE

#endif
