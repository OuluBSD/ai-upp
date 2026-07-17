#ifndef _VideoGameEngineReplayValidator_VideoGameEngineReplayValidator_h_
#define _VideoGameEngineReplayValidator_VideoGameEngineReplayValidator_h_

#include <Core/Core.h>
#include <TexasHoldem/TexasHoldemLogicState.h>

NAMESPACE_UPP

// One per-field comparison result between the real engine's own state and the
// video-parsed ground-truth record it is being checked against, for a single
// replay step. Only fields whose ground-truth <field>_known flag is true are
// ever emitted (an unknown ground-truth field is never counted as a match or a
// mismatch -- it is simply not comparable).
struct ReplayFieldCompare : Moveable<ReplayFieldCompare> {
	int    step = -1;        // replay step index within the hand
	int    frame_id = -1;    // source ground-truth frame this step maps to (-1 == derived check, no gt frame)
	int    street = -1;
	int    seat_acted = -1;  // seat whose action produced this step
	String action;           // fold/check/call/raise/allin as applied to the engine
	String field;            // e.g. "pot", "player_3_stack", "player_5_bet", "board"
	String engine_value;
	String gt_value;
	bool   match = false;
	String note;             // rake / side-pot / winner-discrepancy annotation

	void Jsonize(JsonIO& json) {
		json("step", step)("frame_id", frame_id)("street", street)
		    ("seat_acted", seat_acted)("action", action)("field", field)
		    ("engine_value", engine_value)("gt_value", gt_value)
		    ("match", match)("note", note);
	}
};

struct ReplayHandReport : Moveable<ReplayHandReport> {
	int hand_index = -1;
	int gt_hand_id = -1;
	int dealer_seat = -1;
	int small_blind = -1;
	String derived_actions;              // human-readable derived action list
	String engine_result;                // showdown / winner summary from the engine
	Vector<ReplayFieldCompare> compares;
	int compared = 0;
	int matched = 0;
	int mismatched = 0;

	void Jsonize(JsonIO& json) {
		json("hand_index", hand_index)("gt_hand_id", gt_hand_id)
		    ("dealer_seat", dealer_seat)("small_blind", small_blind)
		    ("derived_actions", derived_actions)("engine_result", engine_result)
		    ("compared", compared)("matched", matched)("mismatched", mismatched)
		    ("compares", compares);
	}
};

struct ReplayReport : Moveable<ReplayReport> {
	String ground_truth_path;
	Vector<ReplayHandReport> hands;

	void Jsonize(JsonIO& json) {
		json("ground_truth_path", ground_truth_path)("hands", hands);
	}
};

END_UPP_NAMESPACE

#endif
