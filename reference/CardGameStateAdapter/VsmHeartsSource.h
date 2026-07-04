// VsmHeartsSource (task 0069): drives uppsrc/ScriptIDE/reference/Hearts/game.gamestate
// forward one logical step at a time via VsmSteppedFrameSource, on top of the
// CardGameDocumentHost + VsmCardGameStateExport plumbing from task 0068.
//
// See docs/VisualStateModel/CARD_GAME_ADAPTER.md ("VsmHeartsSource" section)
// for the full step-granularity writeup, the two set_timeout gotchas this
// class works around, and how it differs from the 0069 plan's assumptions.

class VsmHeartsSource : public VsmSteppedFrameSource {
public:
	// Safety cap: a full round is at most 4 (passing) + 52 (card plays) +
	// 13 (trick resolutions) = 69 Step() calls; 200 leaves ample headroom
	// while still guaranteeing Step() cannot be called forever on a bug.
	static const int kMaxSteps = 200;

	VsmHeartsSource();
	~VsmHeartsSource();

	// VsmFrameSource
	bool   Open(const String& uri) override;
	void   Close() override;
	bool   IsReady()  const override { return ready; }
	int    GetWidth() const override { return width; }
	int    GetHeight()const override { return height; }
	int    GetFPS()   const override { return 0; } // not a fixed-rate stream
	bool   ReadFrame(VsmImageBuffer& out_frame, int64& out_ts_ms) override;
	String GetLastError()  const override { return last_error; }
	String GetSourceInfo() const override;

	// VsmSteppedFrameSource
	bool Step() override;
	bool HasMoreSteps() const override;

	// state_json retrieval for the most recent Step() call. A single Step()
	// can legitimately produce more than one tier event (e.g. the 13th
	// trick's resolution and the round's resolution happen inside the same
	// Step(), since resolve_trick() calls resolve_round() synchronously in
	// Python when the round ends) -- GetLastStepEvents() carries all of
	// them, in emission order; GetLastStateJson() is a convenience returning
	// the last (most significant) one, or "" if this Step() produced none
	// (e.g. a passing-phase sub-step, which has no schema tier of its own).
	const Vector<String>& GetLastStepEvents() const { return last_step_events; }
	String                GetLastStateJson()  const {
		return last_step_events.IsEmpty() ? String() : last_step_events.Top();
	}
	int GetStepCount() const { return step_count; }

	// Exposed for task 0073's OCR/divergence wiring (VsmHeartsOcrPipeline),
	// which needs `host` directly for CardGameDocumentHost::GetZoneRect()
	// (player-label zone rects, for cropping) — nothing this class itself
	// needs from outside otherwise required this. `host` stays a plain
	// member (not re-derived) since VsmHeartsOcrPipeline is driven in lockstep
	// with this same open/Step() loop, never independently.
	CardGameDocumentHost& GetHost() { return host; }

private:
	CardGameDocumentHost   host;
	VsmCardGameStateExport exporter;

	bool   ready  = false;
	int    width  = 0;
	int    height = 0;
	String uri;
	String last_error;

	int step_count       = 0;
	int tricks_resolved   = 0; // exact count of finish_trick_collect() calls we made

	Vector<String> last_step_events;

	PyValue ModuleDict();
	bool    CallModuleFunc(PyValue& mod, const char* name);
	static bool GetModuleBool(PyValue& mod, const char* name);
	static String CardIdToCode(const String& card_id);
};
