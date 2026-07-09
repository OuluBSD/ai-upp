// VsmHeartsOcrPipeline (task 0073): wires VsmHeartsSource's live frames through
// VisualStateModel's real OCR/divergence machinery (VsmOcrExecutor +
// VsmModelRuntime), closing Manager/2-plan/ai-upp/root/VisualStateModel/docs/HEARTS_SOURCE_INVESTIGATION.md's
// gap #5 -- the last step of the Hearts-as-controlled-source investigation
// (tasks 0055-0073).
//
// See Manager/2-plan/ai-upp/root/VisualStateModel/docs/CARD_GAME_ADAPTER.md ("VsmHeartsOcrPipeline"
// section, task 0073) for the full design writeup: why three
// VSM_EXPECT_CONTAINS-mode dynamic OCR rules per player label zone (not one
// exact-match rule against the whole HUD label), the OcrLayer.h `dynamic`
// field fix this needed, and how OCR results are routed through
// VsmModelRuntime's SET_PROP_FROM_OCR + VALIDATE_PROP rule pairing to produce
// real VsmDivergence records.

class VsmHeartsOcrPipeline {
public:
	VsmHeartsOcrPipeline();

	void SetLog(AppLog* sink);

	// Runs the label-HUD OCR/divergence check for the CURRENT live state of
	// `source` (i.e. immediately after a successful Step()) against `frame`
	// (that same step's ReadFrame() output). Crops each of the 4 player
	// label zones ("label_self"/"label_left"/"label_top"/"label_right" --
	// main.py:394-407's update_hud(), zone index == player index) out of
	// `frame` via CardGameDocumentHost::GetZoneRect(), runs 3
	// VSM_EXPECT_CONTAINS + dynamic OCR rules per zone ("T:{scores[N]}",
	// "R:+{round_scores[N]}", "C:{hand_counts[N]}") through a
	// VsmFakeOcrEngine, and feeds every result through this object's
	// VsmModelRuntime as a SET_PROP_FROM_OCR + VALIDATE_PROP rule pair --
	// exactly mirroring reference/VisualStateModelTest/main.cpp's
	// TestPipelineRunner()/TestDeterministicReplay() pattern -- so a mismatch
	// produces a real VsmDivergence via GetModelRuntime().GetDivergences().
	//
	// `engine` supplies the OCR text: by default (both `corrupt_zone_index`
	// and `corrupt_field` unset) every rule is fed the actually-correct
	// resolved text (success path, per this task's decision 3 -- there is no
	// real OCR text-recognition engine in this repo, so the fake engine is
	// seeded with truth or deliberate lies to prove the *wiring*, not a
	// vision model). When `corrupt_zone_index >= 0`, that one zone's
	// `corrupt_field` ("score"/"round"/"hand") is instead fed a deliberately
	// wrong value at every call, to demonstrate the divergence path.
	//
	// `step_index` must be unique per call across the whole run (the driving
	// loop's Step() count is perfect for this) -- it is folded into every
	// rule_id/source_rule_id this call creates so that this step's OCR event
	// only ever matches this step's own SET_PROP_FROM_OCR/VALIDATE_PROP rule
	// pair, never a previous step's (ground truth changes every step; without
	// this, a stale rule from an earlier step would refire against a later
	// step's OCR event using outdated expected values). All rules accumulate
	// in the same long-lived VsmModelRuntime across the whole run, so
	// GetModelRuntime().GetDivergences() reflects every step checked so far.
	void RunStep(VsmHeartsSource& source, VsmFakeOcrEngine& engine,
	             const VsmImageBuffer& frame, int step_index,
	             int corrupt_zone_index = -1, const String& corrupt_field = String());

	const VsmModelRuntime& GetModelRuntime() const { return model_rt; }
	int GetRulesRun() const { return rules_run; }

private:
	VsmModelRuntime model_rt;
	int             rules_run = 0;
};
