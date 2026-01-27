#include "Screen.h"
		
#if defined flagX11 && defined flagSCREEN && defined flagOGL

#define None 0

NAMESPACE_UPP

struct ScrX11Ogl::NativeContext {
    ::Window win;
    ::Display* display;
    ::XImage* fb;
    ::Visual* visual;
    ::GC gc;
    ::XVisualInfo* visual_info;
    ::Atom  atomWmDeleteWindow;
    ::XSetWindowAttributes attr;
    bool running = false;
    ::Atom wmDeleteMessage;
};

struct ScrX11Ogl::NativeSinkDevice {
    NativeContext* ctx;
    ::GLXContext gl_ctx;
    GfxAccelAtom<X11OglGfx> accel;
    bool log_avg = false;
    int avg_log_interval = 16;
    int frame_counter = 0;
    bool desktop_with_hmd = false;
    bool xrandr_setup = true;
    String hmd_output;
    int x_display_idx = -1;
    bool started_hmd = false;
};

struct ScrX11Ogl::NativeEventsBase {
    NativeContext* ctx;
    int time;
    dword seq;
    Vector<UPP::GeomEvent> ev;
    Size sz;
    bool ev_sendable;
    bool is_lalt;
    bool is_ralt;
    bool is_lshift;
    bool is_rshift;
    bool is_lctrl;
    bool is_rctrl;
    Point prev_mouse_pt;
    ::XEvent xev;
};


GfxAccelAtom<X11OglGfx>& Get_ScrX11Ogl_Ogl(ScrX11Ogl::NativeSinkDevice*& dev) {
	return dev->accel;
}


bool X11Ogl_IsExtensionSupported(const char *extList, const char *extension) {
	const char *start;
	const char *where, *terminator;
	
	/* Extension names should not have spaces. */
	where = strchr(extension, ' ');
	if (where || *extension == '\0')
		return false;
		
	/* It takes a bit of care to be fool-proof about parsing the
	 OpenGL extensions string. Don't be fooled by sub-strings,
	 etc. */
	for (start = extList;;) {
		where = strstr(start, extension);
		
		if (!where) {
			break;
		}
		
		terminator = where + strlen(extension);
		
		if (where == start || *(where - 1) == ' ') {
			if (*terminator == ' ' || *terminator == '\0') {
				return true;
			}
		}
		
		start = terminator;
	}
	
	return false;
}



typedef struct {
	unsigned long   flags;
	unsigned long   functions;
	unsigned long   decorations;
	long            input_mode;
	unsigned long   status;
} Hints;

bool ScrX11Ogl::SinkDevice_Create(NativeSinkDevice*& dev) {
	dev = new NativeSinkDevice;
	return true;
}

void ScrX11Ogl::SinkDevice_Destroy(NativeSinkDevice*& dev) {
	delete dev;
}

void ScrX11Ogl::SinkDevice_Visit(NativeSinkDevice& dev, AtomBase&, Visitor& v) {
	v VISN(dev.accel);
}

