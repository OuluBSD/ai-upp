#ifndef _VideoFrameWindowDetector_VideoFrameWindowDetector_h_
#define _VideoFrameWindowDetector_VideoFrameWindowDetector_h_

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <plugin/jpg/jpg.h>

NAMESPACE_UPP

struct DetectorOptions {
	String input;
	String out_dir;
	int    min_title_width = 300;
	int    min_window_height = 120;
	bool   help = false;
};

struct WindowCandidate : Moveable<WindowCandidate> {
	int    index = 0;
	Rect   title;
	Rect   rect;
	String role;
	String crop_path;
};

END_UPP_NAMESPACE

#endif

