#include "LocalHMD.h"

NAMESPACE_UPP

INITBLOCK_(LocalHMD) {
	::Upp::DaemonBase::Register<Upp::HMD::LocalHMDService>("LocalHMD");
}

END_UPP_NAMESPACE
