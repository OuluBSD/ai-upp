#include "Core.h"

NAMESPACE_UPP

VfsValue* AtomBase::GetSpace() {
	return val.FindOwnerNull();
}

END_UPP_NAMESPACE
