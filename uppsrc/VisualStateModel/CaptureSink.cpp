#include "VisualStateModel.h"

namespace Upp {

VsmCaptureSummary VsmCaptureSink::Record(VsmFrameSource& src,
                                          const VsmCaptureSinkOptions& opts)
{
	VsmCaptureSummary summary;

	if(!src.IsReady()) {
		LogError(log_, "CaptureSink", "Frame source not ready: " + src.GetLastError());
		return summary;
	}
	if(opts.output_dir.IsEmpty()) {
		LogError(log_, "CaptureSink", "output_dir is empty");
		return summary;
	}

	// Determine session_id
	String session_id = opts.session_id.IsEmpty()
	                  ? "capture-" + IntStr((int)GetTickCount())
	                  : opts.session_id;

	// Create output session; dimensions come from the source
	int w = src.GetWidth(), h = src.GetHeight();
	if(w <= 0 || h <= 0) {
		LogError(log_, "CaptureSink",
		         "Source reports invalid dimensions: " + IntStr(w) + "x" + IntStr(h));
		return summary;
	}

	if(!store_.Create(opts.output_dir, session_id, w, h, "capture")) {
		LogError(log_, "CaptureSink", "Cannot create session at: " + opts.output_dir);
		return summary;
	}

	LogInfo(log_, "CaptureSink",
	        "Recording into " + opts.output_dir + " session=" + session_id +
	        " src=" + src.GetSourceInfo());

	int frame_idx = 0;
	VsmImageBuffer img;
	int64 ts_ms = 0;

	while(src.ReadFrame(img, ts_ms)) {
		VsmAssetRef ref = store_.SaveFrameImage(frame_idx, img, ts_ms);
		if(ref.IsEmpty()) {
			summary.frames_dropped++;
			summary.error_count++;
			LogWarn(log_, "CaptureSink", "Drop frame " + IntStr(frame_idx) + " (write error)");
		} else {
			summary.frames_recorded++;
		}
		frame_idx++;
		if(opts.max_frames > 0 && frame_idx >= opts.max_frames)
			break;
	}

	if(!src.GetLastError().IsEmpty())
		LogWarn(log_, "CaptureSink", "Source error: " + src.GetLastError());

	if(!store_.SaveManifest()) {
		LogError(log_, "CaptureSink", "Cannot save manifest");
		return summary;
	}

	summary.success     = true;
	summary.session_id  = session_id;
	summary.output_dir  = opts.output_dir;
	summary.source_info = src.GetSourceInfo();

	LogInfo(log_, "CaptureSink",
	        "Record complete: " + IntStr(summary.frames_recorded) + " frames, " +
	        IntStr(summary.frames_dropped) + " dropped");
	return summary;
}

} // namespace Upp
