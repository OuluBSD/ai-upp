#ifndef _VisualStateModel_SessionRegionTimeline_h_
#define _VisualStateModel_SessionRegionTimeline_h_

namespace Upp {

struct VsmSessionRegionRecord : Moveable<VsmSessionRegionRecord> {
	int    frame_prev = -1;
	int    frame      = -1;
	int    x = 0, y = 0, w = 0, h = 0;
	double score      = 0.0;
	String region_id;
	String fingerprint_hash;
	bool   matched = false;
	double match_distance = 0.0;

	VsmChangedRect ToChangedRect() const;
	void Jsonize(JsonIO& json);
};

struct VsmSessionRegionTransition : Moveable<VsmSessionRegionTransition> {
	int frame_prev = -1;
	int frame      = -1;
	Vector<VsmChangedRect>          changes;
	Vector<VsmSessionRegionRecord>  records;
};

struct VsmSessionRegionTimeline : Moveable<VsmSessionRegionTimeline> {
	VsmM01M02SessionInfo            info;
	Vector<VsmSessionRegionRecord>  records;
	Vector<VsmSessionRegionTransition> transitions;
	int distinct_region_count = 0;
	int transitions_processed = 0;
};

struct VsmSessionRegionTimelineOptions : Moveable<VsmSessionRegionTimelineOptions> {
	int frame_start = -1;
	int frame_end   = -1;
	VsmChangeDetectParams params;
	double identity_threshold = 0.3;

	VsmSessionRegionTimelineOptions();
};

struct VsmSessionRegionTimelineResult : Moveable<VsmSessionRegionTimelineResult> {
	bool success = false;
	String error;
	int load_start = -1;
	int frame_start = -1;
	int frame_end = -1;
};

VsmSessionRegionTimelineResult VsmBuildSessionRegionTimeline(
	const String& session_dir,
	const VsmSessionRegionTimelineOptions& options,
	VsmSessionRegionTimeline& out,
	AppLog* log = nullptr);

bool VsmLoadSessionRegionTransitionFrame(const String& session_dir,
                                         const VsmSessionRegionTransition& transition,
                                         VsmFrameImage& out);

} // namespace Upp

#endif
