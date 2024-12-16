#ifndef _AI_Core_ImageBiographySummary_h_
#define _AI_Core_ImageBiographySummary_h_


NAMESPACE_UPP


struct ImageBiographySummary : Component
{
	
	COMPONENT_CONSTRUCTOR(ImageBiographySummary)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_BIOGRAPHY_IMAGES_SUMMARY;}
	
};

INITIALIZE(ImageBiographySummary)


END_UPP_NAMESPACE

#endif