bool ScrX11Ogl::SinkDevice_Initialize(NativeSinkDevice& dev, AtomBase& a, const WorldState& ws) {
	auto ctx_ = a.val.FindOwnerWithCastDeep<X11OglContext>();
	if (!ctx_) { LOG("error: could not find X11 context"); return false;}
	auto& ctx = *ctx_->dev;
	dev.ctx = &ctx;
	
	a.AddDependency(*ctx_);
	
	if (!dev.accel.Initialize(a, ws))
		return false;
	
	bool is_borderless = ws.IsTrue(".borderless");
	bool is_fullscreen = ws.IsTrue(".fullscreen");
	bool print_modes = ws.IsTrue(".print_modes");
	bool find_vr = ws.IsTrue(".find.vr.screen");
	int screen_idx = ws.GetInt(".screen", -1);

	dev.hmd_output = ws.Get(".hmd_output", "");
	String primary_output = ws.Get(".primary_output", "DisplayPort-0");
	bool use_display_1 = ws.GetBool(".hmd_use_display_1", true);
	dev.xrandr_setup = ws.GetBool(".xrandr_setup", true);
	
	int x_display_idx = -1;
	String seat_conf = GetHomeDirFile(".ai-upp/hmd_seat.conf");
	bool has_seat_conf = FileExists(seat_conf);

	int x = 0;
	int y = 0;
	unsigned int width = 1280;
	unsigned int height = 720;

	if (has_seat_conf) {
		FileIn in(seat_conf);
		while(!in.IsEof()) {
			String line = in.GetLine();
			if(line.StartsWith("HMD_DISPLAY=")) {
				String d = line.Mid(12);
				if(d.StartsWith(":")) x_display_idx = ScanInt(d.Mid(1));
			}
			if(line.StartsWith("HMD_OUTPUT=") && dev.hmd_output.IsEmpty()) {
				dev.hmd_output = TrimBoth(line.Mid(11));
			}
		}
		in.Close();
	}
	
	if (x_display_idx < 0) x_display_idx = 1;

	if (!dev.desktop_with_hmd && dev.xrandr_setup) {
		// 1. Auto-detect HMD if not specified (look for disconnected device with VR resolution)
		if (dev.hmd_output.IsEmpty()) {
			String tmp_file = GetTempFileName();
			if (system("xrandr > " + tmp_file) == 0) {
				FileIn in(tmp_file);
				String current_output;
				while(!in.IsEof()) {
					String line = in.GetLine();
					if (line.IsEmpty()) continue;
					if (!IsSpace(line[0])) {
						int space_idx = line.Find(' ');
						if (space_idx > 0)
							current_output = line.Left(space_idx);
					}
					// Look for the resolution in the supported modes of any output
					if (line.Find("2880x1440") >= 0 && !current_output.IsEmpty()) {
						dev.hmd_output = current_output;
						LOG("ScrX11Ogl::SinkDevice_Initialize: detected HMD on " << dev.hmd_output);
						break;
					}
				}
				in.Close();
				FileDelete(tmp_file);
			}
		}

		if (!dev.hmd_output.IsEmpty()) {
			// 2. Check if X server is already running on target display
			String disp_name = Format(":%d", x_display_idx);
			bool is_active = false;
			::Display* d = XOpenDisplay(disp_name);
			if (d) {
				is_active = true;
				XCloseDisplay(d);
			}
			else {
				// If XOpenDisplay fails, it might be due to permissions but the server could still be active.
				// Check for the lock file as a fallback.
				if (FileExists(Format("/tmp/.X%d-lock", x_display_idx))) {
					is_active = true;
				}
			}

			if (is_active) {
				LOG("ScrX11Ogl::SinkDevice_Initialize: X server " << disp_name << " already active, skipping start");
				dev.started_hmd = false; // We didn't start it
				dev.x_display_idx = x_display_idx;
			}
			else {
				// 3. Disconnect from current desktop if it's there
				String cmd_off = "sudo xrandr --output " + dev.hmd_output + " --off";
				LOG("ScrX11Ogl::SinkDevice_Initialize: ensuring HMD off on current desktop: " << cmd_off);
				IGNORE_RESULT(system(cmd_off));

				// 4. Start X server
				LOG("ScrX11Ogl::SinkDevice_Initialize: starting X server on " << disp_name);
				String cmd_x = Format("sudo X %s -ac -nolisten tcp -extension GLX &", disp_name);
				IGNORE_RESULT(system(cmd_x));
				dev.started_hmd = true;
				dev.x_display_idx = x_display_idx;
				Sleep(2000);
			}

			// 5. Configure HMD on the target display (whether we started it or not)
			String cfg_cmd = Format("env DISPLAY=:%d xrandr --output %s --mode 2880x1440 --primary", x_display_idx, dev.hmd_output);
			LOG("ScrX11Ogl::SinkDevice_Initialize: configuring HMD on " << disp_name << ": " << cfg_cmd);
			IGNORE_RESULT(system(cfg_cmd));
			Sleep(1000);
		}
	}
	
	::Display*& display = ctx.display;	// pointer to X Display structure.
	::Window& win = ctx.win;			// pointer to the newly created window.
	::XVisualInfo*& visual = ctx.visual_info;
	unsigned int display_width,
	             display_height;		// height and width of the X display.
	
	String display_str = ws.Get(".display", getenv("DISPLAY"));
	if (!dev.desktop_with_hmd && x_display_idx > 0)
		display_str = Format(":%d", x_display_idx);
	char *display_name = (char*)~display_str; // address of the X display.
	
	bool reverse_video = false;
	
	// open connection with the X server.
	display = XOpenDisplay(display_name);
	if (display == NULL) {
		LOG("ScrX11Ogl::SinkDevice_Initialize: error: cannot connect to X server '" << display_name << "'");
		return false;
	}
	int screen_num = DefaultScreen(display);
	
	
	// Borderless & fullscreen X11 window (https://www.tonyobryan.com/index.php?article=9)
	// Used for secondary display fullscreen
	Hints     hints;
    ::Atom    hints_property;
    hints.flags = 2;        // Specify that we're changing the window decorations.
	hints.decorations = 0;  // 0 (false) means that window decorations should go bye-bye.
	hints_property = XInternAtom(display,"_MOTIF_WM_HINTS", True);
	
	
	GLint majorGLX, minorGLX = 0;
	glXQueryVersion(display, &majorGLX, &minorGLX);
	if (majorGLX <= 1 && minorGLX < 2) {
	    LOG("ScrX11Ogl::SinkDevice_Initialize: error: GLX 1.2 or greater is required.");
	    XCloseDisplay(display);
	    return false;
	}
	else {
	    LOG("ScrX11Ogl::SinkDevice_Initialize: GLX version: " << majorGLX << "." << minorGLX);
	}
	
	
	GLint glx_attribs[] = {
		GLX_X_RENDERABLE    , True,
		GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
		GLX_RENDER_TYPE     , GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
		GLX_RED_SIZE        , 8,
		GLX_GREEN_SIZE      , 8,
		GLX_BLUE_SIZE       , 8,
		GLX_ALPHA_SIZE      , 8,
		GLX_DEPTH_SIZE      , 24,
		GLX_STENCIL_SIZE    , 8,
		GLX_DOUBLEBUFFER    , True,
		0
	};

	
	// choose display
	int fbcount;
	GLXFBConfig* fbc = glXChooseFBConfig(display, screen_num, glx_attribs, &fbcount);
	if (fbc == 0) {
		LOG("ScrX11Ogl::SinkDevice_Initialize: error: failed to retrieve framebuffer.");
		XCloseDisplay(display);
		return false;
	}

	// Pick the FB config/visual with the most samples per pixel
	int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
	for (int i = 0; i < fbcount; ++i) {
		XVisualInfo *vi = glXGetVisualFromFBConfig( display, fbc[i] );
		if ( vi != 0) {
			int samp_buf, samples;
			glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
			glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES       , &samples  );

			if ( best_fbc < 0 || (samp_buf && samples > best_num_samp) ) {
				best_fbc = i;
				best_num_samp = samples;
			}
			if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
				worst_fbc = i;
			worst_num_samp = samples;
		}
		XFree(vi);
	}
	GLXFBConfig bestFbc = fbc[best_fbc];
	XFree(fbc); // Make sure to free this!
	
	
	// get the geometry of the default screen for our display.
	display_width	= DisplayWidth(display, screen_num);
	display_height	= DisplayHeight(display, screen_num);
	int dplanes		= DisplayPlanes(display, screen_num);
	
	visual			= glXGetVisualFromFBConfig( display, bestFbc );
	if (visual == 0) {
		LOG("ScrX11Ogl::SinkDevice_Initialize: error: Could not create correct visual window.");
		XCloseDisplay(display);
		return false;
	}
	
	// Store the visual info in the context for proper cleanup
	ctx.visual_info = visual;
	
	if (screen_num != visual->screen) {
		LOG("ScrX11Ogl::SinkDevice_Initialize: error: screen_num(" << screen_num << ") does not match visual->screen(" << visual->screen << ").");
		XCloseDisplay(display);
		return false;
	}
	
	// Pre-calculate target screen coordinates
	bool is_extended_hmd = false;
	if (x > 0) {
		// Coordinates already forced by xrandr detection above
		is_extended_hmd = true;
	}
	else if ((find_vr || screen_idx >= 0 || !dev.hmd_output.IsEmpty()) && is_fullscreen) {
		Index<Rect> screens;
		XRRScreenResources *xrrr = XRRGetScreenResources(display, RootWindow(display, screen_num));
		XRRCrtcInfo *xrrci;
		int i;
		int ncrtc = xrrr->ncrtc;
		for (i = 0; i < ncrtc; ++i) {
			xrrci = XRRGetCrtcInfo(display, xrrr, xrrr->crtcs[i]);
			if (xrrci->width > 0 && xrrci->height > 0) {
				bool match = false;
				if (!dev.hmd_output.IsEmpty()) {
					for (int j = 0; j < xrrci->noutput; j++) {
						XRROutputInfo *oi = XRRGetOutputInfo(display, xrrr, xrrci->outputs[j]);
						if (oi) {
							if (String(oi->name) == dev.hmd_output) {
								match = true;
							}
							XRRFreeOutputInfo(oi);
						}
						if (match) break;
					}
				}
				else if (find_vr && xrrci->width == 1440*2 && xrrci->height == 1440) {
					match = true;
				}
				
				if (match)
					screen_idx = screens.GetCount();
					
				screens.FindAdd(RectC(xrrci->x, xrrci->y, xrrci->width, xrrci->height));
			}
			XRRFreeCrtcInfo(xrrci);
		}
		XRRFreeScreenResources(xrrr);
		
		if (screen_idx >= 0 && screen_idx < screens.GetCount()) {
			Rect r = screens[screen_idx];
			x = r.left;
			y = r.top;
			width = r.GetWidth();
			height = r.GetHeight();
			is_extended_hmd = true;
			LOG("ScrX11Ogl::SinkDevice_Initialize: target screen " << screen_idx << " at " << AsString(r));
		}
		else {
			LOG("ScrX11Ogl::SinkDevice_Initialize: error: screen index out of range (is HMD connected?)");
			XCloseDisplay(display);
			return false;
		}
	}
	
	RTLOG("ScrX11Ogl::SinkDevice_Initialize: creating window at [" << x << "," << y << "] size " << width << "x" << height);
	
	// create a simple window, as a direct child of the screen's
	// root window. Use the screen's white color as the background
	// color of the window. Place the new window's top-left corner
	// at the given 'x,y' coordinates.
	{
		int win_border_width = 0;
		
		// Set window attributes
		XSetWindowAttributes& attr = ctx.attr;
		attr.border_pixel = BlackPixel(display, screen_num);
		attr.background_pixel = WhitePixel(display, screen_num);
		attr.override_redirect = is_extended_hmd ? True : False;
		attr.colormap = XCreateColormap(display, RootWindow(display, screen_num), visual->visual, AllocNone);
		attr.event_mask = ExposureMask;
		
		unsigned long mask = CWBackPixel | CWColormap | CWBorderPixel | CWEventMask;
		if (is_extended_hmd) mask |= CWOverrideRedirect;

		// Open window
		win = XCreateWindow(
			display, RootWindow(display, screen_num),
			x, y, width, height,
			win_border_width, visual->depth,
			InputOutput, visual->visual,
			mask,
			&attr);
		
		// Redirect Close
		ctx.atomWmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(display, win, &ctx.atomWmDeleteWindow, 1);
		
		// Enable input
		XSelectInput(display, win, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
		//ctx.xkb = XkbGetMap(display, XkbAllClientInfoMask, XkbUseCoreKbd);
		
		dev.ctx->wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(display, win, &dev.ctx->wmDeleteMessage, 1);
		
		// Create GLX OpenGL context
		typedef GLXContext (*glXCreateContextAttribsARBProc)(::Display*, GLXFBConfig, GLXContext, Bool, const int*);
		glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
		glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
		if (glXCreateContextAttribsARB == 0) {
			LOG("ScrX11Ogl::SinkDevice_Initialize: warning: glXCreateContextAttribsARB() not found.");
		}
		
		// Create OpenGL context
		#if CPU_ARM
		// gles 2.0
		int context_attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
			GLX_CONTEXT_MINOR_VERSION_ARB, 0,
			GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_ES_PROFILE_BIT_EXT | GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
			None
		};
		#else
		// opengl 4.2 - use compatibility profile to allow deprecated functions
		int context_attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
			GLX_CONTEXT_MINOR_VERSION_ARB, 2,
			GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
			None
		};
		#endif
		
		dev.gl_ctx = 0;
		const char *glxExts = glXQueryExtensionsString( display,  screen_num );
		LOG("ScrX11Ogl::SinkDevice_Initialize: Late extensions:");
		Vector<String> ext_list = Split(glxExts, " ");
		for (const String& s : ext_list) {LOG("\t" << s);}
		
		if (!X11Ogl_IsExtensionSupported( glxExts, "GLX_ARB_create_context")) {
			LOG("ScrX11Ogl::SinkDevice_Initialize: warning: GLX_ARB_create_context not supported");
			dev.gl_ctx = glXCreateNewContext( display, bestFbc, GLX_RGBA_TYPE, 0, True );
		}
		else {
			LOG("ScrX11Ogl::SinkDevice_Initialize: creating context via glXCreateContextAttribsARB");
			dev.gl_ctx = glXCreateContextAttribsARB( display, bestFbc, 0, true, context_attribs );
		}
		XSync( display, False );
		
		
		// Verifying that context is a direct context
		if (!glXIsDirect (display, dev.gl_ctx)) {
			LOG("ScrX11Ogl::SinkDevice_Initialize: warning: indirect GLX rendering context obtained");
		}
		else {
			LOG("ScrX11Ogl::SinkDevice_Initialize: direct GLX rendering context obtained, thread ID=" << (uint64)UPP::Thread::GetCurrentId());
		}
		glXMakeCurrent(display, win, dev.gl_ctx);


		
		LOG("GL Vendor: " << (const char*)glGetString(GL_VENDOR));
		LOG("GL Renderer: " << (const char*)glGetString(GL_RENDERER));
		LOG("GL Version: " << (const char*)glGetString(GL_VERSION));
		LOG("GL Shading Language: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		
		
		// Glew
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			LOG("Glew error: " << (const char*)glewGetErrorString(err));
			return false;
		}
		
		#ifdef flagDEBUG
		{GLenum gerr = glGetError(); if(gerr != GL_NO_ERROR) LOG("glewInit generated error: " << HexStr(gerr));}
		#endif
		
	
		// make the window actually appear on the screen.
		XMapWindow(display, win);
		
		// Configure borderless & fullscreen related properties
		if (is_fullscreen) {
			if (is_extended_hmd) {
				XChangeProperty(display, win, hints_property, hints_property, 32, PropModeReplace, (unsigned char *)&hints, 5);
				XMoveResizeWindow(display, win, x, y, width, height);
				XMapRaised(display, win);
			}
			else {
				::Atom wm_state = XInternAtom (display, "_NET_WM_STATE", true );
				::Atom wm_fullscreen = XInternAtom (display, "_NET_WM_STATE_FULLSCREEN", true );
				XChangeProperty(display, win, wm_state, ((::Atom) 4), 32,
				    PropModeReplace, (unsigned char *)&wm_fullscreen, 1);
			}
		}
		
		// flush all pending requests to the X server.
		XFlush(display);
	}
	
	VideoFormat::default_width = width;
	VideoFormat::default_height = height;
	
	XSync(display, False);
	
	
	dev.accel.SetNative(ctx.display, ctx.win, 0, 0);
	
	if (!dev.accel.Open(Size(width, height), 4, true)) {
		LOG("ScrX11Ogl::SinkDevice_Initialize: error: could not open opengl atom");
		return false;
	}
	
	// Clear any errors generated during initialization (e.g. by glewInit or driver quirks)
	while(glGetError() != GL_NO_ERROR);

	return true;
}

