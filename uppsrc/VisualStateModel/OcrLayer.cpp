#include "VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// VsmFakeOcrEngine

VsmOcrEngineInfo VsmFakeOcrEngine::GetInfo() const
{
	VsmOcrEngineInfo info;
	info.name      = "FakeOCR";
	info.version   = "1.0";
	info.available = true;
	return info;
}

VsmOcrResult VsmFakeOcrEngine::Execute(const VsmFrameImage& /*img*/,
                                        const VsmOcrRequest& req)
{
	VsmOcrResult result;
	result.rule_id    = req.rule_id;
	result.text       = fake_text_;
	result.confidence = fake_confidence_;
	Time t = GetUtcTime();
	result.ts = Format("%04d-%02d-%02dT%02d:%02d:%02d.000Z",
	                    t.year, t.month, t.day, t.hour, t.minute, t.second);
	return result;
}

// ---------------------------------------------------------------------------
// VsmOcrExecutor

VsmOcrResult VsmOcrExecutor::RunRequest(const VsmFrameImage& img,
                                         const VsmOcrRequest& req)
{
	if(!engine_) {
		VsmOcrResult r;
		r.rule_id = req.rule_id;
		r.text    = "";
		r.confidence = 0.0;
		LogWarn(log_, "VsmOCR", "No OCR engine configured — returning empty result");
		return r;
	}
	VsmOcrResult result = engine_->Execute(img, req);
	LogInfo(log_, "VsmOCR", Format("OCR rule '%s': text='%s' confidence=%.2f",
	                                req.rule_id, result.text, result.confidence));
	return result;
}

VsmOcrComparison VsmOcrExecutor::Compare(const VsmOcrResult& result,
                                          const VsmOcrRule& rule)
{
	VsmOcrComparison cmp;
	cmp.rule_id = rule.rule_id;

	if(result.confidence < rule.confidence_threshold) {
		cmp.severity = VSM_OCR_WARNING;
		cmp.message  = Format("Confidence %.2f below threshold %.2f",
		                       result.confidence, rule.confidence_threshold);
		LogWarn(log_, "VsmOCR", "Rule '" + rule.rule_id + "': " + cmp.message);
		return cmp;
	}

	const VsmTextExpectation& exp = rule.expectation;
	bool matches = false;
	if(exp.mode == VSM_EXPECT_EXACT)
		matches = (result.text == exp.expected_text);
	else if(exp.mode == VSM_EXPECT_CONTAINS)
		matches = (result.text.Find(exp.expected_text) >= 0);

	if(matches) {
		cmp.severity = VSM_OCR_OK;
		cmp.message  = "Match OK";
		LogInfo(log_, "VsmOCR", "Rule '" + rule.rule_id + "': match OK");
	} else {
		cmp.severity = VSM_OCR_WARNING;
		cmp.message  = Format("Expected '%s' got '%s'",
		                       exp.expected_text, result.text);
		LogWarn(log_, "VsmOCR", "Rule '" + rule.rule_id + "': " + cmp.message);
	}
	return cmp;
}

} // namespace Upp
