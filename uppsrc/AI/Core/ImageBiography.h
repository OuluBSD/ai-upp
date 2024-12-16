#ifndef _AI_Core_ImageBiography_h_
#define _AI_Core_ImageBiography_h_


NAMESPACE_UPP


struct ImageBiography : Component
{
	
	COMPONENT_CONSTRUCTOR(ImageBiography)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_BIOGRAPHY_IMAGES;}
	
};

INITIALIZE(ImageBiography)


END_UPP_NAMESPACE

#endif
