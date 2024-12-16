#ifndef _AI_Core_Notepad_h_
#define _AI_Core_Notepad_h_


NAMESPACE_UPP


struct Notepad : Component
{
	
	COMPONENT_CONSTRUCTOR(Notepad)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1);	TODO}
	static int GetKind() {return METAKIND_ECS_COMPONENT_NOTEPAD;}
	
};

INITIALIZE(Notepad)


END_UPP_NAMESPACE

#endif