bool ScrX11Ogl::SinkDevice_PostInitialize(NativeSinkDevice& dev, AtomBase& a) {
	return dev.accel.PostInitialize();
}

bool ScrX11Ogl::SinkDevice_Start(NativeSinkDevice& dev, AtomBase& a) {
	return true;
}

void ScrX11Ogl::SinkDevice_Stop(NativeSinkDevice& dev, AtomBase& a) {
	// Don't access ctx here - it may already be destroyed during VFS teardown
}

void ScrX11Ogl::SinkDevice_Uninitialize(NativeSinkDevice& dev, AtomBase& a) {
	dev.accel.Uninitialize();

	if (dev.started_hmd) {
		String stop_script = GetHomeDirFile(".ai-upp/bin/stop_hmd_x.sh");
		if (FileExists(stop_script)) {
			LOG("ScrX11Ogl::SinkDevice_Uninitialize: executing " << stop_script);
			IGNORE_RESULT(system(stop_script));
		}
		else if (!dev.desktop_with_hmd && dev.xrandr_setup) {
			LOG("ScrX11Ogl::SinkDevice_Uninitialize: turning off HMD output " << dev.hmd_output);
			IGNORE_RESULT(system("sudo xrandr --output " + dev.hmd_output + " --off"));
			
			if (dev.x_display_idx > 0) {
				LOG("ScrX11Ogl::SinkDevice_Uninitialize: killing X server on :" << dev.x_display_idx);
				// Find PID from lock file
				String lock_file = Format("/tmp/.X%d-lock", dev.x_display_idx);
				FileIn in(lock_file);
				if (in.IsOpen()) {
					String pid_str = in.GetLine();
					if (!pid_str.IsEmpty()) {
						// The lock file format is text containing the PID as a decimal ASCII number (11 chars).
						pid_str = TrimBoth(pid_str); // Remove spaces/newlines
						int pid = ScanInt(pid_str);
						if (pid > 0) {
							String cmd = Format("sudo kill %d", pid);
							LOG("ScrX11Ogl::SinkDevice_Uninitialize: executing " << cmd);
							IGNORE_RESULT(system(cmd));
						}
					}
					in.Close();
				}
				// Fallback: simple kill command pattern if lock file was gone or unreadable
				else {
					String cmd = Format("sudo pkill -f \"X :%d\"", dev.x_display_idx);
					LOG("ScrX11Ogl::SinkDevice_Uninitialize: fallback kill: " << cmd);
					IGNORE_RESULT(system(cmd));
				}
			}
		}
	}

	// Don't access ctx here - it may already be destroyed during VFS teardown
}

