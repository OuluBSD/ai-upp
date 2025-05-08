#ifndef _Shell_Prefabs_h_
#define _Shell_Prefabs_h_

#if defined flagSDL2
	#ifdef flagOGL
		// The piped version is currently non-optimal and unimplemented
		#if 0
			#define GUI_EON "deps/default_gui_app_sdl_ogl.eon"
		#else
			#define GUI_EON "deps/default_gui_app_uppsdl_ogl.eon"
		#endif
		#define GUBO_EON "deps/default_gubo_app_sdl_ogl.eon"
		#define SURFACE_EON "deps/default_surface_app_sdl.eon"
	#else
		#define GUI_EON "deps/default_gui_app_sdl.eon"
		#define GUBO_EON "deps/default_gubo_app_sdl.eon"
		#define SURFACE_EON "deps/default_surface_app_sdl.eon"
	#endif
#elif defined flagPOSIX
	#define GUI_EON "deps/default_gui_app_x11.eon"
	#define GUBO_EON "deps/default_gubo_app_x11.eon"
#elif defined flagWIN32
	#define GUI_EON ""
	#define GUBO_EON ""
	#define SURFACE_EON ""
#else
	#error No eon file for GUI app on this platform.
#endif

#ifndef flagMSC
	#warning TODO
#endif

/*#define GUI_APP_MAIN \
void UserGuiMainFn_(); \
\
APP_INITIALIZE_STARTUP_(Upp::DefaultSerialInitializer, Upp::DefaultStartup) \
RENDER_APP_MAIN { \
	Upp::Serial::Machine::WhenUserProgram << Callback(UserGuiMainFn_); \
	Upp::DefaultRunner(0, "Gui App", GUI_EON);} \
\
void UserGuiMainFn_()*/

#define GUBO_APP_MAIN \
void UserGuiMainFn_(); \
\
APP_INITIALIZE_STARTUP_(Upp::DefaultSerialInitializer, Upp::DefaultStartup) \
RENDER_APP_MAIN { \
	Upp::Serial::Machine::WhenUserProgram << Callback(UserGuiMainFn_); \
	Upp::DefaultRunner(0, "Gui App", GUBO_EON);} \
\
void UserGuiMainFn_()

#endif

#endif
