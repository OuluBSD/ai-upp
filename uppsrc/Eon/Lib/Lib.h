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

#if defined flagPHYSICS
 #include <api/Physics/Physics.h>
#endif

#include <api/Graphics/Graphics.h>

#ifdef flagVR
 #include <api/Holograph/Holograph.h>
#endif

#ifdef flagSCREEN
 #include <api/Screen/Screen.h>
#endif

#ifdef flagMEDIA
 #include <api/Media/Media.h>
#endif

#ifdef flagVOLUMETRIC
 #include <api/Volumetric/Volumetric.h>
#endif

#ifdef flagCAMERA
 #include <api/Camera/Camera.h>
#endif


#include <Eon/Draw/Draw.h>


#ifdef flagGUI
 #include <CtrlLib/CtrlLib.h>
 #include <CtrlLib/Eon/Eon.h>
 #include <Eon/Ecs/CommonComponents.h>
 #include <Painter/Painter.h>
 #include <Vfs/Ecs/Component.h>
 #include <Vfs/Ecs/Entity.h>
 #include <Geometry/GeomEvent.h>
 #include <CtrlLib/Eon/Eon.h>
#endif

#ifdef flagGUBO
 #include <GuboCore/CtrlEvent.h>
#endif


#define BIG_NUMBER 100000000

NAMESPACE_UPP

#include "Geom2DComponent.h"
#include "Gui.h"
#include "Handle.h"

#include "GeneratedAudio.h"

#include "GeneratedHandle.h"
#include "GeneratedMinimal.h"
#include "GeneratedVR.h"
#include "RenderingSystem.h"
#include "Absolute2D.h"

#ifdef flagPHYSICS
 #include "EcsPhysics.h"
 #include "OdePrefab.h"
 #include "TPrefab.h"
#endif

END_UPP_NAMESPACE

#endif

