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
// Dynamic OCR expected-text templates

namespace {

// Parses a non-negative decimal integer from `s`. Returns false (and leaves
// `out` untouched) if `s` is empty or contains anything but digits.
bool VsmParseTemplateIndex(const String& s, int& out)
{
	if(s.IsEmpty())
		return false;
	int v = 0;
	for(int i = 0; i < s.GetCount(); i++) {
		int c = s[i];
		if(c < '0' || c > '9')
			return false;
		v = v * 10 + (c - '0');
	}
	out = v;
	return true;
}

// Looks up `field` (optionally indexed with `[index]`) in `state`. Returns
// false when the field is absent or the index is out of range — callers
// treat that as "unresolved" and keep the original placeholder text.
bool VsmLookupTemplateField(const Value& state, const String& field,
                             bool has_index, int index, Value& out)
{
	const Value& base = state[field];
	if(base.IsError())
		return false;
	if(!has_index) {
		out = base;
		return true;
	}
	if(index < 0 || index >= base.GetCount())
		return false;
	out = base[index];
	return true;
}

} // anonymous namespace

String VsmResolveDynamicText(const String& template_text, const Value& state,
                              Vector<String>* missing_fields)
{
	String out;
	int n = template_text.GetCount();
	int i = 0;
	while(i < n) {
		int ch = template_text[i];
		if(ch != '{') {
			out.Cat(ch);
			i++;
			continue;
		}

		int close = template_text.Find('}', i + 1);
		if(close < 0) {
			// Unterminated placeholder — copy the rest verbatim and stop.
			out.Cat(template_text.Mid(i));
			break;
		}

		String token       = template_text.Mid(i + 1, close - i - 1);
		String placeholder = template_text.Mid(i, close - i + 1); // "{...}"
		String field        = token;
		bool   has_index     = false;
		int    index         = -1;

		int lb = token.Find('[');
		if(lb >= 0 && token.GetCount() > 0 && token[token.GetCount() - 1] == ']') {
			String idx_s = token.Mid(lb + 1, token.GetCount() - lb - 2);
			if(VsmParseTemplateIndex(idx_s, index)) {
				field     = token.Mid(0, lb);
				has_index = true;
			}
			// malformed index (e.g. "{field[x]}") — fall through and try
			// `field` = the whole token, which will simply fail to resolve.
		}

		Value resolved;
		bool ok = !field.IsEmpty() && VsmLookupTemplateField(state, field, has_index, index, resolved);
		if(ok) {
			out.Cat(resolved.ToString());
		} else {
			out.Cat(placeholder);
			const String& missing_name = field.IsEmpty() ? token : field;
			if(missing_fields)
				missing_fields->Add(missing_name);
			RLOG("VsmResolveDynamicText: unresolved field '" << missing_name
			     << "' in template '" << template_text << "' — leaving placeholder as-is");
		}
		i = close + 1;
	}
	return out;
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
                                          const VsmOcrRule& rule,
                                          const Value& live_state)
{
	VsmOcrComparison cmp;
	cmp.rule_id = rule.rule_id;

	const VsmTextExpectation& exp = rule.expectation;

	// Two independent axes (see VsmTextExpectation's comment in OcrLayer.h,
	// task 0073): whether the expected text is resolved from a live-state
	// template (`is_dynamic`), and how the resolved text is compared to the
	// OCR result (`compare_mode`, EXACT or CONTAINS). `mode ==
	// VSM_EXPECT_DYNAMIC` is a back-compat spelling of "dynamic + EXACT" —
	// every pre-0073 caller that set it gets identical behavior to before.
	bool is_dynamic    = exp.dynamic || exp.mode == VSM_EXPECT_DYNAMIC;
	int  compare_mode  = exp.mode == VSM_EXPECT_DYNAMIC ? VSM_EXPECT_EXACT : exp.mode;

	// Resolve the effective expected text up front. For dynamic mode this
	// requires a real live_state — its absence is a rule-configuration
	// error, independent of this run's OCR confidence, so check it first.
	String effective_expected = exp.expected_text;
	if(is_dynamic) {
		if(IsNull(live_state) || live_state.GetCount() == 0) {
			cmp.severity = VSM_OCR_ERROR_S;
			cmp.message  = "Rule '" + rule.rule_id + "' uses dynamic expected text but no "
			               "live_state was supplied (rule configuration error)";
			LogError(log_, "VsmOCR", cmp.message);
			return cmp;
		}
		Vector<String> missing_fields;
		effective_expected = VsmResolveDynamicText(exp.template_text, live_state, &missing_fields);
		if(!missing_fields.IsEmpty())
			LogWarn(log_, "VsmOCR", "Rule '" + rule.rule_id + "': dynamic template has "
			        "unresolved field(s): " + Join(missing_fields, ", "));
	}

	if(result.confidence < rule.confidence_threshold) {
		cmp.severity = VSM_OCR_WARNING;
		cmp.message  = Format("Confidence %.2f below threshold %.2f",
		                       result.confidence, rule.confidence_threshold);
		LogWarn(log_, "VsmOCR", "Rule '" + rule.rule_id + "': " + cmp.message);
		return cmp;
	}

	bool matches = false;
	if(compare_mode == VSM_EXPECT_CONTAINS)
		matches = (result.text.Find(effective_expected) >= 0);
	else
		matches = (result.text == effective_expected);

	if(matches) {
		cmp.severity = VSM_OCR_OK;
		cmp.message  = "Match OK";
		LogInfo(log_, "VsmOCR", "Rule '" + rule.rule_id + "': match OK");
	} else {
		cmp.severity = VSM_OCR_WARNING;
		cmp.message  = Format("Expected '%s' got '%s'",
		                       effective_expected, result.text);
		LogWarn(log_, "VsmOCR", "Rule '" + rule.rule_id + "': " + cmp.message);
	}
	return cmp;
}

} // namespace Upp
