#ifndef _AI_TextCtrl_Entity_h_
#define _AI_TextCtrl_Entity_h_

NAMESPACE_UPP


class EntityEditorCtrl : public ToolAppCtrl {
	
public:
	typedef EntityEditorCtrl CLASSNAME;
	EntityEditorCtrl();
	
	static String GetExt() { return ".ecs"; }
	static String GetID() { return "Entity Editor"; }
	
};

END_UPP_NAMESPACE

#endif
