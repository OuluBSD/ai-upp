#ifndef _VisualStateModel_FrameSource_h_
#define _VisualStateModel_FrameSource_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Abstract frame source interface

class VsmFrameSource {
public:
	virtual ~VsmFrameSource() {}

	virtual bool   Open(const String& uri) = 0;
	virtual void   Close() = 0;
	virtual bool   IsReady()  const = 0;
	virtual int    GetWidth() const = 0;
	virtual int    GetHeight()const = 0;
	virtual int    GetFPS()   const = 0; // 0 if unknown

	// Read next frame into out_frame; set out_ts_ms to millisecond timestamp.
	// Returns false at end of stream or on unrecoverable error.
	virtual bool   ReadFrame(VsmImageBuffer& out_frame, int64& out_ts_ms) = 0;

	virtual String GetLastError()  const { return String(); }
	virtual String GetSourceInfo() const { return String(); }
};

// ---------------------------------------------------------------------------
// Source info metadata

struct VsmFrameSourceInfo : Moveable<VsmFrameSourceInfo> {
	String source_type;
	String uri;
	int    width       = 0;
	int    height      = 0;
	int    fps         = 0;
	int    frame_count = -1; // -1 if unknown / live

	void Jsonize(JsonIO& json) {
		json("source_type", source_type)("uri", uri)
		    ("width",       width)      ("height",      height)
		    ("fps",         fps)        ("frame_count", frame_count);
	}
};

// ---------------------------------------------------------------------------
// VsmSessionStoreSource — replays an existing VSM session directory

class VsmSessionStoreSource : public VsmFrameSource {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }

	bool   Open(const String& uri) override;
	void   Close() override;
	bool   IsReady()  const override { return is_ready_; }
	int    GetWidth() const override { return width_; }
	int    GetHeight()const override { return height_; }
	int    GetFPS()   const override { return 0; } // unknown from manifest

	// Reads vsm frames in manifest order; skips placeholder frames.
	// out_ts_ms is approximated as frame_index * 33 ms when not available.
	bool   ReadFrame(VsmImageBuffer& out_frame, int64& out_ts_ms) override;

	String GetLastError()  const override { return last_error_; }
	String GetSourceInfo() const override;

	int GetFrameCount() const;

private:
	VsmSessionStore store_;
	CoreLog         log_;
	bool            is_ready_  = false;
	int             width_     = 0;
	int             height_    = 0;
	int             cursor_    = 0;  // index into manifest.frames
	String          last_error_;
};

} // namespace Upp

#endif
