#include "CardGameStateAdapter.h"

using namespace Upp;

// Headless demo/test for VsmCardGameStateExport (task 0068):
//   1. Loads uppsrc/ScriptIDE/reference/Hearts/game.gamestate into a
//      CardGameDocumentHost and runs it synchronously (ExecuteSync()).
//   2. Exports one card_play, one trick, and one round tier state_json.
//   3. Self-checks each against VsmCanonicalJsonEqual() and asserts the
//      emitted field set matches docs/VisualStateModel/CARD_GAME_STATE_SCHEMA.md
//      exactly (no missing/extra required fields).
//
// CardGameDocumentHost derives from Ctrl, so this needs GUI_APP_MAIN (like
// ScriptIDE's own --headless mode, uppsrc/ScriptIDE/Main.cpp) even though no
// window is ever shown -- CaptureRecordFrame()/ExecuteSync() both run through
// the offscreen SImageDraw path with no X11/display-server dependency
// (confirmed working in this environment by docs/VisualStateModel/HEARTS_SOURCE_INVESTIGATION.md's
// 2026-07-04 addendum).

static void AssertFieldsExact(const String& json, const char* tier, std::initializer_list<const char*> expected_fields)
{
	Value v = ParseJSON(json);
	ASSERT_(!v.IsError(), "AssertFieldsExact: failed to parse JSON: " + json);
	ASSERT_(v.Is<ValueMap>(), "AssertFieldsExact: top-level JSON is not an object: " + json);
	ValueMap vm = v;

	Index<String> expected;
	for(const char* f : expected_fields)
		expected.Add(f);

	Index<String> actual;
	for(int i = 0; i < vm.GetCount(); i++)
		actual.Add(vm.GetKey(i).ToString());

	for(int i = 0; i < expected.GetCount(); i++)
		ASSERT_(actual.Find(expected[i]) >= 0,
		        String("AssertFieldsExact[") + tier + "]: missing required field '" + expected[i] + "' in " + json);
	for(int i = 0; i < actual.GetCount(); i++)
		ASSERT_(expected.Find(actual[i]) >= 0,
		        String("AssertFieldsExact[") + tier + "]: unexpected extra field '" + actual[i] + "' in " + json);

	ASSERT_(vm["tier"].ToString() == tier,
	        String("AssertFieldsExact[") + tier + "]: 'tier' field mismatch in " + json);
}

static void AssertIntArrayLen(const String& json, const char* field, int len)
{
	Value v = ParseJSON(json);
	ValueMap vm = v;
	Value arr = vm[field];
	ASSERT_(arr.Is<ValueArray>(), String("AssertIntArrayLen: field '") + field + "' is not an array in " + json);
	ValueArray va = arr;
	ASSERT_(va.GetCount() == len,
	        String("AssertIntArrayLen: field '") + field + "' has " + AsString(va.GetCount()) + " elements, expected " + AsString(len));
}

