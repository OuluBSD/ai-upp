#ifndef _AI_Core_ReleaseBriefing_h_
#define _AI_Core_ReleaseBriefing_h_


NAMESPACE_UPP


struct ReleaseBriefing : Component
{
	
	COMPONENT_CONSTRUCTOR(ReleaseBriefing)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_RELEASE_BRIEFING;}
	
};

INITIALIZE(ReleaseBriefing)


END_UPP_NAMESPACE

#endif
