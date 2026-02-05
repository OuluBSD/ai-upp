#ifndef _MaestroHub_DebugWorkspace_h_
#define _MaestroHub_DebugWorkspace_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

NAMESPACE_UPP

#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class DebugWorkspace : public WithDebugWorkspaceLayout<ParentCtrl> {
public:
	ParentCtrl left_pane;
	DropList   target_device;
	TreeCtrl   call_stack;
	
	ParentCtrl center_pane;
	RichTextCtrl source_code;
	
	ParentCtrl bottom_pane;
	ArrayCtrl  locals;
	
	Splitter   hsplit;
	Splitter   vsplit;
	
	void Load(const String& maestro_root);
	void OnRun();
	void OnStop();
	void OnStep();
	void OnToolbar(Bar& bar);
	
	typedef DebugWorkspace CLASSNAME;
	DebugWorkspace();
};

END_UPP_NAMESPACE

#endif