GUI_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	// Binary lives in <repo_root>/bin/; game.gamestate lives in
	// <repo_root>/uppsrc/ScriptIDE/reference/Hearts/.
	String gamestate_path = GetFullPath(GetExeDirFile("../uppsrc/ScriptIDE/reference/Hearts/game.gamestate"));

	Cout() << "CardGameStateAdapter demo: loading " << gamestate_path << "\n";
	ASSERT_(FileExists(gamestate_path), "game.gamestate not found: " + gamestate_path);

	CardGameDocumentHost::log_to_stdout = true;
	CardGameDocumentHost host;
	host.SetFixedArea(Size(1024, 768));
	bool loaded = host.Load(gamestate_path);
	ASSERT_(loaded, "CardGameDocumentHost::Load failed for " + gamestate_path);

	host.ExecuteSync();
	Cout() << "ExecuteSync() returned.\n";

	VsmCardGameStateExport exporter;

	String card_play_json = exporter.ExportCardPlayState(host, 0, "2C");
	Cout() << "card_play: " << card_play_json << "\n";
	AssertFieldsExact(card_play_json, "card_play",
		{"tier", "round_number", "phase", "turn", "trick_number", "leading_suit", "hearts_broken", "player", "card_played", "hand_counts"});
	AssertIntArrayLen(card_play_json, "hand_counts", 4);
	ASSERT_(VsmCanonicalJsonEqual(card_play_json, card_play_json), "card_play_json does not round-trip against itself");

	String trick_json = exporter.ExportTrickState(host, 1, 2, 5);
	Cout() << "trick: " << trick_json << "\n";
	AssertFieldsExact(trick_json, "trick",
		{"tier", "round_number", "trick_number", "trick_winner", "trick_points", "round_scores"});
	AssertIntArrayLen(trick_json, "round_scores", 4);
	ASSERT_(VsmCanonicalJsonEqual(trick_json, trick_json), "trick_json does not round-trip against itself");

	String round_json = exporter.ExportRoundState(host, 1);
	Cout() << "round: " << round_json << "\n";
	AssertFieldsExact(round_json, "round",
		{"tier", "round_number", "round_scores", "scores", "moon_shooter", "game_over"});
	AssertIntArrayLen(round_json, "round_scores", 4);
	AssertIntArrayLen(round_json, "scores", 4);
	ASSERT_(VsmCanonicalJsonEqual(round_json, round_json), "round_json does not round-trip against itself");

	Cout() << "All CardGameStateAdapter checks passed.\n";

	// -----------------------------------------------------------------
	// VsmHeartsSource (task 0069): drive one full round via Step()/
	// HasMoreSteps()/ReadFrame(), collecting state_json along the way.
	// Driven directly rather than through VsmObservationPipeline::
	// RunFromSteppedSource(): that function's per-frame loop
	// (uppsrc/VisualStateModel/PipelineRunner.cpp) has no hook at all for
	// a stepped source's own state_json (it only consumes frames via
	// ProcessSourceFrame()), so wiring through it today would silently
	// drop every state_json event this task exists to produce -- a real
	// API gap, not "premature," confirmed by reading RunFromSteppedSource()
	// in full before choosing direct driving instead.
	Cout() << "\n--- VsmHeartsSource: driving one full round ---\n";
	{
		// CardGameDocumentHost::log_to_stdout is a *static* (process-wide) flag
		// the 0068 demo above turned on for its own single ExecuteSync() call.
		// Left on, it also applies to every ai_step()/refresh_ui() call this
		// driver makes -- and CardGameDocumentHost's own internal
		// CheckExpectedSpriteCounts() sprite-bookkeeping self-check (unrelated
		// to game logic; see docs/VisualStateModel/CARD_GAME_ADAPTER.md's
		// "VsmHeartsSource" section) re-logs its *entire* accumulated game_log
		// on every mismatch, which compounds across dozens of Step() calls into
        // an unusably large log. Turn it off before driving; it does not affect
		// GetLastStepEvents()/state_json correctness, only stdout volume.
		CardGameDocumentHost::log_to_stdout = false;

		VsmHeartsSource src;
		bool opened = src.Open(gamestate_path);
		ASSERT_(opened, "VsmHeartsSource::Open failed: " + src.GetLastError());
		Cout() << "VsmHeartsSource opened: " << src.GetSourceInfo()
		       << " (" << src.GetWidth() << "x" << src.GetHeight() << ")\n";

		int card_play_events = 0, trick_events = 0, round_events = 0;
		bool reached_round_end = false;
		String last_json;
		Vector<VsmCardGameEvent> all_events; // full emitted sequence, for the 0072 consistency check below

		while(src.HasMoreSteps() && src.GetStepCount() < VsmHeartsSource::kMaxSteps) {
			bool ok = src.Step();
			ASSERT_(ok, "VsmHeartsSource::Step failed: " + src.GetLastError());

			VsmImageBuffer frame;
			int64 ts_ms = 0;
			bool got_frame = src.ReadFrame(frame, ts_ms);
			ASSERT_(got_frame, "VsmHeartsSource::ReadFrame failed: " + src.GetLastError());
			ASSERT_(!frame.IsEmpty(), "VsmHeartsSource::ReadFrame produced an empty frame");

			for(const String& json : src.GetLastStepEvents()) {
				Cout() << "  [" << src.GetStepCount() << "] " << json << "\n";
				Value v = ParseJSON(json);
				ASSERT_(!v.IsError(), "VsmHeartsSource emitted unparsable JSON: " + json);
				ValueMap vm = v;
				String tier = vm["tier"].ToString();
				if(tier == "card_play") card_play_events++;
				else if(tier == "trick") trick_events++;
				else if(tier == "round") { round_events++; reached_round_end = true; }
				last_json = json;

				VsmCardGameEvent& e = all_events.Add();
				e.tier = tier;
				e.state_json = json;
			}
			if(reached_round_end)
				break;
		}

		ASSERT_(reached_round_end,
			Format("VsmHeartsSource: did not reach a 'round' tier event within %d Step() calls "
			       "(cap reached -- treating as a hang, not a pass): last_error='%s'",
			       VsmHeartsSource::kMaxSteps, src.GetLastError()));

		AssertFieldsExact(last_json, "round",
			{"tier", "round_number", "round_scores", "scores", "moon_shooter", "game_over"});
		AssertIntArrayLen(last_json, "round_scores", 4);
		AssertIntArrayLen(last_json, "scores", 4);

		Cout() << Format(
			"VsmHeartsSource: %d Step() calls, %d card_play events, %d trick events, "
			"%d round event. Final round_json: %s\n",
			src.GetStepCount(), card_play_events, trick_events, round_events, last_json);
		Cout() << "VsmHeartsSource one-round drive: OK.\n";

		// -------------------------------------------------------------
		// Task 0072: ground-truth self-consistency check on the real,
		// VsmHeartsSource-produced event sequence -- does the generator
		// itself internally reconcile (trick winners vs round-score
		// deltas, no duplicate cards, correct card count, correct round
		// total/shoot-the-moon handling) before this ground truth would be
		// fed into a pipeline test. See docs/VisualStateModel/HEARTS_SOURCE_INVESTIGATION.md
		// gap #6 and uppsrc/VisualStateModel/CardGameConsistency.h.
		Cout() << "\n--- VsmCheckCardGameConsistency: self-consistency check on the real round ---\n";
		VsmValidationResult consistency = VsmCheckCardGameConsistency(all_events);
		for(const VsmValidationIssue& issue : consistency.issues)
			Cout() << "  [" << issue.severity << "] " << issue.message << "\n";
		Cout() << Format("VsmCheckCardGameConsistency: %d event(s) checked, %d issue(s), ok=%s\n",
			consistency.frames_checked, consistency.issues.GetCount(), consistency.ok ? "true" : "false");
		ASSERT_(consistency.ok, "VsmCheckCardGameConsistency found consistency errors in the real VsmHeartsSource-driven round (see issues above)");
		Cout() << "VsmCheckCardGameConsistency: OK.\n";
	}

	// -----------------------------------------------------------------
	// Task 0073: wire VsmHeartsSource through the real OCR/divergence
	// pipeline (VsmOcrExecutor + VsmModelRuntime), closing gap #5 of
	// docs/VisualStateModel/HEARTS_SOURCE_INVESTIGATION.md -- the final step
	// of the Hearts controlled-source chain (tasks 0055-0073). See
	// docs/VisualStateModel/CARD_GAME_ADAPTER.md's "VsmHeartsOcrPipeline"
	// section and VsmHeartsOcrPipeline.h for the full design writeup.
	//
	// Two scenarios, each driving a fresh full round from scratch:
	//   1. Success path: every OCR rule, every Step(), is fed the actually-
	//      correct resolved text -> zero real VsmDivergence records.
	//   2. Divergence path: label_self's "C:" (hand-count) substring is fed
	//      a deliberately wrong value at every Step() -> at least one real
	//      VsmDivergence, with non-empty expected_json/observed_json.
	Cout() << "\n--- VsmHeartsOcrPipeline: OCR/divergence pipeline (success path) ---\n";
	{
		CardGameDocumentHost::log_to_stdout = false;

		VsmHeartsSource src;
		bool opened = src.Open(gamestate_path);
		ASSERT_(opened, "VsmHeartsOcrPipeline success-path: VsmHeartsSource::Open failed: " + src.GetLastError());

		VsmFakeOcrEngine      engine;
		VsmHeartsOcrPipeline  ocr_pipe;

		bool reached_round_end = false;
		while(src.HasMoreSteps() && src.GetStepCount() < VsmHeartsSource::kMaxSteps) {
			bool ok = src.Step();
			ASSERT_(ok, "VsmHeartsOcrPipeline success-path: Step failed: " + src.GetLastError());

			VsmImageBuffer frame;
			int64 ts_ms = 0;
			bool got_frame = src.ReadFrame(frame, ts_ms);
			ASSERT_(got_frame, "VsmHeartsOcrPipeline success-path: ReadFrame failed: " + src.GetLastError());

			// No corruption: every rule this step is fed the actually-correct
			// resolved text (decision 3: VsmFakeOcrEngine seeded with truth).
			ocr_pipe.RunStep(src, engine, frame, src.GetStepCount());

			for(const String& json : src.GetLastStepEvents())
				if(ParseJSON(json)["tier"].ToString() == "round")
					reached_round_end = true;
			if(reached_round_end)
				break;
		}
		ASSERT_(reached_round_end, "VsmHeartsOcrPipeline success-path: did not reach round end");

		int n_divergences = ocr_pipe.GetModelRuntime().GetDivergences().GetCount();
		Cout() << Format("VsmHeartsOcrPipeline success path: %d OCR rule(s) run over %d Step() calls, %d divergence(s)\n",
			ocr_pipe.GetRulesRun(), src.GetStepCount(), n_divergences);
		ASSERT_(n_divergences == 0, "VsmHeartsOcrPipeline success-path: expected zero divergences with correct OCR text");
		Cout() << "VsmHeartsOcrPipeline success path: OK (zero divergences).\n";
	}

	Cout() << "\n--- VsmHeartsOcrPipeline: OCR/divergence pipeline (divergence path) ---\n";
	{
		CardGameDocumentHost::log_to_stdout = false;

		VsmHeartsSource src;
		bool opened = src.Open(gamestate_path);
		ASSERT_(opened, "VsmHeartsOcrPipeline divergence-path: VsmHeartsSource::Open failed: " + src.GetLastError());

		VsmFakeOcrEngine      engine;
		VsmHeartsOcrPipeline  ocr_pipe;

		bool reached_round_end = false;
		while(src.HasMoreSteps() && src.GetStepCount() < VsmHeartsSource::kMaxSteps) {
			bool ok = src.Step();
			ASSERT_(ok, "VsmHeartsOcrPipeline divergence-path: Step failed: " + src.GetLastError());

			VsmImageBuffer frame;
			int64 ts_ms = 0;
			bool got_frame = src.ReadFrame(frame, ts_ms);
			ASSERT_(got_frame, "VsmHeartsOcrPipeline divergence-path: ReadFrame failed: " + src.GetLastError());

			// Deliberately wrong OCR text for label_self's "C:" (hand-count)
			// substring at every step -- every other rule/zone/step still
			// gets the correct text, so this isolates one real mismatch path.
			ocr_pipe.RunStep(src, engine, frame, src.GetStepCount(), /*corrupt_zone_index=*/0, "hand");

			for(const String& json : src.GetLastStepEvents())
				if(ParseJSON(json)["tier"].ToString() == "round")
					reached_round_end = true;
			if(reached_round_end)
				break;
		}
		ASSERT_(reached_round_end, "VsmHeartsOcrPipeline divergence-path: did not reach round end");

		const Vector<VsmDivergence>& divergences = ocr_pipe.GetModelRuntime().GetDivergences();
		Cout() << Format("VsmHeartsOcrPipeline divergence path: %d OCR rule(s) run over %d Step() calls, %d divergence(s)\n",
			ocr_pipe.GetRulesRun(), src.GetStepCount(), divergences.GetCount());
		ASSERT_(!divergences.IsEmpty(), "VsmHeartsOcrPipeline divergence-path: expected at least one real VsmDivergence");

		const VsmDivergence& d0 = divergences[0];
		Cout() << "First real VsmDivergence:\n";
		Cout() << "  region_id:     " << d0.region_id << "\n";
		Cout() << "  severity:      " << d0.severity << "\n";
		Cout() << "  message:       " << d0.message << "\n";
		Cout() << "  expected_json: " << d0.expected_json << "\n";
		Cout() << "  observed_json: " << d0.observed_json << "\n";
		ASSERT_(!d0.expected_json.IsEmpty() && !d0.observed_json.IsEmpty(),
			"VsmHeartsOcrPipeline divergence-path: expected_json/observed_json must not be empty");
		Cout() << "VsmHeartsOcrPipeline divergence path: OK (" << divergences.GetCount() << " real divergence(s)).\n";
	}

	SetExitCode(0);
}
