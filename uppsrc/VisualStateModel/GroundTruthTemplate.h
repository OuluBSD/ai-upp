#ifndef _VisualStateModel_GroundTruthTemplate_h_
#define _VisualStateModel_GroundTruthTemplate_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Ground Truth Template Generator Options

struct VsmGroundTruthTemplateOptions : Moveable<VsmGroundTruthTemplateOptions> {
	String session_dir;   // existing recorded/imported session to read
	String output_path;   // where to write the template JSON
};

// ---------------------------------------------------------------------------
// Ground Truth Template Generator Result

struct VsmGroundTruthTemplateResult : Moveable<VsmGroundTruthTemplateResult> {
	bool   success = false;
	int    frame_count = 0;
	String session_id;
	String output_path;
};

// ---------------------------------------------------------------------------
// Ground Truth Template Generator

class VsmGroundTruthTemplateGenerator {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }

	// Generate a template ground truth JSON file from an existing session
	VsmGroundTruthTemplateResult Generate(const VsmGroundTruthTemplateOptions& opts);

private:
	CoreLog log_;
};

} // namespace Upp

#endif
