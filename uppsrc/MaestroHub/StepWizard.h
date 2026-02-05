#ifndef _MaestroHub_StepWizard_h_
#define _MaestroHub_StepWizard_h_

#include "MaestroHub.h"

NAMESPACE_UPP

// Ensure layout is only included once in the blitz unit
#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class StepWizard : public WithStepWizardLayout<TopWindow> {
public:
	RunbookStep step;
	
	void SetStep(const RunbookStep& s);
	RunbookStep GetStep();
	
	void AddVariant();
	void RemoveVariant();
	
	typedef StepWizard CLASSNAME;
	StepWizard();
};

END_UPP_NAMESPACE

#endif
