#ifndef _Sound_Sound_h_
#define _Sound_Sound_h_

#include <Core/Core.h>

#if defined flagSYS_PORTAUDIO || (defined flagWIN32 && !defined flagMSC)
	#include <portaudio.h>
#else
	#include <plugin/portaudio/portaudio.h>
#endif

#include "Types.h"
#include "SoundClip.h"
#include "SoundSystem.h"
#include "SoundThread.h"
#include "Discussion.h"
#include "SoundDaemon.h"

#endif
