#ifndef _WebDriver_WaitMatch_h_
#define _WebDriver_WaitMatch_h_

#include <Core/Core.h>

NAMESPACE_UPP

template<typename T>
bool Wait_until_matches(
	const T& condition,
	const Session& session,
	int timeout_ms = 5000,
	int interval_ms = 500
	);

END_UPP_NAMESPACE

#endif