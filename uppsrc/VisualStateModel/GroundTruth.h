#ifndef _VisualStateModel_GroundTruth_h_
#define _VisualStateModel_GroundTruth_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Loaded ground truth session from a .vsm.json file

struct VsmSession {
	// Session metadata
	int    schema      = 0;
	String session_id;
	String source_type;
	int    frame_width  = 0;
	int    frame_height = 0;
	String started_at;
	String ended_at;
	String image_dir;
	String crop_dir;

	// Events in load order
	Vector<VsmFrameRef>           frames;
	Vector<VsmChangeEvent>        changes;
	Vector<VsmRegionNode>         regions;
	Vector<VsmOcrObservation>     ocr_results;
	Vector<VsmTemplateObservation> template_results;
	Vector<VsmModelStateRef>      state_snapshots;
	Vector<VsmDivergence>         divergences;

	// Warnings and errors from the file itself
	Vector<String> load_warnings;

	bool IsEmpty() const { return frames.IsEmpty() && changes.IsEmpty(); }
};

// ---------------------------------------------------------------------------
// Ground truth loader

class VsmGroundTruthLoader {
public:
	VsmGroundTruthLoader() {}

	void SetLog(AppLog* sink)  { log_.SetSink(sink); }
	CoreLog& GetLog()          { return log_; }

	// Load a .vsm.json file into a session. Returns false on parse failure.
	bool Load(const String& json_path, VsmSession& out);

private:
	CoreLog log_;

	void ProcessEvent(const Value& ev, VsmSession& out);
};

// ---------------------------------------------------------------------------
// Write a minimal sample ground truth JSON for testing

String VsmMakeSampleJson();

// ---------------------------------------------------------------------------
// Ground-truth session alias (a VsmSession loaded from a .vsm.json file)

using VsmGroundTruthSession = VsmSession;

// ---------------------------------------------------------------------------
// Canonical JSON comparison — structural equality independent of object key
// order and whitespace. Arrays remain order-sensitive. Returns false (does
// not throw) if either string fails to parse as JSON.

bool VsmCanonicalJsonEqual(const String& a, const String& b);

// ---------------------------------------------------------------------------
// Ground-truth vs observed comparison

struct VsmComparisonEntry : Moveable<VsmComparisonEntry> {
	String event_name;
	int    expected_frame = -1;
	int    observed_frame = -1; // -1 if missing or unexpected
	String status;              // "matched", "missing", "unexpected"
	String notes;
	void Jsonize(JsonIO& json) {
		json("event_name",event_name)("expected_frame",expected_frame)
		    ("observed_frame",observed_frame)("status",status)("notes",notes);
	}
};

struct VsmComparisonResult : Moveable<VsmComparisonResult> {
	int matched    = 0;
	int missing    = 0;
	int unexpected = 0;
	Vector<VsmComparisonEntry> entries;
	void Jsonize(JsonIO& json) {
		json("matched",matched)("missing",missing)("unexpected",unexpected)
		    ("entries",entries);
	}
};

class VsmGroundTruthComparison {
public:
	// Compare expected ground-truth divergences against those observed by the
	// pipeline.  Expected events are from gt.divergences; matching key is
	// expected_json (string equality) within ±5 frames.
	VsmComparisonResult Compare(const VsmGroundTruthSession& expected,
	                            const Vector<VsmDivergence>& observed);
};

} // namespace Upp

#endif
