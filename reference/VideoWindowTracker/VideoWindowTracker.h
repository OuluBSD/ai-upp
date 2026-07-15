#ifndef _VideoWindowTracker_VideoWindowTracker_h_
#define _VideoWindowTracker_VideoWindowTracker_h_

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <plugin/jpg/jpg.h>
#include <VisualStateModel/VisualStateModel.h>

NAMESPACE_UPP

struct TrackerOptions {
	String input_dir;
	String out_dir;
	int    min_title_width = 300;
	int    min_window_height = 120;
	int    block = 24;
	int    pixel_threshold = 35;
	int    min_changed_pixels = 24;
	String table_mode = "unknown";
	bool   help = false;
};

struct TrackerWindow : Moveable<TrackerWindow> {
	int  id = 0;
	Rect rect;
	Rect title;
	String crop_path;
};

struct SemanticCrop : Moveable<SemanticCrop> {
	String name;
	Rect   rect;
	String path;
};

struct TrackerChange : Moveable<TrackerChange> {
	int  window_id = 0;
	Rect rect;
	int  changed_pixels = 0;
};

struct SemanticEvent : Moveable<SemanticEvent> {
	int    frame_index = 0;
	int    table_id = 0;
	String type;
	String reason;
	String semantic;
	int    change_blocks = 0;
	double confidence = 0;
};

END_UPP_NAMESPACE

#endif
