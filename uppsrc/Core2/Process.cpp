#include "Core.h"

NAMESPACE_UPP

void ErrorSourceBuffered::DumpMessages() {
	for(const ProcMsg& pm : messages) {
		LOG(pm.ToString());
	}
}

END_UPP_NAMESPACE
