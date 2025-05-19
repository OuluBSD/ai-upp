#ifndef _AI_Core_ReleaseBriefing_h_
#define _AI_Core_ReleaseBriefing_h_





struct ReleaseBriefing : Component
{
	
	COMPONENT_CONSTRUCTOR(ReleaseBriefing)
	
	void Visit(Vis& v) override {
		v.Ver(1)
		(1);TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_RELEASE_BRIEFING;}
	
};

INITIALIZE(ReleaseBriefing)




#endif
