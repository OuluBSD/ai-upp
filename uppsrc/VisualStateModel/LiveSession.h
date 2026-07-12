#ifndef _VisualStateModel_LiveSession_h_
#define _VisualStateModel_LiveSession_h_

namespace Upp {

// ---------------------------------------------------------------------------
// VsmLiveM01M02SessionSource — a VsmFrameSource that TAILS a growing M01/M02
// TexasHoldem session directory (metadata.json + groundtruth.jsonl +
// frames/%08d.png) while `TexasHoldem.exe --record-session` is still writing
// it, instead of requiring the whole session to be finished first (task 0137,
// M07). It also fully reads an already-complete session as a drop-in
// replacement for the one-shot VsmReadM01M02SessionInfo/
// VsmLoadM01M02SessionFrame path.
//
// Dependency rule (same as PngFrame.h): this class stays within
// Core/Draw/Painter/png only and MUST NOT depend on game/TexasHoldem. It reuses
// VsmLoadPngFrame for decode and duplicates the small, stable string literals
// of the contract ("frames/%08d.png", metadata keys) exactly as PngFrame.h
// already does — it is not a re-implementation of the format.
//
// Progress model — the recorder writes metadata.json up front with
// "status":"recording" and rewrites it "status":"complete" as its very last
// write. Old (pre-0137) sessions have no "status" field; its ABSENCE is treated
// as "complete" (never as "recording forever"), so old fixtures open and read
// as fully-available immediately.

class VsmLiveM01M02SessionSource : public VsmFrameSource {
public:
	// ReadFrameLive() result — the third state the base bool ReadFrame() cannot
	// express. LIVE_PENDING means "the next expected frame is not on disk yet,
	// recording is still in progress, poll/retry" — it is explicitly NOT the
	// same as end-of-stream.
	enum LiveResult {
		LIVE_OK,       // a frame was decoded into out_frame; cursor advanced
		LIVE_PENDING,  // next frame not written yet, recording in progress — retry
		LIVE_EOS       // recording complete AND all frames consumed — genuinely done
	};

	bool   Open(const String& uri) override;   // uri = session root directory
	void   Close() override;
	bool   IsReady()  const override { return is_ready_; }
	int    GetWidth() const override { return width_; }
	int    GetHeight()const override { return height_; }
	int    GetFPS()   const override { return 0; }

	// Base contract: true iff a frame was read (LIVE_OK). Both LIVE_PENDING and
	// LIVE_EOS collapse to false here — callers that must distinguish "wait" from
	// "done" should use ReadFrameLive() instead. For an already-complete session
	// this behaves exactly like a one-shot reader: true per frame, false at end.
	bool   ReadFrame(VsmImageBuffer& out_frame, int64& out_ts_ms) override;

	// Full three-state read used by live/tailing callers.
	LiveResult ReadFrameLive(VsmImageBuffer& out_frame, int64& out_ts_ms);

	String GetLastError()  const override { return last_error_; }
	String GetSourceInfo() const override;

	// --- Tailing / progress queries ---------------------------------------

	// Re-reads metadata.json's "status" from disk. True once the recorder has
	// finalized the session (or if the session predates the marker, i.e. no
	// "status" field). Cheap enough to poll.
	bool   IsRecordingComplete() const;

	// Target frame count declared in metadata.json (fixed up front by the recorder).
	int    GetTargetFrameCount() const { return frame_count_; }

	// Index of the next frame ReadFrameLive() will attempt (0-based).
	int    GetCursor() const { return cursor_; }

	// Number of frame PNGs currently present on disk (may lag GetTargetFrameCount
	// while recording is in progress).
	int    GetAvailableFrameCount() const;

	// Opportunistic per-frame ground truth: fills out_json with the raw JSONL
	// line for frame_id iff that line has actually been appended to
	// groundtruth.jsonl already. Never blocks/waits. Returns false if not yet
	// written (or malformed).
	bool   TryReadGroundTruth(int frame_id, String& out_json) const;

	String GetProvider()  const { return provider_; }
	String GetSessionId() const { return session_id_; }

private:
	String frame_path(int frame_id) const;
	bool   read_metadata();

	String root_;
	String provider_;
	String session_id_;
	bool   is_ready_    = false;
	int    width_       = 0;
	int    height_      = 0;
	int    frame_count_ = 0;   // target (final) count from metadata.json
	int    cursor_      = 0;   // next frame index to read
	String last_error_;
};

} // namespace Upp

#endif
