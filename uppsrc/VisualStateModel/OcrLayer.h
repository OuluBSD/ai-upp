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
	// VSM_EXPECT_DYNAMIC is kept only for source/back-compat with code written
	// against task 0070: setting `mode = VSM_EXPECT_DYNAMIC` still means
	// "dynamic, compared EXACT", exactly as before (see VsmTextExpectation's
	// `dynamic` field below, added by task 0073, for the general case).
	VSM_EXPECT_DYNAMIC  = 2, // expected_text is resolved from template_text + a live state_json Value
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
	// `mode` is purely the *comparison* mode (EXACT vs CONTAINS) — whether the
	// resolved expected text must equal the OCR result or merely appear
	// within it. `dynamic` is an orthogonal, independent flag: whether that
	// resolved expected text comes from `expected_text` verbatim (dynamic ==
	// false) or from resolving `template_text` against a live state_json
	// Value at compare time (dynamic == true). These two axes were originally
	// conflated into one three-value enum (task 0070), which meant
	// VSM_EXPECT_DYNAMIC could only ever compare EXACT — there was no way to
	// ask for "dynamic template, but CONTAINS comparison" (needed by task
	// 0073's per-substring HUD label checks). `mode == VSM_EXPECT_DYNAMIC` is
	// kept working exactly as before (dynamic resolution + EXACT compare) for
	// back-compat with existing callers that never set `dynamic` explicitly;
	// new callers should set `mode` to EXACT/CONTAINS and `dynamic` to true
	// when they want templated CONTAINS matching.
	int    mode = VSM_EXPECT_EXACT;
	bool   dynamic = false;
	String expected_text;
	String template_text; // used instead of expected_text when dynamic resolution is in effect
	void Jsonize(JsonIO& json) {
		json("mode",mode)("dynamic",dynamic)("expected_text",expected_text)("template_text",template_text);
	}
};

// ---------------------------------------------------------------------------
// Dynamic OCR expected-text templates
//
// Minimal field-substitution against a parsed state_json Value — not a
// general expression language. Syntax:
//   {field}     -> scalar value of state[field], via Value::ToString()
//   {field[N]}  -> the Nth element of the array-valued state[field]
//
// Fields absent from `state` (unknown key, index out of range, or state
// itself null/empty) leave the original "{...}" placeholder text untouched
// in the output rather than silently collapsing to an empty string
// (loud-not-silent). Each unresolved placeholder is also reported via RLOG
// and, if `missing_fields` is supplied, appended to it — callers can check
// either to detect that this happened.
String VsmResolveDynamicText(const String& template_text, const Value& state,
                              Vector<String>* missing_fields = nullptr);

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
	// live_state is only consulted when rule.expectation.mode == VSM_EXPECT_DYNAMIC:
	// the effective expected text is resolved via VsmResolveDynamicText(template_text,
	// live_state) before comparing. A null/empty live_state with dynamic mode is treated
	// as a rule-configuration error (VSM_OCR_ERROR_S), not a silent pass.
	VsmOcrComparison Compare(const VsmOcrResult& result, const VsmOcrRule& rule,
	                          const Value& live_state = Value());

private:
	CoreLog       log_;
	VsmOcrEngine* engine_ = nullptr;
};

} // namespace Upp

#endif
