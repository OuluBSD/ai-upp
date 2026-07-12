#include "VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// VsmLiveM01M02SessionSource

String VsmLiveM01M02SessionSource::frame_path(int frame_id) const
{
	// Stable, documented contract literal (same "%08d.png" as PngFrame.cpp).
	String frame_name = Format("%08d.png", frame_id);
	return AppendFileName(AppendFileName(root_, "frames"), frame_name);
}

// Read metadata.json for the standard M01/M02 fields. Reuses
// VsmReadM01M02SessionInfo (width/height/frame_count/provider/session_id). Kept
// separate from the "status" query because status is polled repeatedly and can
// change from "recording" to "complete" over the source's lifetime, whereas
// these fields are fixed once the recorder writes metadata up front.
bool VsmLiveM01M02SessionSource::read_metadata()
{
	VsmM01M02SessionInfo info;
	if(!VsmReadM01M02SessionInfo(root_, info))
		return false;
	width_       = info.table_width;
	height_      = info.table_height;
	frame_count_ = info.frame_count;
	provider_    = info.provider;
	session_id_  = info.session_id;
	return true;
}

bool VsmLiveM01M02SessionSource::Open(const String& uri)
{
	Close();
	root_ = uri;

	if(!DirectoryExists(root_)) {
		last_error_ = "Session directory does not exist: " + root_;
		return false;
	}
	// metadata.json is written up front by the recorder (task 0137), so it must
	// be readable even for a just-started session with zero frames on disk yet.
	if(!read_metadata()) {
		last_error_ = "Cannot read metadata.json (session not started yet?): " + root_;
		return false;
	}
	cursor_   = 0;
	is_ready_ = true;
	return true;
}

void VsmLiveM01M02SessionSource::Close()
{
	is_ready_    = false;
	root_        = String();
	provider_    = String();
	session_id_  = String();
	width_       = 0;
	height_      = 0;
	frame_count_ = 0;
	cursor_      = 0;
	last_error_  = String();
}

bool VsmLiveM01M02SessionSource::IsRecordingComplete() const
{
	// Re-read from disk each call so a long-lived source observes the recorder's
	// final "status":"complete" rewrite. Absence of the field (old fixtures) ==
	// complete, never "recording forever".
	String meta_path = AppendFileName(root_, "metadata.json");
	if(!FileExists(meta_path))
		return false;
	Value meta_value;
	try {
		meta_value = ParseJSON(LoadFile(meta_path));
	}
	catch(...) {
		return false;
	}
	if(!meta_value.Is<ValueMap>())
		return false;
	ValueMap meta = meta_value;
	String status = (String)meta.Get("status", "complete");
	return status != "recording";
}

int VsmLiveM01M02SessionSource::GetAvailableFrameCount() const
{
	if(!is_ready_)
		return 0;
	int n = 0;
	while(n < frame_count_ && FileExists(frame_path(n)))
		n++;
	return n;
}

VsmLiveM01M02SessionSource::LiveResult
VsmLiveM01M02SessionSource::ReadFrameLive(VsmImageBuffer& out_frame, int64& out_ts_ms)
{
	if(!is_ready_) {
		last_error_ = "Source not open";
		return LIVE_EOS;
	}

	// All expected frames already consumed → genuinely done.
	if(cursor_ >= frame_count_)
		return LIVE_EOS;

	String path = frame_path(cursor_);
	bool complete = IsRecordingComplete();

	if(!FileExists(path)) {
		// Next expected frame not written yet. If the recorder says it is
		// complete but the frame still isn't there, treat as end-of-stream to
		// avoid waiting forever; otherwise ask the caller to poll/retry.
		if(complete) {
			last_error_ = "Recording complete but frame missing: " + path;
			return LIVE_EOS;
		}
		return LIVE_PENDING;
	}

	// The PNG exists but may be a partial write still in flight. Reuse
	// VsmLoadPngFrame for decode (do not reimplement). A decode failure on an
	// in-progress recording means "not fully written yet" → retry; on a complete
	// recording it is a genuine error we surface as EOS.
	VsmFrameImage img;
	if(!VsmLoadPngFrame(path, img)) {
		if(complete) {
			last_error_ = "Failed to decode frame: " + path;
			return LIVE_EOS;
		}
		return LIVE_PENDING;
	}

	out_frame.Create(img.width, img.height, 4);
	memcpy(out_frame.pixels.begin(), ~img.data, (size_t)img.width * img.height * 4);

	// Opportunistic per-frame ground-truth timestamp if its JSONL line already
	// exists; otherwise fall back to a synthetic monotonic estimate.
	out_ts_ms = (int64)cursor_ * 100;
	String gt_line;
	if(TryReadGroundTruth(cursor_, gt_line)) {
		try {
			Value v = ParseJSON(gt_line);
			if(v.Is<ValueMap>()) {
				ValueMap gt = v;
				Value ts = gt.Get("timestamp_ms", Value());
				if(!IsNull(ts))
					out_ts_ms = (int64)ts;
			}
		}
		catch(...) {
			// keep synthetic fallback
		}
	}

	cursor_++;
	return LIVE_OK;
}

bool VsmLiveM01M02SessionSource::ReadFrame(VsmImageBuffer& out_frame, int64& out_ts_ms)
{
	return ReadFrameLive(out_frame, out_ts_ms) == LIVE_OK;
}

bool VsmLiveM01M02SessionSource::TryReadGroundTruth(int frame_id, String& out_json) const
{
	if(!is_ready_)
		return false;
	String gt_path = AppendFileName(root_, "groundtruth.jsonl");
	if(!FileExists(gt_path))
		return false;
	// groundtruth.jsonl grows one line per recorded frame; re-read each call
	// (small file). A trailing partial line (mid-flush) simply fails to parse and
	// is ignored — never blocks.
	String data = LoadFile(gt_path);
	Vector<String> rows = Split(data, '\n', false);
	for(String row : rows) {
		row = TrimBoth(row);
		if(row.IsEmpty())
			continue;
		Value v;
		try {
			v = ParseJSON(row);
		}
		catch(...) {
			continue;
		}
		if(!v.Is<ValueMap>())
			continue;
		ValueMap gt = v;
		if((int)gt.Get("frame_id", -1) == frame_id) {
			out_json = row;
			return true;
		}
	}
	return false;
}

String VsmLiveM01M02SessionSource::GetSourceInfo() const
{
	if(!is_ready_)
		return "VsmLiveM01M02SessionSource (not open)";
	return "live-session:" + session_id_ +
	       " provider=" + provider_ +
	       " frames=" + IntStr(cursor_) + "/" + IntStr(frame_count_) +
	       " " + IntStr(width_) + "x" + IntStr(height_) +
	       (IsRecordingComplete() ? " [complete]" : " [recording]");
}

} // namespace Upp
