#ifndef _EonLib_EonLib_h_
#define _EonLib_EonLib_h_

#include <EonDraw/EonDraw.h>

#include <api/MidiFile/MidiFile.h>
#include <api/MidiHw/MidiHw.h>
#include <api/Synth/Synth.h>
#include <api/Effect/Effect.h>
#include <api/Audio/Audio.h>
#include <api/Hal/Hal.h>

#ifdef flagSCREEN
	#include <api/Graphics/Graphics.h>
#endif

#ifdef flagVR
	#include <api/Holograph/Holograph.h>
#endif

#include <api/Screen/Screen.h>
#include <api/Media/Media.h>
#include <api/Volumetric/Volumetric.h>
#include <api/Camera/Camera.h>
#include <api/AudioFileOut/AudioFileOut.h>
#include <api/Holograph/Holograph.h>

#define BIG_NUMBER 100000000

/*
TODO
 - move normal calculation after culling from MinimalVertexLoaderShader
*/

#include "GeneratedAudio.h"


#endif
