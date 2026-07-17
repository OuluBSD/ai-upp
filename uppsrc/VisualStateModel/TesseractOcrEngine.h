#ifndef _VisualStateModel_TesseractOcrEngine_h_
#define _VisualStateModel_TesseractOcrEngine_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Task 0277: a real VsmOcrEngine (see OcrLayer.h) backed by the validated
// tesseract + Otsu + confidence-based-variant-selection pipeline
// (TesseractOcr.h, extracted from reference/VideoSemanticOcrProbe -- 100% on
// a real 31-crop dataset, Tasks 0271-0276). This is the first real engine
// implementation in uppsrc/ -- until now only VsmFakeOcrEngine (hardcoded
// "FAKE_OCR" text) existed. Wiring this in as VsmObservationPipeline's
// *default* engine is a separate, later decision (see this class's own
// comment below and Task 0277's task file) -- this class only proves a real
// engine exists and works correctly end-to-end.
class VsmTesseractOcrEngine : public VsmOcrEngine {
public:
	// Auto-resolves the tesseract executable + tessdata via
	// VsmAutoResolveTesseract(). GetInfo().available reflects whether that
	// resolution actually succeeded (mirrors the reference tool's own
	// real-exe-detection instead of a hardcoded `true`).
	VsmTesseractOcrEngine();
	explicit VsmTesseractOcrEngine(const VsmTesseractOptions& opt);

	VsmOcrEngineInfo GetInfo() const override;
	VsmOcrResult     Execute(const VsmFrameImage& img, const VsmOcrRequest& req) override;

private:
	VsmTesseractOptions opt_;
	bool                available_ = false;
	String              resolve_error_;
};

} // namespace Upp

#endif
