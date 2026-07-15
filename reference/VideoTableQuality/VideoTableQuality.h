#ifndef _VideoTableQuality_VideoTableQuality_h_
#define _VideoTableQuality_VideoTableQuality_h_

#include <Core/Core.h>

NAMESPACE_UPP

struct TableQualityOptions {
	String tracker_dir;
	String ocr_json;
	String out_path;
	bool   help = false;
};

struct TableQualityEntry : Moveable<TableQualityEntry> {
	int    frame_index = 0;
	int    table_id = 0;
	bool   has_title_anchor = false;
	bool   has_pot_anchor = false;
	bool   has_obstruction_text = false;
	bool   usable = false;
	String title_text;
	String pot_text;
	String reason;
};

END_UPP_NAMESPACE

#endif