bool ScrX11Ogl::SinkDevice_Recv(NativeSinkDevice& dev, AtomBase&, int ch_i, const Packet& p) {
	return dev.accel.Recv(ch_i, p);
}

void ScrX11Ogl::SinkDevice_Finalize(NativeSinkDevice& dev, AtomBase&, RealtimeSourceConfig& cfg) {
	auto& ctx = *dev.ctx;
	
	if (!ctx.running)
		return;
	
}

bool ScrX11Ogl::SinkDevice_Send(NativeSinkDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {

	if (!dev.accel.Send(cfg, out, src_ch))
		return false;

	// Average color logging for debugging
	if (dev.log_avg) {
		dev.frame_counter++;
		if (dev.frame_counter % dev.avg_log_interval == 0) {
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);
			int width = viewport[2];
			int height = viewport[3];

			if (width > 0 && height > 0) {
				// Read back framebuffer from GPU
				Vector<byte> pixels;
				pixels.SetCount(width * height * 3);
				glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.Begin());

				// Calculate average color
				uint64 sum_r = 0, sum_g = 0, sum_b = 0;
				const byte* px = pixels.Begin();
				for (int i = 0; i < width * height; i++) {
					sum_r += px[0];
					sum_g += px[1];
					sum_b += px[2];
					px += 3;
				}
				int pixel_count = width * height;
				int avg_r = (int)(sum_r / pixel_count);
				int avg_g = (int)(sum_g / pixel_count);
				int avg_b = (int)(sum_b / pixel_count);

				// Use LOG instead of GFXLOG so output is visible when avg_color_log=true
				LOG("X11Ogl frame #" << dev.frame_counter << " avg color (RGB)=" << avg_r << "," << avg_g << "," << avg_b);
			}
		}
	}

	return true;
}

