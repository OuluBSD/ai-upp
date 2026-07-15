#ifndef _VideoEventCorrelator_VideoEventCorrelator_h_
#define _VideoEventCorrelator_VideoEventCorrelator_h_

#include <Core/Core.h>

NAMESPACE_UPP

struct EventCorrelatorOptions {
	String tracker_dir;
	String events_json;
	String out_path;
	int    frame_gap = 1;
	bool   help = false;
};

struct RawVideoEvent : Moveable<RawVideoEvent> {
	int    frame_index = 0;
	int    table_id = 0;
	String type;
	String reason;
	String semantic;
	int    change_blocks = 0;
	double confidence = 0;
};

struct CorrelatedVideoEvent : Moveable<CorrelatedVideoEvent> {
	int    id = 0;
	int    start_frame = 0;
	int    end_frame = 0;
	int    table_id = 0;
	String kind;
	Vector<String> types;
	Vector<String> semantics;
	int    raw_event_count = 0;
	int    change_blocks = 0;
	double confidence = 0;
};

END_UPP_NAMESPACE

#endif

