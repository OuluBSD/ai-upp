#ifndef _DShow_inc_h_
#define _DShow_inc_h_

#include <Core/Core.h>

// This header encapsulates the messy details of including DirectShow headers
// in a U++ project to avoid conflicts with Core/Core.h.

#ifdef flagWIN32

#define CY win32_CY_
#define FAR win32_FAR_

#include <windows.h>
#include <dshow.h>
#include <qedit.h>

#undef CY
#undef FAR

#pragma comment(lib, "strmiids.lib")

#endif

#endif
