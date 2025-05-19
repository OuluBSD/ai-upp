#ifndef _AI_Core_Reasoning_h_
#define _AI_Core_Reasoning_h_





struct ScriptReasoning : Component
{
	
	COMPONENT_CONSTRUCTOR(ScriptReasoning)
	
	void Visit(Vis& v) override {
		v.Ver(1)
		(1);
	}
	static int GetKind() {return METAKIND_ECS_COMPONENT_SCRIPT_REASONING;}
	
};

INITIALIZE(ScriptReasoning)




#endif
