#include "CardGameStateAdapter.h"

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// Small local helpers -- deliberately not reusing VsmCardGameStateExport.cpp's
// or VsmHeartsSource.cpp's own file-local GetAttr()/HGetAttr() (both stay
// file-local by design, per those files' own header comments); this mirrors
// the same small pattern independently, as those two already do with each
// other.

static PyValue OcrGetAttr(const PyValue& obj, const char* name)
{
	if(obj.GetType() != PY_DICT)
		return PyValue::None();
	PyValue v = obj.GetItem(PyValue(String(name)));
	if(!v.IsNone())
		return v;
	PyValue cls = obj.GetItem(PyValue("__class__"));
	if(cls.GetType() == PY_DICT)
		return cls.GetItem(PyValue(String(name)));
	return PyValue::None();
}

static Value OcrGetIntList(const PyValue& obj, const char* name)
{
	PyValue v = OcrGetAttr(obj, name);
	if(v.GetType() != PY_LIST)
		return ValueArray();
	return v.ToValue();
}

// Same derivation as VsmCardGameStateExport.cpp's GetHandCounts(): state.players
// is a list of 4 lists of Card objects; ToValue() doesn't give us "count per
// sublist" directly, so read lengths manually.
static Value OcrGetHandCounts(const PyValue& state)
{
	PyValue players = OcrGetAttr(state, "players");
	ValueArray counts;
	if(players.GetType() == PY_LIST) {
		for(int i = 0; i < players.GetCount(); i++) {
			PyValue hand = players.GetItem(i);
			counts.Add(hand.GetType() == PY_LIST ? hand.GetCount() : 0);
		}
	}
	return counts;
}

// Tier-independent HUD snapshot: {scores:[..], round_scores:[..], hand_counts:[..]}.
// Not one of CARD_GAME_STATE_SCHEMA.md's three schema tiers (those are per-event,
// caller-supplied-plus-live-fields) -- this is a plain live read of exactly the
// three fields main.py's update_hud() (main.py:394-407) renders into every
// player label at every frame, independent of which tier (if any) this
// particular Step() happened to emit. Used only as the `live_state` Value fed
// to VsmResolveDynamicText()/VsmOcrExecutor::Compare()'s dynamic-template
// resolution -- never serialized as a state_json event of its own.
static Value BuildHudLiveState(CardGameDocumentHost& host)
{
	PyVM* vm = host.GetVM();
	if(!vm)
		return Value();
	PyValue mod = VsmCardGameStateExport::FindEntryModuleDict(host, *vm);
	PyValue state = mod.GetItem(PyValue("state"));
	if(state.GetType() != PY_DICT)
		return Value();

	ValueMap m;
	m.Add("scores", OcrGetIntList(state, "scores"));
	m.Add("round_scores", OcrGetIntList(state, "round_scores"));
	m.Add("hand_counts", OcrGetHandCounts(state));
	return m;
}

// Crops [r] out of `frame` (RGB, 3-channel, per VsmHeartsSource::ReadFrame())
// into a VsmFrameImage (RGBA) -- same crop-and-convert shape as
// VsmObservationPipeline::GetRegionCrop()'s real-frame branch
// (uppsrc/VisualStateModel/PipelineRunner.cpp), reused as a pattern rather
// than a shared function since that one is a private method on a different
// class and reads from a VsmSessionStore, not a live VsmImageBuffer.
static VsmFrameImage CropZoneToFrameImage(const VsmImageBuffer& frame, const Rect& r)
{
	VsmFrameImage crop;
	int cx = max(0, r.left), cy = max(0, r.top);
	int cw = min(r.GetWidth(),  frame.width  - cx);
	int ch = min(r.GetHeight(), frame.height - cy);
	if(cw <= 0 || ch <= 0) {
		crop.Set(1, 1, nullptr);
		return crop;
	}
	crop.Set(cw, ch, nullptr);
	for(int y = 0; y < ch; y++) {
		for(int x = 0; x < cw; x++) {
			byte rr = frame.Get(cx + x, cy + y, 0);
			byte gg = frame.channels > 1 ? frame.Get(cx + x, cy + y, 1) : rr;
			byte bb = frame.channels > 2 ? frame.Get(cx + x, cy + y, 2) : rr;
			byte* dst = crop.data + (y * cw + x) * 4;
			dst[0] = rr; dst[1] = gg; dst[2] = bb; dst[3] = 255;
		}
	}
	return crop;
}

static Rect ZoneRectFromValue(const Value& zr)
{
	int zx = zr["x"], zy = zr["y"], zw = zr["w"], zh = zr["h"];
	return Rect(zx, zy, zx + zw, zy + zh);
}

// ---------------------------------------------------------------------------
// VsmHeartsOcrPipeline

