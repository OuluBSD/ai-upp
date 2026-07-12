#include "TexasHoldemSessionAdapter.h"

namespace Upp {

Vector<const VsmRegionNode*> VsmTexasHoldemSession::RegionsForFrame(int frame_id) const
{
	Vector<const VsmRegionNode*> out;
	for(const VsmRegionNode& rn : regions)
		if(rn.frame == frame_id)
			out.Add(&rn);
	return out;
}

const TexasHoldemGroundTruthRecord* VsmTexasHoldemSession::GroundTruthForFrame(int frame_id) const
{
	if(frame_id < 0 || frame_id >= ground_truth.GetCount())
		return nullptr;
	return &ground_truth[frame_id];
}

VsmTexasHoldemLoadResult VsmLoadTexasHoldemSession(const String& root,
                                                   VsmTexasHoldemSession& out,
                                                   AppLog* log)
{
	VsmTexasHoldemLoadResult res;
	out = VsmTexasHoldemSession();
	out.root = root;

	if(!DirectoryExists(root)) {
		res.error = "session directory not found: " + root;
		return res;
	}

	if(!VsmReadM01M02SessionInfo(root, out.info)) {
		res.error = "failed to read M01/M02 metadata.json under: " + root;
		return res;
	}
	res.frame_count = out.info.frame_count;

	// ---- Ground truth: groundtruth.jsonl, one JSON object per line, indexed
	// by frame_id. Densely sized to at least frame_count so GroundTruthForFrame
	// is a direct index. Missing lines leave a default-constructed slot.
	String gt_path = AppendFileName(root, "groundtruth.jsonl");
	if(FileExists(gt_path)) {
		FileIn in(gt_path);
		if(in) {
			while(!in.IsEof()) {
				String line = in.GetLine();
				if(line.IsEmpty())
					continue;
				TexasHoldemGroundTruthRecord rec;
				if(!LoadFromJson(rec, line))
					continue;
				int idx = rec.frame_id;
				if(idx < 0)
					continue;
				if(idx >= out.ground_truth.GetCount())
					out.ground_truth.SetCount(idx + 1);
				out.ground_truth[idx] = pick(rec);
				res.ground_truth_records++;
			}
		}
	}
	if(out.ground_truth.GetCount() < out.info.frame_count)
		out.ground_truth.SetCount(out.info.frame_count);
	out.ground_truth_count = res.ground_truth_records;

	// ---- Region detection across consecutive frames.
	// Mirrors reference/VisualStateRegionDump/main.cpp's M01/M02 path exactly:
	// same VsmChangeDetectParams, same VsmRegionMemory FindNearest(0.3), same
	// rgn-%04d id assignment, same load order.
	if(out.info.frame_count >= 2) {
		VsmChangeDetectParams params;
		params.pixel_threshold = 30;
		params.block_size      = 8;
		params.block_min_score = 0.05;
		params.merge_gap       = 16;
		params.min_region_area = 64;

		VsmRegionMemory mem;
		if(log)
			mem.SetLog(log);

		int rgn_counter = 0;

		VsmFrameImage prev_frame;
		if(!VsmLoadM01M02SessionFrame(root, 0, prev_frame)) {
			res.error = "failed to decode frame 0";
			return res;
		}

		for(int fid = 1; fid <= out.info.frame_count - 1; fid++) {
			VsmFrameImage curr_frame;
			if(!VsmLoadM01M02SessionFrame(root, fid, curr_frame)) {
				res.error = Format("failed to decode frame %d", fid);
				return res;
			}

			Vector<VsmChangedRect> changes = VsmDetectChanges(prev_frame, curr_frame, params);
			res.transitions++;

			for(const VsmChangedRect& cr : changes) {
				VsmFingerprint32 fp;
				if(!VsmRegionMemory::ExtractFingerprint(curr_frame, cr.x, cr.y, cr.w, cr.h, fp)) {
					res.error = Format("ExtractFingerprint failed at frame %d", fid);
					return res;
				}

				VsmRegionMatch match = mem.FindNearest(fp, 0.3);
				bool matched = !match.region_id.IsEmpty();
				VsmRegionId rid;
				if(matched)
					rid = match.region_id;
				else {
					rid = Format("rgn-%04d", ++rgn_counter);
					mem.Add(rid, fp);
				}

				VsmRegionNode& rn = out.regions.Add();
				rn.id              = rid;
				rn.frame           = fid;
				rn.x               = cr.x;
				rn.y               = cr.y;
				rn.w               = cr.w;
				rn.h               = cr.h;
				rn.action          = matched ? "moved" : "created";
				rn.fingerprint.hash = fp.ComputeHash();
			}

			// Copy curr into prev for the next iteration (VsmFrameImage wraps a
			// non-assignable Buffer<byte>, so copy raw bytes — same technique
			// VisualStateRegionDump uses).
			if(prev_frame.width != curr_frame.width || prev_frame.height != curr_frame.height)
				prev_frame.Set(curr_frame.width, curr_frame.height, nullptr);
			memcpy(prev_frame.data, curr_frame.data,
			       (size_t)curr_frame.width * curr_frame.height * 4);
		}

		out.distinct_region_count = mem.GetCount();
	}

	res.region_records   = out.regions.GetCount();
	res.distinct_regions = out.distinct_region_count;
	out.loaded           = true;
	res.success          = true;
	return res;
}

bool VsmLoadTexasHoldemFrameImage(const VsmTexasHoldemSession& s, int frame_id,
                                  VsmFrameImage& out)
{
	if(s.root.IsEmpty())
		return false;
	return VsmLoadM01M02SessionFrame(s.root, frame_id, out);
}

} // namespace Upp
