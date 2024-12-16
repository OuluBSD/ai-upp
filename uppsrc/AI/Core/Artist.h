#ifndef _AI_Core_Artist_h_
#define _AI_Core_Artist_h_

NAMESPACE_UPP


struct Artist : Component
{
	
	COMPONENT_CONSTRUCTOR(Artist)
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO;}
	static int GetKind() {return METAKIND_ECS_COMPONENT_ARTIST;}
	
};

INITIALIZE(Artist)


END_UPP_NAMESPACE

#endif
