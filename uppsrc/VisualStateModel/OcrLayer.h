#ifndef _VisualStateModel_OcrLayer_h_
#define _VisualStateModel_OcrLayer_h_

namespace Upp {

// ---------------------------------------------------------------------------
// OCR data model

struct VsmOcrEngineInfo : Moveable<VsmOcrEngineInfo> {
	String name, version;
	bool   available = false;
	void Jsonize(JsonIO& json) {
		json("name",name)("version",version)("available",available);
	}
};

enum VsmTextExpectMode {
	VSM_EXPECT_EXACT    = 0,
	VSM_EXPECT_CONTAINS = 1,
};

enum VsmOcrStatus {
	VSM_OCR_PENDING  = 0,
	VSM_OCR_RUNNING  = 1,
	VSM_OCR_COMPLETE = 2,
	VSM_OCR_ERROR    = 3,
};

enum VsmOcrSeverity {
	VSM_OCR_OK      = 0,
	VSM_OCR_WARNING = 1,
	VSM_OCR_ERROR_S = 2,
};

struct VsmTextExpectation : Moveable<VsmTextExpectation> {
	int    mode = VSM_EXPECT_EXACT;
	String expected_text;
	void Jsonize(JsonIO& json) {
		json("mode",mode)("expected_text",expected_text);
	}
};

struct VsmOcrRule : Moveable<VsmOcrRule> {
	String             rule_id;
	String             annotation_id;
	String             pipeline_id;
	VsmTextExpectation expectation;
	double             confidence_threshold = 0.5;
	void Jsonize(JsonIO& json) {
		json("rule_id",rule_id)("annotation_id",annotation_id)
		    ("pipeline_id",pipeline_id)("expectation",expectation)
		    ("confidence_threshold",confidence_threshold);
	}
};

struct VsmOcrRequest : Moveable<VsmOcrRequest> {
	String rule_id, region_id;
	int    frame  = -1;
	String ts;
	int    status = VSM_OCR_PENDING; // VsmOcrStatus
	void Jsonize(JsonIO& json) {
		json("rule_id",rule_id)("region_id",region_id)
		    ("frame",frame)("ts",ts)("status",status);
	}
};

struct VsmOcrResult : Moveable<VsmOcrResult> {
	String rule_id, text;
	double confidence = 0.0;
	String ts;
	void Jsonize(JsonIO& json) {
		json("rule_id",rule_id)("text",text)
		    ("confidence",confidence)("ts",ts);
	}
};

struct VsmOcrComparison : Moveable<VsmOcrComparison> {
	String rule_id;
	int    severity = VSM_OCR_OK; // VsmOcrSeverity
	String message;
	void Jsonize(JsonIO& json) {
		json("rule_id",rule_id)("severity",severity)("message",message);
	}
};

// ---------------------------------------------------------------------------
// Engine adapter interface (pure virtual — implement for real engines)

class VsmOcrEngine {
public:
	virtual ~VsmOcrEngine() {}
	virtual VsmOcrEngineInfo  GetInfo() const = 0;
	virtual VsmOcrResult      Execute(const VsmFrameImage& img,
	                                  const VsmOcrRequest& req) = 0;
};

// Fake OCR engine for headless testing — always returns a configured string.
class VsmFakeOcrEngine : public VsmOcrEngine {
public:
	VsmFakeOcrEngine() {}
	explicit VsmFakeOcrEngine(const String& text, double confidence = 0.95)
	    : fake_text_(text), fake_confidence_(confidence) {}

	VsmOcrEngineInfo GetInfo() const override;
	VsmOcrResult     Execute(const VsmFrameImage& img,
	                         const VsmOcrRequest& req) override;

	void SetText(const String& text)         { fake_text_       = text; }
	void SetConfidence(double c)             { fake_confidence_ = c;    }

private:
	String fake_text_       = "FAKE_OCR";
	double fake_confidence_ = 0.95;
};

// ---------------------------------------------------------------------------
// OCR executor: runs a rule against a region image, compares to expectation.

class VsmOcrExecutor {
public:
	void SetLog(AppLog* sink)           { log_.SetSink(sink); }
	void SetEngine(VsmOcrEngine* eng)   { engine_ = eng;       }

	VsmOcrResult     RunRequest(const VsmFrameImage& img, const VsmOcrRequest& req);
	VsmOcrComparison Compare(const VsmOcrResult& result, const VsmOcrRule& rule);

private:
	CoreLog       log_;
	VsmOcrEngine* engine_ = nullptr;
};

} // namespace Upp

#endif
