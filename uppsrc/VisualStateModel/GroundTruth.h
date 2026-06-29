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

} // namespace Upp

#endif
