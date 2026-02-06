#ifndef _WebDriver_Wait_h_
#define _WebDriver_Wait_h_

#include <Core/Core.h>

NAMESPACE_UPP

template<typename T>
bool Wait_until(
	const T& condition,
	const Session& session,
	int timeout_ms = 5000,
	int interval_ms = 500
	);

template<typename T>
bool Wait_for(
	const T& condition,
	const Session& session,
	int timeout_ms = 5000,
	int interval_ms = 500
	);

END_UPP_NAMESPACE

#endif