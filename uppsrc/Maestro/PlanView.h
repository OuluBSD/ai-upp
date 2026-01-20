#ifndef _Maestro_PlanView_h_
#define _Maestro_PlanView_h_

#include <CtrlLib/CtrlLib.h>
#include "PlanModels.h"

NAMESPACE_UPP

class PlanView : public ParentCtrl {
public:
	TreeCtrl tree;
	
	void Set(const Array<Track>& tracks);

	typedef PlanView CLASSNAME;
	PlanView();
};

END_UPP_NAMESPACE

#endif
