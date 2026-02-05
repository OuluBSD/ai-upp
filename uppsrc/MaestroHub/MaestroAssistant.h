#ifndef _MaestroHub_MaestroAssistant_h_
#define _MaestroHub_MaestroAssistant_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

NAMESPACE_UPP

// Ensure layout is only included once in the blitz unit
#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class MaestroAssistant : public WithGlobalAssistantLayout<ParentCtrl> {
public:
	AIChatCtrl chat;
	ArrayCtrl  context_stack;
	bool       is_expanded = true;
	
	void Toggle();
	void UpdateContext(const String& track, const String& phase, const String& task);
	
	typedef MaestroAssistant CLASSNAME;
	MaestroAssistant();
};

END_UPP_NAMESPACE

#endif