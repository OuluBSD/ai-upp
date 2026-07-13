#include "VisualStateModel.h"

namespace Upp {

static void CopyFrame(VsmFrameImage& dst, const VsmFrameImage& src)
{
	if(dst.width != src.width || dst.height != src.height)
		dst.Set(src.width, src.height, nullptr);
	memcpy(dst.data, src.data, (size_t)src.width * src.height * 4);
}

VsmChangedRect VsmSessionRegionRecord::ToChangedRect() const
{
	VsmChangedRect out;
	out.x = x;
	out.y = y;
	out.w = w;
	out.h = h;
	out.score = score;
	return out;
}

void VsmSessionRegionRecord::Jsonize(JsonIO& json)
{
	json("frame_prev", frame_prev)
	    ("frame",      frame)
	    ("x", x)("y", y)("w", w)("h", h)
	    ("score",      score)
	    ("region_id",  region_id);
}

VsmSessionRegionTimelineOptions::VsmSessionRegionTimelineOptions()
{
	params.pixel_threshold = 30;
	params.block_size      = 8;
	params.block_min_score = 0.05;
	params.merge_gap       = 16;
	params.min_region_area = 64;
}

VsmSessionRegionTimelineResult VsmBuildSessionRegionTimeline(
	const String& session_dir,
	const VsmSessionRegionTimelineOptions& options,
	VsmSessionRegionTimeline& out,
	AppLog* log)
{
	VsmSessionRegionTimelineResult result;
	out = VsmSessionRegionTimeline();

	if(!DirectoryExists(session_dir)) {
		result.error = Format("Session directory not found: %s", session_dir);
		return result;
	}

	if(!VsmReadM01M02SessionInfo(session_dir, out.info)) {
		result.error = Format("Failed to read M01/M02 session metadata.json under: %s", session_dir);
		return result;
	}

	if(out.info.frame_count < 2) {
		result.error = "Session has fewer than 2 frames — no transitions to detect";
		return result;
	}

	int fs = options.frame_start >= 0 ? options.frame_start : 0;
	int fe = options.frame_end   >= 0 ? options.frame_end   : out.info.frame_count - 1;
	if(fs < 1)
		fs = 1;
	if(fe > out.info.frame_count - 1)
		fe = out.info.frame_count - 1;
	if(fs > fe) {
		result.error = Format("Invalid frame range: --frame-start resolves to %d > --frame-end %d", fs, fe);
		return result;
	}

	int load_start = fs - 1;
	result.load_start = load_start;
	result.frame_start = fs;
	result.frame_end = fe;

	VsmRegionMemory mem;
	if(log)
		mem.SetLog(log);

	int rgn_counter = 0;

	VsmFrameImage prev_frame;
	if(!VsmLoadM01M02SessionFrame(session_dir, load_start, prev_frame)) {
		result.error = Format("Failed to decode frame %d", load_start);
		return result;
	}

	for(int fid = fs; fid <= fe; fid++) {
		VsmFrameImage curr_frame;
		if(!VsmLoadM01M02SessionFrame(session_dir, fid, curr_frame)) {
			result.error = Format("Failed to decode frame %d", fid);
			return result;
		}

		VsmSessionRegionTransition& transition = out.transitions.Add();
		transition.frame_prev = fid - 1;
		transition.frame = fid;
		transition.changes = VsmDetectChanges(prev_frame, curr_frame, options.params);
		out.transitions_processed++;

		for(const VsmChangedRect& cr : transition.changes) {
			VsmFingerprint32 fp;
			if(!VsmRegionMemory::ExtractFingerprint(curr_frame, cr.x, cr.y, cr.w, cr.h, fp)) {
				result.error = Format("ExtractFingerprint frame %d", fid);
				return result;
			}

			VsmRegionMatch match = mem.FindNearest(fp, options.identity_threshold);
			VsmRegionId rid;
			if(!match.region_id.IsEmpty())
				rid = match.region_id;
			else {
				rid = Format("rgn-%04d", ++rgn_counter);
				mem.Add(rid, fp);
			}

			VsmSessionRegionRecord rec;
			rec.frame_prev = fid - 1;
			rec.frame      = fid;
			rec.x = cr.x;
			rec.y = cr.y;
			rec.w = cr.w;
			rec.h = cr.h;
			rec.score = cr.score;
			rec.region_id = rid;
			rec.fingerprint_hash = fp.ComputeHash();
			rec.matched = !match.region_id.IsEmpty();
			rec.match_distance = match.distance;
			out.records.Add(rec);
			transition.records.Add(rec);
		}

		CopyFrame(prev_frame, curr_frame);
	}

	out.distinct_region_count = mem.GetCount();
	result.success = true;
	return result;
}

bool VsmLoadSessionRegionTransitionFrame(const String& session_dir,
                                         const VsmSessionRegionTransition& transition,
                                         VsmFrameImage& out)
{
	return VsmLoadM01M02SessionFrame(session_dir, transition.frame, out);
}

} // namespace Upp
