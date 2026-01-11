#ifndef _CtrlLib_Eon_Defs_h_
#define _CtrlLib_Eon_Defs_h_



#if defined flagOGL
	#if UPP_OLD_VERSION || (defined flagWIN32 && defined flagGCC)
		#define GLEW_STATIC 1
		#include <plugin/glew/glew.h>
	#else
		#include <GL/glew.h>
	#endif
	
	#if HAVE_GLUT
		#include <GL/freeglut.h>
	#endif
#endif



#ifdef GUIPLATFORM_VIRTUALGUI_INCLUDE
	#include GUIPLATFORM_VIRTUALGUI_INCLUDE
#endif


#endif
