#ifndef _AI_Core_BiographySummary_h_
#define _AI_Core_BiographySummary_h_

NAMESPACE_UPP


struct BiographySummary : Component
{
	
	COMPONENT_CONSTRUCTOR(BiographySummary)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_BIOGRAPHY_SUMMARY;}
	
};

INITIALIZE(BiographySummary)


END_UPP_NAMESPACE

#endif
