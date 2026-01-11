#include "Ctrl.h"


NAMESPACE_UPP


InterfaceCtrlFactoryMap& GetInterfaceCtrlFactoryMap() {static InterfaceCtrlFactoryMap map; return map;}

void RegisterInterfaceCtrlFactory(TypeCls iface, InterfaceCtrlFactory fac) {
	GetInterfaceCtrlFactoryMap().GetAdd(iface) = fac;
}

InterfaceCtrl* NewInterfaceCtrl(TypeCls iface) {
	auto& map = GetInterfaceCtrlFactoryMap();
	InterfaceCtrlFactory fn = 0;
	int i = map.Find(iface);
	if (i >= 0)
		fn = map[i];
	return fn ? fn() : 0;
}


END_UPP_NAMESPACE
