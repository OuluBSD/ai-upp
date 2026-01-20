#ifndef _Maestro_RepoView_h_
#define _Maestro_RepoView_h_

#include <CtrlLib/CtrlLib.h>
#include "RepoScanner.h"

NAMESPACE_UPP

class RepoView : public ParentCtrl {
public:
	TreeCtrl tree;
	
	void Set(const Array<AssemblyInfo>& assemblies, const Array<PackageInfo>& packages);

	typedef RepoView CLASSNAME;
	RepoView();
};

END_UPP_NAMESPACE

#endif
