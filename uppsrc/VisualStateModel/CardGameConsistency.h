#ifndef _VisualStateModel_CardGameConsistency_h_
#define _VisualStateModel_CardGameConsistency_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Ground-truth self-consistency validator for card-game state_json sequences
// (task 0072, closes docs/VisualStateModel/HEARTS_SOURCE_INVESTIGATION.md
// gap #6).
//
// VsmGroundTruthComparison (GroundTruth.h) assumes the ground-truth side is
// trustworthy and only checks the observed/pipeline side against it. A
// controlled card-game source's whole point is that ground truth is
// *generated* mechanically -- nothing upstream of this checker validates
// that the generator itself is internally consistent (do emitted
// trick-winner events actually match emitted round-score deltas? no
// duplicate cards? do round totals check out?) before that ground truth is
// fed into a pipeline test. This is a headless, reusable equivalent of
// uppsrc/ScriptIDE/reference/Hearts/main.py's assert_state_invariants()/
// assert_render_invariants(), but operating on the emitted event log
// (docs/VisualStateModel/CARD_GAME_STATE_SCHEMA.md's card_play/trick/round
// tiers) rather than live Python GameState.

// One card-game tier event in emission order, as produced by a card-game
// ground-truth source (VsmCardGameStateExport / VsmHeartsSource in
// reference/CardGameStateAdapter/) -- see
// docs/VisualStateModel/CARD_GAME_STATE_SCHEMA.md for the three tier shapes.
struct VsmCardGameEvent : Moveable<VsmCardGameEvent> {
	String tier;       // "card_play" | "trick" | "round"
	String state_json; // serialized JSON object matching that tier's schema
};

// Checks a sequence of card-game tier events covering one round:
//
//  1. Card uniqueness -- every card_play event's card_played value is
//     unique across the whole sequence.
//  2. Trick-winner / round-score reconciliation -- for each trick event (in
//     sequence order), round_scores should show trick_points credited to
//     trick_winner relative to the previous trick event's round_scores (or
//     all-zero before the first trick), with every other player's entry
//     unchanged.
//  3. Round-total reconciliation -- the final round event's round_scores
//     should equal either the raw per-player sum of trick_points won across
//     all trick events (no shoot-the-moon), or the shoot-the-moon-adjusted
//     values (round.moon_shooter's entry 0, everyone else 26) when
//     round.moon_shooter >= 0 -- see hearts/logic.py's resolve_round() /
//     HEARTS_SOURCE_INVESTIGATION.md section 3 for the 26-point rule.
//  4. Card-count invariant -- total card_play events in the sequence equals
//     4 x (number of resolved trick events) -- every trick is exactly one
//     card per player, at any round size; for a fully resolved 13-trick
//     round this is the familiar 52 (13 tricks x 4 players).
//
// IMPORTANT (documented limitation, do not rediscover): a card_play event's
// own `trick_number` field (emitted by VsmCardGameStateExport) undercounts
// by one when the same player wins two consecutive tricks -- see
// docs/VisualStateModel/CARD_GAME_ADAPTER.md. This checker does NOT trust
// that field's monotonic progression for trick ordering; it validates
// trick-winner/round-score reconciliation (check 2) using each `trick` tier
// event's *position* in the sequence (the Nth resolved trick event is trick
// N, regardless of what its own trick_number field says), and it also
// tracks that same position while walking card_play events (the trick
// currently in progress is position+1). A mismatch between position-based
// and either tier's field-based trick numbering is reported as an
// informational note (severity "info"), tied to this known limitation --
// not a hard validation failure. (VsmHeartsSource's own `trick` tier field
// is tracked independently and does not exhibit the bug in practice, but
// this checker does not special-case that -- it reports whatever mismatch,
// if any, actually shows up in a given sequence.)
VsmValidationResult VsmCheckCardGameConsistency(const Vector<VsmCardGameEvent>& events);

} // namespace Upp

#endif
