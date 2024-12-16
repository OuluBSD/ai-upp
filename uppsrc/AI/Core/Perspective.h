#ifndef _AI_Core_Perspective_h_
#define _AI_Core_Perspective_h_


NAMESPACE_UPP


struct Perspective : Component
{
	
	COMPONENT_CONSTRUCTOR(Perspective)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_PERSPECTIVE;}
	
};

INITIALIZE(Perspective);


END_UPP_NAMESPACE

#endif