bool ScrX11Ogl::SinkDevice_NegotiateSinkFormat(NativeSinkDevice& dev, AtomBase& a, LinkBase& link, int sink_ch, const ValueFormat& new_fmt) {
	// accept all valid video formats for now
	if (new_fmt.IsValid() && new_fmt.IsVideo()) {
		ISinkPtr sink = a.GetSink();
		ValueBase& val = sink->GetValue(sink_ch);
		val.SetFormat(new_fmt);
		return true;
	}
	return false;
}

bool ScrX11Ogl::SinkDevice_IsReady(NativeSinkDevice& dev, AtomBase&, PacketIO& io) {
	return true;
}





bool ScrX11Ogl::Context_Create(NativeContext*& dev) {
	dev = new NativeContext;
	return true;
}

void ScrX11Ogl::Context_Destroy(NativeContext*& dev) {
	delete dev;
	dev = 0;
}

void ScrX11Ogl::Context_Visit(NativeContext& dev, AtomBase&, Visitor& vis) {
	
}

bool ScrX11Ogl::Context_Initialize(NativeContext& ctx, AtomBase& a, const WorldState& ws) {
	ctx.running = true;
	return true;
}

bool ScrX11Ogl::Context_PostInitialize(NativeContext& ctx, AtomBase& a) {
	return true;
}

bool ScrX11Ogl::Context_Start(NativeContext& ctx, AtomBase& a) {
	return true;
}

void ScrX11Ogl::Context_Stop(NativeContext& ctx, AtomBase& a) {
	
}

void ScrX11Ogl::Context_Uninitialize(NativeContext& ctx, AtomBase& a) {
	
}

bool ScrX11Ogl::Context_Send(NativeContext& ctx, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	return false;
}

bool ScrX11Ogl::Context_Recv(NativeContext& ctx, AtomBase& a, int sink_ch, const Packet&) {
	return false;
}

void ScrX11Ogl::Context_Finalize(NativeContext& ctx, AtomBase& a, RealtimeSourceConfig&) {
	
}

bool ScrX11Ogl::Context_NegotiateSinkFormat(NativeContext& ctx, AtomBase& a, LinkBase& link, int sink_ch, const ValueFormat& new_fmt) {
	return false;
}

bool ScrX11Ogl::Context_IsReady(NativeContext& dev, AtomBase&, PacketIO& io) {
	return true;
}


#define ABBR Ogl
#define X11IMPL 1
#include "Impl.inl"
#undef ABBR

END_UPP_NAMESPACE
#endif

