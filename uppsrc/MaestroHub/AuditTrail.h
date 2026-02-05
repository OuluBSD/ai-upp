#ifndef _MaestroHub_AuditTrail_h_
#define _MaestroHub_AuditTrail_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

NAMESPACE_UPP

#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class AuditTrailCorrelator : public WithAuditTrailLayout<ParentCtrl> {
public:
	ArrayCtrl    event_list;
	RichTextView detail_view;
	
	void Load(const String& maestro_root);
	void OnEventCursor();
	
	typedef AuditTrailCorrelator CLASSNAME;
	AuditTrailCorrelator();
};

END_UPP_NAMESPACE

#endif
