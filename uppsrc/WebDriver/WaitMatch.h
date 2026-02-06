#ifndef _WebDriver_WaitMatch_h_
#define _WebDriver_WaitMatch_h_

#include <Core/Core.h>

NAMESPACE_UPP

template<typename T>
bool WaitUntilMatches(
	const T& condition,
	const Session& session,
	int timeout_ms = 5000,
	int interval_ms = 500
) {
	int start = GetTickCount();
	while (int(GetTickCount()) - start < timeout_ms) {
		if (condition(session)) return true;
		Sleep(interval_ms);
	}
	return false;
}

END_UPP_NAMESPACE

#endif