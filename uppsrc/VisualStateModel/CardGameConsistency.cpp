#include "VisualStateModel.h"

namespace Upp {

namespace {

bool ParseObj(const String& json, ValueMap& out)
{
	Value v = ParseJSON(json);
	if(v.IsError() || !v.Is<ValueMap>())
		return false;
	out = v;
	return true;
}

Vector<int> ToIntArray(const Value& v)
{
	Vector<int> out;
	if(v.Is<ValueArray>()) {
		ValueArray va = v;
		for(int i = 0; i < va.GetCount(); i++)
			out.Add((int)va[i]);
	}
	return out;
}

} // anon namespace

VsmValidationResult VsmCheckCardGameConsistency(const Vector<VsmCardGameEvent>& events)
{
	VsmValidationResult result;
	result.frames_checked = events.GetCount(); // total events examined
	result.crops_checked  = 0;                 // not applicable to this checker

	auto AddIssue = [&](const char* severity, const String& message) {
		VsmValidationIssue& issue = result.issues.Add();
		issue.severity = severity;
		issue.message  = message;
	};

	// -----------------------------------------------------------------
	// Single pass over the sequence, tracking trick *position* (the number
	// of trick tier events resolved so far) as we go. Every check that
	// needs "which trick was this" uses this position counter, never the
	// event's own trick_number field -- see header comment /
	// CARD_GAME_ADAPTER.md's documented trick_number undercount limitation
	// (a card_play event's trick_number undercounts by one once the same
	// player wins two consecutive tricks; VsmHeartsSource's own `trick`
	// tier counter is independently tracked and does not share that bug,
	// but this checker treats neither field as trustworthy for ordering,
	// on principle, and reports any mismatch it finds as an informational
	// note rather than silently trusting whichever field happens to agree).
	Index<String> seen_cards;
	int card_play_count      = 0;
	int prev_scores[4]       = { 0, 0, 0, 0 };
	int trick_points_won[4]  = { 0, 0, 0, 0 }; // raw per-player sum, for check 3
	int trick_position       = 0;               // tricks resolved so far (0-based count)

	const VsmCardGameEvent* round_event = nullptr;
	int round_event_index = -1;

	for(int i = 0; i < events.GetCount(); i++) {
		const VsmCardGameEvent& ev = events[i];

		if(ev.tier == "card_play") {
			// --- Check 1: card uniqueness ---
			card_play_count++;

			ValueMap vm;
			if(!ParseObj(ev.state_json, vm)) {
				AddIssue("error", Format("event[%d] (card_play): unparsable state_json: %s", i, ev.state_json));
				continue;
			}
			String card = vm["card_played"].ToString();
			if(card.IsEmpty()) {
				AddIssue("error", Format("event[%d] (card_play): missing/empty card_played field", i));
			} else if(seen_cards.Find(card) >= 0) {
				AddIssue("error", Format("event[%d] (card_play): duplicate card_played '%s' (already played earlier this round)", i, card));
			} else {
				seen_cards.Add(card);
			}

			// Informational: the trick currently in progress is trick
			// (trick_position + 1) by position; compare against this
			// event's own trick_number field (the one documented to
			// undercount on same-winner-twice-in-a-row).
			int field_trick_number = (int)vm["trick_number"];
			int expected_trick_number = trick_position + 1;
			if(field_trick_number != expected_trick_number)
				AddIssue("info", Format(
					"event[%d] (card_play): position-based trick number (%d) differs from this event's own "
					"trick_number field (%d) -- known limitation, trick_number undercounts by one once the "
					"same player has won two consecutive tricks (see docs/VisualStateModel/CARD_GAME_ADAPTER.md); "
					"not flagged as an error",
					i, expected_trick_number, field_trick_number));
		}
		else if(ev.tier == "trick") {
			trick_position++;

			ValueMap vm;
			if(!ParseObj(ev.state_json, vm)) {
				AddIssue("error", Format("event[%d] (trick, position %d): unparsable state_json: %s", i, trick_position, ev.state_json));
				continue;
			}

			int field_trick_number = (int)vm["trick_number"];
			int trick_winner        = (int)vm["trick_winner"];
			int trick_points        = (int)vm["trick_points"];
			Vector<int> round_scores = ToIntArray(vm["round_scores"]);

			if(field_trick_number != trick_position)
				AddIssue("info", Format(
					"event[%d] (trick): position-based trick number (%d) differs from this event's own "
					"trick_number field (%d) -- known limitation, trick_number undercounts by one when "
					"the same player wins two consecutive tricks (see docs/VisualStateModel/CARD_GAME_ADAPTER.md); "
					"validated by position, not flagged as an error",
					i, trick_position, field_trick_number));

			if(round_scores.GetCount() != 4) {
				AddIssue("error", Format("event[%d] (trick, position %d): round_scores has %d entries, expected 4", i, trick_position, round_scores.GetCount()));
				continue;
			}
			if(trick_winner < 0 || trick_winner >= 4) {
				AddIssue("error", Format("event[%d] (trick, position %d): trick_winner %d out of range [0,3]", i, trick_position, trick_winner));
				continue;
			}

			bool row_ok = true;
			for(int p = 0; p < 4; p++) {
				int delta          = round_scores[p] - prev_scores[p];
				int expected_delta = (p == trick_winner) ? trick_points : 0;
				if(delta != expected_delta) {
					AddIssue("error", Format(
						"event[%d] (trick, position %d): round_scores[%d] changed by %d, expected %d "
						"(trick_winner=%d, trick_points=%d, prev round_scores[%d]=%d, now=%d)",
						i, trick_position, p, delta, expected_delta, trick_winner, trick_points, p, prev_scores[p], round_scores[p]));
					row_ok = false;
				}
			}
			// Always advance the running tally to the actually-observed
			// round_scores (not the expected one) so a single bad trick
			// doesn't cascade into spurious failures on every later trick.
			for(int p = 0; p < 4; p++)
				prev_scores[p] = round_scores[p];
			if(row_ok)
				trick_points_won[trick_winner] += trick_points;
		}
		else if(ev.tier == "round") {
			round_event       = &ev;
			round_event_index = i;
		}
	}

	// -----------------------------------------------------------------
	// Check 4: card-count invariant. Every trick is exactly 4 card_play
	// events (one per player) -- this holds at any round size, not just a
	// fully resolved 13-trick round (13 x 4 = 52 is simply that relation's
	// value at a complete round, mirroring assert_state_invariants's
	// total_cards % 4 == 0 / total_cards <= 52 check in main.py, but for
	// the emitted event log rather than live Python state).
	int expected_card_play_count = 4 * trick_position;
	if(card_play_count != expected_card_play_count)
		AddIssue("error", Format(
			"card_play event count = %d, expected %d (4 x %d resolved trick event(s))",
			card_play_count, expected_card_play_count, trick_position));

	// -----------------------------------------------------------------
	// Check 3: round-total reconciliation against the final round event.
	if(!round_event) {
		AddIssue("error", "no 'round' tier event found in the sequence");
	} else {
		ValueMap vm;
		if(!ParseObj(round_event->state_json, vm)) {
			AddIssue("error", Format("event[%d] (round): unparsable state_json: %s", round_event_index, round_event->state_json));
		} else {
			Vector<int> round_scores = ToIntArray(vm["round_scores"]);
			int moon_shooter = (int)vm["moon_shooter"];

			if(round_scores.GetCount() != 4) {
				AddIssue("error", Format("event[%d] (round): round_scores has %d entries, expected 4", round_event_index, round_scores.GetCount()));
			} else {
				int expected[4];
				bool moon = (moon_shooter >= 0 && moon_shooter < 4);
				for(int p = 0; p < 4; p++)
					expected[p] = moon ? (p == moon_shooter ? 0 : 26) : trick_points_won[p];

				for(int p = 0; p < 4; p++) {
					if(round_scores[p] != expected[p])
						AddIssue("error", Format(
							"event[%d] (round): round_scores[%d] = %d, expected %d (moon_shooter=%d, %s)",
							round_event_index, p, round_scores[p], expected[p], moon_shooter,
							moon ? "shoot-the-moon adjustment applied (0/26 rule)" : "raw sum of trick_points won"));
				}
			}
		}
	}

	// -----------------------------------------------------------------
	bool has_error = false;
	for(const VsmValidationIssue& issue : result.issues)
		if(issue.severity == "error") { has_error = true; break; }
	result.ok = !has_error;

	return result;
}

} // namespace Upp
