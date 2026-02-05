#ifndef _MaestroHub_RunbookEditor_h_
#define _MaestroHub_RunbookEditor_h_

#include "MaestroHub.h"

NAMESPACE_UPP

// Ensure layout is only included once in the blitz unit
#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class RunbookEditor : public WithRunbookEditorLayout<TopWindow> {
	One<RunbookManager> rbm;
	One<Runbook>        work_rb;
	String              root;

public:
	void New(const String& maestro_root);
	void Load(const String& maestro_root, const String& id);
	void UpdateUI();
	void UpdatePreview();
	
	void AddStep();
	void EditStep();
	void RemoveStep();
	void OnSave();
	void OnResolve();
	
	typedef RunbookEditor CLASSNAME;
	RunbookEditor();
};

END_UPP_NAMESPACE

#endif