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
		{"tier", "round_number", "phase", "turn", "trick_number", "leading_suit", "hearts_broken", "player", "card_played"});
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
	}

	SetExitCode(0);
}
