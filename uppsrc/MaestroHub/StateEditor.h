#ifndef _MaestroHub_StateEditor_h_
#define _MaestroHub_StateEditor_h_

#include "MaestroHub.h"

NAMESPACE_UPP

// Ensure layout is only included once in the blitz unit
#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class StateEditor : public WithStateEditorLayout<TopWindow> {
	LineEdit        puml_editor;
	GraphLib::GraphNodeCtrl graph_view;
	
	One<WorkflowManager> wfm;
	String          root;
	String          current_id;

public:
	void Load(const String& maestro_root, const String& id);
	void UpdatePreview();
	void OnToolbar(Bar& bar);
	void NewState();
	void NewTransition();
	void Save();
	
	typedef StateEditor CLASSNAME;
	StateEditor();
};

END_UPP_NAMESPACE

#endif