namespace {
// zone index N == PLAYER_NAMES[N]/state.scores[N]/etc. index -- confirmed
// directly from main.py:394-407's update_hud(): `label_ids[i]` and
// `PLAYER_NAMES[i]` share the same index i, no rotation/remapping involved.
const char* kZoneIds[4] = { "label_self", "label_left", "label_top", "label_right" };

struct FieldSpec {
	const char* key;       // model-runtime property key
	const char* tmpl_fmt;  // %d -> zone/player index
};
const FieldSpec kFields[3] = {
	{ "score", "T:{scores[%d]}" },
	{ "round", "R:+{round_scores[%d]}" },
	{ "hand",  "C:{hand_counts[%d]}" },
};
} // anonymous namespace

VsmHeartsOcrPipeline::VsmHeartsOcrPipeline() {}

void VsmHeartsOcrPipeline::SetLog(AppLog* sink)
{
	model_rt.SetLog(sink);
}

void VsmHeartsOcrPipeline::RunStep(VsmHeartsSource& source, VsmFakeOcrEngine& engine,
                                    const VsmImageBuffer& frame, int step_index,
                                    int corrupt_zone_index, const String& corrupt_field)
{
	CardGameDocumentHost& host = source.GetHost();
	Value live_state = BuildHudLiveState(host);
	if(IsNull(live_state) || frame.IsEmpty())
		return; // not ready yet (e.g. called before Open()/first Step())

	VsmOcrExecutor exec;
	exec.SetEngine(&engine);

	for(int zi = 0; zi < 4; zi++) {
		String zone_id = kZoneIds[zi];
		Rect r = ZoneRectFromValue(host.GetZoneRect(zone_id));
		VsmFrameImage crop = CropZoneToFrameImage(frame, r);

		for(int fi = 0; fi < 3; fi++) {
			const FieldSpec& fs = kFields[fi];
			String template_text = Format(fs.tmpl_fmt, zi);
			String expected_text = VsmResolveDynamicText(template_text, live_state);

			bool inject_wrong = corrupt_zone_index == zi && corrupt_field == fs.key;
			String ocr_text = expected_text;
			if(inject_wrong) {
				// Deliberately wrong: reuse the same "<label>:" prefix (so it's
				// still recognizably an OCR read of the right substring, just
				// with a wrong value) but a number that cannot equal the live
				// ground truth this step -- live counters are all >= 0, so
				// "-1" is guaranteed to differ from any of them.
				int colon = expected_text.Find(':');
				String prefix = colon >= 0 ? expected_text.Left(colon + 1) : expected_text;
				ocr_text = prefix + "-1";
			}
			engine.SetText(ocr_text);

			String base_id = Format("hearts-ocr-%s-%s-step%d", zone_id, fs.key, step_index);

			VsmOcrRule rule;
			rule.rule_id              = base_id;
			rule.annotation_id        = zone_id;
			rule.expectation.mode     = VSM_EXPECT_CONTAINS;
			rule.expectation.dynamic  = true;
			rule.expectation.template_text = template_text;
			rule.confidence_threshold = 0.5;

			VsmOcrRequest req;
			req.rule_id   = rule.rule_id;
			req.region_id = zone_id;
			req.frame     = step_index;

			VsmOcrResult result = exec.RunRequest(crop, req);
			rules_run++;

			// Mirror TestPipelineRunner()/TestDeterministicReplay()
			// (reference/VisualStateModelTest/main.cpp): a SET_PROP_FROM_OCR
			// rule copies the OCR text into a model object property, paired
			// with a VSM_MR_VALIDATE_PROP rule whose expected_value is this
			// step's real resolved ground truth -- a mismatch there is what
			// produces a real VsmDivergence (VsmModelRuntime::GetDivergences()),
			// not just a VsmOcrComparison warning.
			VsmModelRule set_rule;
			set_rule.rule_id        = base_id + "-set";
			set_rule.type           = VSM_MR_SET_PROP_FROM_OCR;
			set_rule.object_id      = zone_id;
			set_rule.property_key   = fs.key;
			set_rule.source_rule_id = rule.rule_id;
			model_rt.AddRule(set_rule);

			VsmModelRule val_rule;
			val_rule.rule_id        = base_id + "-validate";
			val_rule.type           = VSM_MR_VALIDATE_PROP;
			val_rule.object_id      = zone_id;
			val_rule.property_key   = fs.key;
			// Quoted, matching SET_PROP_FROM_OCR's ev.data_json format below --
			// same convention TestDeterministicReplay/TestModelRuntime use
			// (mr.expected_value = "\"ExpectedText\"").
			val_rule.expected_value = "\"" + expected_text + "\"";
			val_rule.source_rule_id = rule.rule_id;
			model_rt.AddRule(val_rule);

			VsmModelEvent ev;
			ev.type             = "ocr_result";
			ev.ts               = result.ts;
			ev.source_region_id = zone_id;
			ev.source_rule_id   = rule.rule_id;
			ev.data_json        = "\"" + result.text + "\"";
			model_rt.ApplyEvent(ev);
		}
	}
}

END_UPP_NAMESPACE
