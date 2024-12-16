#ifndef _AI_Core_Audience_h_
#define _AI_Core_Audience_h_

NAMESPACE_UPP


struct Audience : Component
{
	
	COMPONENT_CONSTRUCTOR(Audience)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	 TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_AUDIENCE;}
	
};

INITIALIZE(Audience)


END_UPP_NAMESPACE

#endif
