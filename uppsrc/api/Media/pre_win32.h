#ifndef _pre_win32_h_
#define _pre_win32_h_

#ifdef PLATFORM_WIN32

#include <windows.h>

#ifdef FAR
#undef FAR
#endif

#ifdef CY
#undef CY
#endif

#endif

#endif
