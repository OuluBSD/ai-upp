#ifndef _VisualStateModel_Preprocess_h_
#define _VisualStateModel_Preprocess_h_

namespace Upp {

enum VsmPreprocessStepType {
	VSM_PREP_GRAYSCALE    = 0, // Convert RGBA → grayscale (luminance)
	VSM_PREP_INVERT       = 1, // Invert grayscale values
	VSM_PREP_THRESHOLD    = 2, // Fixed threshold → black/white
	VSM_PREP_NORMALIZE_32 = 3, // Nearest-neighbor scale to 32×32
	VSM_PREP_OTSU         = 4, // Deferred: Otsu threshold (requires image lib)
	VSM_PREP_EDGE_DETECT  = 5, // Deferred: edge detection (requires image lib)
};

struct VsmPreprocessParams : Moveable<VsmPreprocessParams> {
	int threshold_value = 128; // for THRESHOLD
	int target_w        = 32;  // for NORMALIZE
	int target_h        = 32;
	void Jsonize(JsonIO& json) {
		json("threshold_value", threshold_value)
		    ("target_w",        target_w)
		    ("target_h",        target_h);
	}
};

struct VsmPreprocessStep : Moveable<VsmPreprocessStep> {
	int                type = VSM_PREP_GRAYSCALE;
	VsmPreprocessParams params;
	void Jsonize(JsonIO& json) { json("type",type)("params",params); }
};

struct VsmPreprocessPipeline : Moveable<VsmPreprocessPipeline> {
	String                    id, name;
	Vector<VsmPreprocessStep> steps;
	void Jsonize(JsonIO& json) { json("id",id)("name",name)("steps",steps); }
};

struct VsmPreprocessResultRef : Moveable<VsmPreprocessResultRef> {
	bool   success    = false;
	int    steps_run  = 0;
	String output_asset; // relative path to output asset (if saved)
	Vector<String> warnings;
	void Jsonize(JsonIO& json) {
		json("success",      success)
		    ("steps_run",    steps_run)
		    ("output_asset", output_asset)
		    ("warnings",     warnings);
	}
};

class VsmPreprocessExecutor {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }

	// Run pipeline on img, writing result into out_img.
	// Returns metadata about what was run.
	VsmPreprocessResultRef Execute(const VsmFrameImage& img,
	                               const VsmPreprocessPipeline& pipeline,
	                               VsmFrameImage& out_img);

private:
	CoreLog log_;

	static void StepGrayscale (const VsmFrameImage& src, VsmFrameImage& dst);
	static void StepInvert    (VsmFrameImage& img);
	static void StepThreshold (VsmFrameImage& img, int t);
	static void StepNormalize (const VsmFrameImage& src, VsmFrameImage& dst, int tw, int th);
};

} // namespace Upp

#endif
