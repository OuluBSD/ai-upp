#ifndef _IGraphics_IGraphics_h_
#define _IGraphics_IGraphics_h_

typedef float			Gxf;
typedef unsigned char	Gxb;
typedef unsigned char	Gxby;
typedef int				Gxi;
typedef unsigned int	Gxu;
typedef unsigned int	Gxbf;

#include <Core/config.h>
#include <Core/Core.h>

#if defined flagDX11
	#include "dxstdafx.h"
#endif


#if defined flagOGL
	#if UPP_OLD_VERSION || (defined flagGCC && defined flagWIN32)
		#define GLEW_STATIC 1
		#include <plugin/glew/glew.h>
	#else
		#ifdef flagMSC
			#ifdef flagWIN32
				#define CY win32_CY_
				#define FAR win32_FAR_
			#endif
			#include <GL/glew.h>
			#ifdef flagWIN32
				#undef CY
				#undef FAR
			#endif
		#else
			#include <GL/glew.h>
		#endif
	#endif
#endif


#if defined flagSDL2
	#ifdef flagWIN32
		#define CY win32_CY_
		#define FAR win32_FAR_
	#endif
	#if 0 //defined flagWIN32 && defined flagMSC
		#include <SDL.h>
		#include <SDL_ttf.h>
		#include <SDL_image.h>
		#include <SDL_syswm.h>
		#ifdef flagOGL
			#include <SDL_opengl.h>
		#endif
	#else
		#include <SDL2/SDL.h>
		#include <SDL2/SDL_ttf.h>
		#include <SDL2/SDL_image.h>
		#include <SDL2/SDL_syswm.h>
		#ifdef flagOGL
			#include <SDL2/SDL_opengl.h>
		#endif
	#endif
	#ifdef flagWIN32
		#undef CY
		#undef FAR
	#endif
#endif

#undef main

#include <Geom/Geom.h>
#include <plugin/mikktspace/mikktspace.h>
#include <plugin/stb/stb_image.h>
#include <api/Volumetric/Volumetric.h>
#include <Vfs/Vfs.h>
#include <Geometry/Geometry.h>
#include <SoftRend/SoftRend.h>
#include <Painter/Painter.h>

#include "Types.h"

#define NAMESPACE_GRAPHICS_BEGIN namespace  Upp { namespace  GL {
#define NAMESPACE_GRAPHICS_END }}

#define CHKLOGRET(x, y) if (!(x)) {LOG(y); return;}
#define CHKLOGRET0(x, y) if (!(x)) {LOG(y); return false;}
#define CHKLOGRET1(x, y) if (!(x)) {LOG(y); return true;}

#include "State.h"
#include "GfxClasses.h"
#include "GlobalApi.h"
#include "OpenGL.h"
#include "IfaceOgl.h"
#include "Base.h"
#include "FboBase.h"
#include "ImageBase.h"
#include "ProgBase.h"
#include "ObjViewProg.h"
#include "EcsViewProg.h"
#include "TState.h"
#include "TFramebuffer.h"
#include "TPipeline.h"
#include "TContext.h"
#include "TUtil.h"
#include "TBuffer.h"
#include "TBufferField.h"
#include "ProgDraw.h"
#include "GfxAccelAtom.h"

#endif