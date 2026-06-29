#include "VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// VsmPreprocessExecutor

VsmPreprocessResultRef VsmPreprocessExecutor::Execute(const VsmFrameImage& img,
                                                       const VsmPreprocessPipeline& pipeline,
                                                       VsmFrameImage& out_img)
{
	VsmPreprocessResultRef result;
	if(img.IsEmpty()) {
		result.warnings.Add("Input image is empty");
		return result;
	}

	// Start with a copy of src
	out_img.Set(img.width, img.height, img.data);

	for(const VsmPreprocessStep& step : pipeline.steps) {
		switch(step.type) {
		case VSM_PREP_GRAYSCALE: {
			VsmFrameImage gs;
			StepGrayscale(out_img, gs);
			out_img = pick(gs);
			result.steps_run++;
			break;
		}
		case VSM_PREP_INVERT:
			StepInvert(out_img);
			result.steps_run++;
			break;
		case VSM_PREP_THRESHOLD:
			StepThreshold(out_img, step.params.threshold_value);
			result.steps_run++;
			break;
		case VSM_PREP_NORMALIZE_32: {
			VsmFrameImage norm;
			StepNormalize(out_img, norm, step.params.target_w, step.params.target_h);
			out_img = pick(norm);
			result.steps_run++;
			break;
		}
		case VSM_PREP_OTSU:
			result.warnings.Add("Otsu threshold: deferred, requires image library");
			LogWarn(log_, "VsmPrep", "Otsu step skipped — not implemented headlessly");
			break;
		case VSM_PREP_EDGE_DETECT:
			result.warnings.Add("Edge detection: deferred, requires image library");
			LogWarn(log_, "VsmPrep", "EdgeDetect step skipped — not implemented headlessly");
			break;
		default:
			result.warnings.Add(Format("Unknown step type %d — skipped", step.type));
			break;
		}
	}

	result.success = true;
	LogInfo(log_, "VsmPrep", Format("Pipeline '%s': %d steps run, %d warnings",
	                                 pipeline.name, result.steps_run,
	                                 result.warnings.GetCount()));
	return result;
}

// Each output pixel: luma = (R*77 + G*150 + B*29) >> 8, stored in RGBA as gray
void VsmPreprocessExecutor::StepGrayscale(const VsmFrameImage& src, VsmFrameImage& dst)
{
	dst.Set(src.width, src.height, nullptr);
	const byte* s = src.data;
	byte*       d = dst.data;
	int n = src.width * src.height;
	for(int i = 0; i < n; i++) {
		byte luma = (byte)((int(s[0])*77 + int(s[1])*150 + int(s[2])*29) >> 8);
		d[0] = d[1] = d[2] = luma;
		d[3] = 255;
		s += 4; d += 4;
	}
}

void VsmPreprocessExecutor::StepInvert(VsmFrameImage& img)
{
	byte* p = img.data;
	int n   = img.width * img.height;
	for(int i = 0; i < n; i++) {
		p[0] = 255 - p[0];
		p[1] = 255 - p[1];
		p[2] = 255 - p[2];
		// alpha unchanged
		p += 4;
	}
}

void VsmPreprocessExecutor::StepThreshold(VsmFrameImage& img, int t)
{
	byte* p = img.data;
	int n   = img.width * img.height;
	for(int i = 0; i < n; i++) {
		byte luma = (byte)((int(p[0])*77 + int(p[1])*150 + int(p[2])*29) >> 8);
		byte out  = (luma >= t) ? 255 : 0;
		p[0] = p[1] = p[2] = out;
		p[3] = 255;
		p += 4;
	}
}

// Nearest-neighbor scaling
void VsmPreprocessExecutor::StepNormalize(const VsmFrameImage& src, VsmFrameImage& dst,
                                           int tw, int th)
{
	if(tw <= 0 || th <= 0) return;
	dst.Set(tw, th, nullptr);
	for(int dy = 0; dy < th; dy++) {
		int sy = dy * src.height / th;
		for(int dx = 0; dx < tw; dx++) {
			int sx = dx * src.width / tw;
			const byte* sp = src.data + (sy * src.width + sx) * 4;
			byte*       dp = dst.data + (dy * tw + dx) * 4;
			dp[0] = sp[0]; dp[1] = sp[1]; dp[2] = sp[2]; dp[3] = sp[3];
		}
	}
}

} // namespace Upp
