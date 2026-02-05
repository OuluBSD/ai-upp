#ifndef _MaestroHub_IssuesView_h_
#define _MaestroHub_IssuesView_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

NAMESPACE_UPP

class IssuesPane : public ParentCtrl {
	ArrayCtrl list;
	String    current_root;

public:
	void Load(const String& root);
	void OnMenu(Bar& bar);
	void OnTriage();
	void OnResolve();

	typedef IssuesPane CLASSNAME;
	IssuesPane();
};

END_UPP_NAMESPACE

#endif
