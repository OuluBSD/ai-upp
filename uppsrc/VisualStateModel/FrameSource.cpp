#include "VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// VsmSessionStoreSource

bool VsmSessionStoreSource::Open(const String& uri)
{
	Close();
	if(!store_.Open(uri)) {
		last_error_ = "Cannot open session: " + uri;
		return false;
	}
	const VsmSessionManifest& m = store_.GetManifest();
	width_    = m.frame_width;
	height_   = m.frame_height;
	cursor_   = 0;
	is_ready_ = true;
	LogInfo(log_, "VsmSessionStoreSource",
	        "Opened: " + m.session_id + " frames=" + IntStr(m.frames.GetCount()) +
	        " " + IntStr(width_) + "x" + IntStr(height_));
	return true;
}

void VsmSessionStoreSource::Close()
{
	is_ready_   = false;
	cursor_     = 0;
	last_error_ = String();
}

bool VsmSessionStoreSource::ReadFrame(VsmImageBuffer& out_frame, int64& out_ts_ms)
{
	if(!is_ready_) {
		last_error_ = "Source not open";
		return false;
	}
	const VsmSessionManifest& m = store_.GetManifest();
	while(cursor_ < m.frames.GetCount()) {
		const VsmFrameAsset& fa = m.frames[cursor_];
		int fi = fa.frame_index;
		cursor_++;
		if(fa.format == "vsm") {
			if(!store_.LoadFrameImage(fi, out_frame)) {
				last_error_ = "Cannot load frame: " + fa.relative_path;
				LogWarn(log_, "VsmSessionStoreSource", last_error_);
				continue; // skip corrupt frame, try next
			}
			out_ts_ms = (int64)fi * 33; // approximate 30 fps
			return true;
		}
		// placeholder — not a real image; skip silently
		LogInfo(log_, "VsmSessionStoreSource",
		        "Skipping placeholder frame " + IntStr(fi));
	}
	return false; // end of stream
}

String VsmSessionStoreSource::GetSourceInfo() const
{
	if(!is_ready_)
		return "VsmSessionStoreSource (not open)";
	const VsmSessionManifest& m = store_.GetManifest();
	return "session:" + m.session_id +
	       " frames=" + IntStr(m.frames.GetCount()) +
	       " " + IntStr(width_) + "x" + IntStr(height_);
}

int VsmSessionStoreSource::GetFrameCount() const
{
	return store_.IsOpen() ? store_.GetManifest().frames.GetCount() : 0;
}

} // namespace Upp
