#ifndef _EonLib_EonLib_h_
#define _EonLib_EonLib_h_

#include <Eon/Draw/Draw.h>

#ifdef flagMIDI
 #include <api/MidiFile/MidiFile.h>
 #include <api/MidiHw/MidiHw.h>
#endif

#if defined flagAUDIO && defined flagMIDI
 #include <api/Synth/Synth.h>
#endif

#if defined flagAUDIO
 #include <api/Effect/Effect.h>
 #include <api/Audio/Audio.h>
 #include <api/AudioFileOut/AudioFileOut.h>
#endif

#ifdef flagHAL
 #include <api/Hal/Hal.h>
#endif

NAMESPACE_UPP

#ifdef flagHAL
 using CenterGuiFileSrcBase = Upp::UppGuiFileSrc;
#endif

#include "Geom2DComponent.h"
#include "Gui.h"
#include "Handle.h"
#include "GeneratedMinimal.h"
#include "GeneratedVR.h"
#include "GeneratedLocal.h"
#include "RenderingSystem.h"
#include "Absolute2D.h"

#ifdef flagPHYSICS
 #include "EcsPhysics.h"
 #include "OdePrefab.h"
 #include "TPrefab.h"
#endif

END_UPP_NAMESPACE

#endif

