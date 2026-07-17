#include "VisualStateModel.h"
#include <plugin/jpg/jpg.h>

namespace Upp {

VsmTesseractOcrEngine::VsmTesseractOcrEngine()
{
	available_ = VsmAutoResolveTesseract(opt_, resolve_error_);
}

VsmTesseractOcrEngine::VsmTesseractOcrEngine(const VsmTesseractOptions& opt)
	: opt_(opt)
{
	available_ = VsmAutoResolveTesseract(opt_, resolve_error_);
}

VsmOcrEngineInfo VsmTesseractOcrEngine::GetInfo() const
{
	VsmOcrEngineInfo info;
	info.name      = "TesseractOCR";
	info.version   = "1.0";
	info.available = available_;
	return info;
}

VsmOcrResult VsmTesseractOcrEngine::Execute(const VsmFrameImage& img, const VsmOcrRequest& req)
{
	VsmOcrResult result;
	result.rule_id = req.rule_id;
	Time t = GetUtcTime();
	result.ts = Format("%04d-%02d-%02dT%02d:%02d:%02d.000Z",
	                    t.year, t.month, t.day, t.hour, t.minute, t.second);

	if(!available_ || img.IsEmpty())
		return result; // no engine / no image -- text stays empty, confidence 0

	// Convert the in-memory RGBA crop to a real file: tesseract only ever
	// operates on files on disk (see TesseractOcr.h), so this is the same
	// bridge VideoSemanticOcrProbe already relies on, just starting from a
	// VsmFrameImage instead of a file that was already on disk.
	Image image = VsmFrameImageToImage(img);
	String temp_path = GetTempFileName() + ".jpg";
	if(!JPGEncoder().Quality(95).SaveFile(temp_path, image))
		return result;

	VsmTesseractOcrResult r = VsmRunTesseractOcr(opt_, temp_path, req.semantic);
	DeleteFile(temp_path);

	result.text = r.text;
	// Tesseract's avg_conf (r.confidence) is a 0..100 tsv word-confidence
	// percentage; VsmOcrRule::confidence_threshold and VsmFakeOcrEngine's own
	// fake confidence are both 0..1 (VsmOcrExecutor::Compare() compares
	// result.confidence directly against confidence_threshold, default 0.5)
	// -- normalize here so a real caller sees the same scale a fake-engine
	// caller already does. r.confidence == -1 (no tsv data at all, e.g. tsv
	// config unresolved) maps to 0, not a negative confidence.
	result.confidence = r.confidence > 0 ? min(1.0, r.confidence / 100.0) : 0.0;
	return result;
}

} // namespace Upp
