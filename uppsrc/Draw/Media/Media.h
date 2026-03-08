#ifndef _Draw_Media_Media_h_
#define _Draw_Media_Media_h_

#include <Core/Core.h>
#include <Vfs/Ecs/Ecs.h>

#ifdef flagFFMPEG
extern "C" {
	#include <libavdevice/avdevice.h>
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libavutil/imgutils.h>
}
#define FFMPEG_VIDEOFRAME_RGBA_CONVERSION 1
#endif

NAMESPACE_UPP
#include <Core/EcsEngine/Util2.h>
#include <Eon/Core/Types.h>
#include <Eon/Core/Atom.h>

#include "Types.h"
#include "Audio.h"
#include "Video.h"
#include "MediaStream.h"
#include "FileIn.h"

END_UPP_NAMESPACE

#endif
