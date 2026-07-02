#ifndef _VisualStateModel_CaptureSink_h_
#define _VisualStateModel_CaptureSink_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Options for recording a new session from a VsmFrameSource

struct VsmCaptureSinkOptions : Moveable<VsmCaptureSinkOptions> {
	String output_dir;           // destination session directory
	String session_id;           // auto-generated if empty
	int    max_frames    = -1;   // -1 = unlimited
	bool   detect_changes = false; // reserved: run change detect while recording

	void Jsonize(JsonIO& json) {
		json("output_dir",      output_dir)
		    ("session_id",      session_id)
		    ("max_frames",      max_frames)
		    ("detect_changes",  detect_changes);
	}
};

// ---------------------------------------------------------------------------
// Summary returned after a capture run

struct VsmCaptureSummary : Moveable<VsmCaptureSummary> {
	bool   success         = false;
	int    frames_recorded = 0;
	int    frames_dropped  = 0;
	int    error_count     = 0;
	String session_id;
	String output_dir;
	String source_info;

	void Jsonize(JsonIO& json) {
		json("success",          success)
		    ("frames_recorded",  frames_recorded)
		    ("frames_dropped",   frames_dropped)
		    ("error_count",      error_count)
		    ("session_id",       session_id)
		    ("output_dir",       output_dir)
		    ("source_info",      source_info);
	}
};

// ---------------------------------------------------------------------------
// VsmCaptureSink
//
// Records frames from any VsmFrameSource into a new VisualStateModel session.
// Preserves source timestamps where available (nonzero).
// No threading — drives a synchronous pull loop.

class VsmCaptureSink {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }

	// Pull frames from src until end-of-stream or max_frames is reached.
	// Creates output session at opts.output_dir.
	VsmCaptureSummary Record(VsmFrameSource& src, const VsmCaptureSinkOptions& opts);

	// Access the output store after a successful Record call.
	VsmSessionStore& GetStore() { return store_; }

private:
	CoreLog         log_;
	VsmSessionStore store_;
};

} // namespace Upp

#endif
