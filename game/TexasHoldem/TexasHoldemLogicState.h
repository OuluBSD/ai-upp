#ifndef _CardEngine_TexasHoldemLogicState_h_
#define _CardEngine_TexasHoldemLogicState_h_

#include <Core/Core.h>

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// M05-01 (task 0119): the vision pipeline's parsed logic-state schema.
//
// Mirrors TexasHoldemGroundTruthRecord / TexasHoldemPlayerSnapshot
// (TexasHoldemSessionContract.h:8-42) FIELD FOR FIELD, so a later task only
// has to fill values in, never restructure this record. The key difference:
// every field here is paired with an explicit `<field>_known` bool (default
// false), so "not yet parsed" can never be confused with "parsed as a real
// value of 0/-1/""/empty".
//
// Why a parallel bool per field rather than a single sentinel value (e.g.
// -1 or Null): every ground-truth field's legitimate value range already
// includes at least one of 0/-1/""/empty for a REAL parsed reading (seat 0 is
// a valid seat, street 0 is preflop, pot 0 is a real pot value, an empty
// hole_cards vector happens pre-deal, etc.) - there is no single sentinel
// that is simultaneously invalid for every field. A parallel `bool` is
// simple, uniform across every field type (int/int64/String/Vector), and
// trivially greppable/JSON-visible (`"stack_known":false` is unambiguous in
// the emitted JSONL, unlike inferring "unknown" from a magic number).
//
// Only ONE field is actually vision-derived by this task: `dealer_seat`
// (see reference/VisualStateLogicCompare). EVERY other field's `_known` flag
// is permanently false in this task's output - per the task's guardrail,
// nothing here may be guessed or silently defaulted. `frame_id` itself is
// not a "parsed" field at all (it's the record's own identity/index, always
// present), so it has no `_known` flag, matching how
// TexasHoldemGroundTruthRecord::frame_id has none either.

struct TexasHoldemLogicPlayerState : Moveable<TexasHoldemLogicPlayerState> {
	int seat = -1;   // identity (array position), not a "parsed" field - always meaningful once the record exists

	bool uid_known = false;
	int  uid = 0;

	bool name_known = false;
	String name;

	bool hero_known = false;
	bool hero = false;

	bool active_known = false;
	bool active = false;

	bool stack_known = false;
	int  stack = 0;

	bool bet_known = false;
	int  bet = 0;

	bool action_known = false;
	int  action = 0;

	// Mirrors TexasHoldemPlayerSnapshot::button (1 == GBUTTON_DEALER, see
	// game/GameRules/GameDefs.h:280 and game/TexasHoldem/GameTable.cpp:1570-
	// 1581). This is the ONE per-player field this task's CLI actually sets
	// button_known=true/button=1 for - and only for the single seat it has
	// derived as the current dealer (see VisualStateLogicCompare/main.cpp's
	// dealer-seat derivation). It deliberately does NOT set
	// button_known=true/button=0 for every other seat: the vision signal
	// only tells us "seat N's dealer_button sub-slot changed", never
	// "seat M definitely has no puck" (that would be an inference beyond
	// what was actually observed), so every non-derived seat's button field
	// stays honestly unknown.
	bool button_known = false;
	int  button = 0;

	bool hole_cards_known = false;
	Vector<int> hole_cards;

	void Jsonize(JsonIO& jio);
};

struct TexasHoldemLogicState : Moveable<TexasHoldemLogicState> {
	int schema = 1;

	bool session_id_known = false;
	String session_id;

	int frame_id = -1;   // identity, not a "parsed" field - always present

	bool render_step_known = false;
	int  render_step = 0;

	bool timestamp_ms_known = false;
	int64 timestamp_ms = 0;

	bool provider_known = false;
	String provider;

	bool table_size_known = false;
	int  table_width = 0;
	int  table_height = 0;

	bool seed_known = false;
	int  seed = -1;

	bool game_id_known = false;
	int  game_id = 0;

	bool hand_id_known = false;
	int  hand_id = 0;

	bool street_known = false;
	int  street = -1;

	bool turn_uid_known = false;
	int  turn_uid = -1;

	bool pot_known = false;
	int  pot = 0;

	bool board_cards_known = false;
	Vector<int> board_cards;

	// One entry per seat this record is tracking (seat count/identity comes
	// from the session being observed, NOT parsed from vision either - see
	// VisualStateLogicCompare/main.cpp for how `players` is populated).
	// `players_known` reflects whether ANY per-player field beyond bare
	// seat identity has been set for at least one player in this record.
	bool players_known = false;
	Vector<TexasHoldemLogicPlayerState> players;

	// --------------------------------------------------------------------
	// The one real vision-derived fact this task establishes end to end:
	// which seat currently has the dealer button, derived from the layout
	// model's "dealer_button"-role sub-slot (see LayoutProfile.cpp:165,
	// FormLayout.cpp's "button_puck" sub-slot row) changing between frames.
	bool dealer_seat_known = false;
	int  dealer_seat = -1;   // seat index, meaningful only if dealer_seat_known

	void Jsonize(JsonIO& jio);
};

END_UPP_NAMESPACE

#endif
