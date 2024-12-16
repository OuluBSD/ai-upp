#ifndef _AI_Core_Reasoning_h_
#define _AI_Core_Reasoning_h_


NAMESPACE_UPP


struct ScriptReasoning : Component
{
	
	COMPONENT_CONSTRUCTOR(ScriptReasoning)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	 TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_SCRIPT_REASONING;}
	
};

INITIALIZE(ScriptReasoning)


END_UPP_NAMESPACE

#endif
