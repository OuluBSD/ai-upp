#ifndef _Sound_Sound_h_
#define _Sound_Sound_h_

#include <Core/Core.h>

#ifdef flagWIN32
	#define CY win32_CY_
	#define FAR win32_FAR_
#endif
#if defined flagBUILTIN_PORTAUDIO
	#include <plugin/portaudio/portaudio.h>
#else
	#ifdef flagPORTAUDIO
	#include <portaudio.h>
	#endif
#endif
#ifdef flagWIN32
	#undef CY
	#undef FAR
#endif


#include "Types.h"
#include "SoundClip.h"
#include "SoundSystem.h"
#include "SoundThread.h"
#include "Discussion.h"
#include "SoundDaemon.h"

#endif
