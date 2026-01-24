#ifndef _MaestroHub_Product_h_
#define _MaestroHub_Product_h_

#include "MaestroHub.h"

NAMESPACE_UPP

class ProductPane : public ParentCtrl {
public:
	Splitter split;
	ArrayCtrl runbooks;
	ArrayCtrl workflows;
	
	void Load(const String& root);

	typedef ProductPane CLASSNAME;
	ProductPane();
};

END_UPP_NAMESPACE

#endif
