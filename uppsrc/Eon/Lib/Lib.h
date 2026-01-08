#ifndef _EonLib_EonLib_h_
#define _EonLib_EonLib_h_

#include <Eon/Draw/Draw.h>

#ifdef flagMIDI
	#include <api/MidiFile/MidiFile.h>
	#include <api/MidiHw/MidiHw.h>
#endif

#ifdef flagAUDIO
	#include <api/Synth/Synth.h>
	#include <api/Effect/Effect.h>
	#include <api/Audio/Audio.h>
#endif

#include <api/Hal/Hal.h>

#ifdef flagPHYSICS
	#include <api/Physics/Physics.h>
#endif

#ifdef flagSCREEN
	#include <api/Graphics/Graphics.h>
#endif

#ifdef flagVR
	#include <api/Holograph/Holograph.h>
#endif

#include <api/Screen/Screen.h>

#ifdef flagMEDIA
	#include <api/Media/Media.h>
#endif

#ifdef flagVOLUMETRIC
	#include <api/Volumetric/Volumetric.h>
#endif

#ifdef flagCAMERA
	#include <api/Camera/Camera.h>
#endif

#ifdef flagAUDIO
	#include <api/AudioFileOut/AudioFileOut.h>
#endif

#include <Eon/Draw/Draw.h>


#define BIG_NUMBER 100000000

NAMESPACE_UPP

#include "Handle.h"

#include "GeneratedAudio.h"

#include "GeneratedHandle.h"
#include "GeneratedMinimal.h"
#ifdef flagVR
	#include "GeneratedVR.h"
#endif
#include "RenderingSystem.h"
#include "Absolute2D.h"
#include "Geom2DComponent.h"
#include "Gui.h"
#ifdef flagPHYSICS
	#include "EcsPhysics.h"
	#include "OdePrefab.h"
	#include "TPrefab.h"
#endif

END_UPP_NAMESPACE

#endif

