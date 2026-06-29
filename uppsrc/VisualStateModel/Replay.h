#ifndef _VisualStateModel_Replay_h_
#define _VisualStateModel_Replay_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Replay session — iterates a loaded VsmSession event by event

class VsmReplaySession : NoCopy {
public:
	VsmReplaySession() {}

	void SetLog(AppLog* sink) { log_.SetSink(sink); loader_.SetLog(sink); }
	CoreLog& GetLog()         { return log_; }

	// Load a ground truth JSON file. Returns false on parse error.
	bool Load(const String& json_path);

	// Load from an in-memory JSON string (for testing)
	bool LoadFromString(const String& json);

	// Returns true if there are still events to step through
	bool CanStep() const;

	// Process next event (returns false when done)
	bool Step();

	// Run all remaining events at once
	void RunAll();

	const VsmSession&         GetSession()     const { return session_; }
	const Vector<VsmDivergence>& GetDivergences() const { return session_.divergences; }
	int   GetEventPosition() const { return event_pos_; }
	int   GetTotalEvents()   const;

private:
	VsmSession           session_;
	VsmGroundTruthLoader loader_;
	CoreLog              log_;

	// Unified sorted event list for stepping
	struct StepEvent : Moveable<StepEvent> {
		String type;
		int    index = 0;
		int    frame = -1;
	};
	Vector<StepEvent> step_events_;
	int               event_pos_ = 0;

	void BuildStepEvents();
	void ProcessStepEvent(const StepEvent& se);
};

} // namespace Upp

#endif
