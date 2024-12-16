#ifndef _AI_Core_BiographyElements_h_
#define _AI_Core_BiographyElements_h_

NAMESPACE_UPP


struct BiographyElements : Component
{
	
	COMPONENT_CONSTRUCTOR(BiographyElements)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_BIOGRAPHY_ELEMENTS;}
	
};

INITIALIZE(BiographyElements)


END_UPP_NAMESPACE

#endif
