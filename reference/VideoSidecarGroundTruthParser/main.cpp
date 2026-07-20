#include "VideoSidecarGroundTruthParser.h"

using namespace Upp;

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// Task 0262: Real-Recording Sidecar Ground Truth Ingestion (First Pass)
//
// This file parses a hand-authored sidecar narration text file into a
// TexasHoldemLogicState-shaped ground-truth JSONL. See
// VideoSidecarGroundTruthParser.h for the file-level overview and the
// Manager task file (VisualStateModel/0262_*.md) for the full grammar spec.
//
// -------------------------- Documented design decisions --------------------
//
// 1. Chip amounts (balances, bet/raise/call/pot amounts) are given in the
//    sidecar as fractional BB with AT MOST one decimal digit ("108.5BB",
//    "2.5BB", "4BB"). TexasHoldemLogicPlayerState.stack/bet and
//    TexasHoldemLogicState.pot are `int`. Every such amount is scaled by
//    x10 (units of 0.1 BB) so it is represented as an exact integer with no
//    rounding -- e.g. "108.5BB" -> 1085, "2.5BB" -> 25, "4BB" -> 40.
//    ParseBBAmountX10() below both performs this scaling AND validates that
//    invariant, failing loudly if a future sidecar line ever violates it
//    (e.g. "2.55BB" or a trailing-dot "2.BB").
//    CORRECTION vs. the task file's stated invariant ("every value...has
//    exactly one decimal digit"): running this parser against the REAL
//    sidecar surfaced two amounts with NO decimal point at all -- "(pot
//    4BB)" (line 19) and "call 23BB" (line 34), both whole-BB values
//    written without a trailing ".0". This is a genuine, confirmed finding
//    from real data (not a hypothetical edge case), so the decimal point is
//    treated as OPTIONAL rather than mandatory -- see ParseBBAmountX10()'s
//    own comment for the exact accepted grammar.
//
// 2. frame_id = whole seconds elapsed since the FIRST event line's own
//    timestamp (per the task file), monotonically clamped forward
//    (frame_id = max(elapsed_seconds, previous_frame_id + 1)) so it is
//    always strictly increasing. This clamp is NOT a cosmetic nicety: the
//    real bin/video_record_25min_20260716_203356.txt has a genuine
//    non-monotonic anomaly at its very start -- the first line
//    ("R 00:00:05: windowtitle=...") is timestamped LATER than the six
//    "R 00:00:04: ..." lines that immediately follow it in file order. This
//    looks like a real authoring artifact in the hand-typed sidecar (not
//    something this parser should silently "fix" by reordering or
//    rewriting the source timestamps), so it is called out explicitly here
//    and in the task's final report rather than papered over. The clamp
//    guarantees the required "strictly increasing, meaningful index"
//    property regardless.
//    A separate, unclamped field -- timestamp_ms = raw_seconds * 1000 -- is
//    also emitted, carrying the literal (possibly non-monotonic) elapsed
//    video time as a distinct fact from the synthetic frame_id index.
//
// 3. Seats: the sidecar's "seatN" tokens (both in the position legend and
//    in every event line) are 1-indexed (seat1..seat6). TexasHoldemLogicPlayerState.seat
//    is 0-indexed (see game/TexasHoldem/Main.cpp:80, `ps.seat = i` with `i`
//    from 0), matching the live engine's own player-array convention. Every
//    "seatN" token is converted via ParseSeatToken() to seat index N-1.
//
// 4. Card notation: rank one of "23456789TJQKA", suit one of "cdhs",
//    concatenated <rank><suit> pairs (e.g. "3d3sKh" = 3d,3s,Kh). ParseCard()
//    is the exact inverse of GameEngineSyncer::FormatCard()
//    (reference/VideoGameEngineSyncer/main.cpp): FormatCard returns
//    ranks[card%13] + suits[card/13], so card = suit_index*13 + rank_index.
//
// 5. board_cards is always emitted as a fixed 5-element vector (matching the
//    -1-sentinel-for-"not yet dealt" convention TexasHoldemGroundTruthRecord
//    itself already uses, game/TexasHoldem/Main.cpp:66-68), reset to all -1
//    at every "new hand" event (explicitly instructed by the task file) and
//    filled in at flop (indices 0-2) / turn (index 3) / river (index 4).
//
// 6. Only fields the sidecar text ACTUALLY asserts at a given point are
//    marked *_known=true -- but "actually asserts" is read to include
//    direct, unambiguous consequences of the poker rules a real-money cash
//    game is bound by, not just literal substrings, e.g.:
//      - "call" (no amount) means "match the current street's outstanding
//        bet exactly" -- so its bet value is resolved from the last known
//        street-high bet, not invented.
//      - a new street (flop/turn/river dealt) resets every player's
//        bet-this-street to 0 -- a hold'em rule, not a guess (and required
//        for reference/VideoGameEngineSyncer's own street-transition check
//        to not fire a false divergence -- see its ProcessFrame's
//        "Street transitioned, but active Player N bet was not reset to 0"
//        check).
//      - a "new hand" line makes every seat active again (fresh hand).
//    Anything NOT directly stated or a direct rule-consequence of what IS
//    stated is left unknown -- most notably, this parser deliberately does
//    NOT compute/assert the small/big blind POST amounts themselves (the
//    sidecar only ever names WHO is SB/BB, never how much they posted), and
//    does NOT keep asserting a player's `stack` once they take any
//    stack-changing action (bet/call/raise/all-in, or posting a blind at
//    "new hand") -- the initial balance= reading becomes stale the instant
//    such an action happens and the sidecar never restates a new balance,
//    so `stack_known` decays back to false for that seat from that point on
//    rather than continuing to assert an increasingly-wrong number.
//
// 7. `button` (GuiButtonId: 1=dealer, 2=small blind, 3=big blind) is set for
//    all three roles directly named on the "new hand" line. This is a
//    WIDER use of the `button` field than reference/VisualStateLogicCompare's
//    existing precedent, which (per TexasHoldemLogicState.h's own doc
//    comment) only ever asserts button=1 for a vision-DERIVED dealer seat
//    and deliberately leaves every other seat's button field unknown,
//    because a vision signal alone can positively confirm "the puck moved
//    to seat N" but never positively confirm "seat M definitely has no
//    puck". Here the situation is different: the sidecar's "new hand" line
//    is a direct textual assertion of all three roles at once (not an
//    inferred vision delta), so all three are set. This is a deliberate,
//    documented deviation from the narrower precedent -- flagged in the
//    task's evidence for the user to double check, not silently done.
//
// 8. Two event kinds are recognized (not a parse failure) but map to no
//    TexasHoldemLogicState field at all, because no such field exists
//    anywhere in TexasHoldemLogicState/TexasHoldemLogicPlayerState (both
//    structs were read in full before writing this parser):
//      - "windowtitle=..." -- no raw window-title field.
//      - "seatN wins"      -- no "is winner" field (the closest thing in
//        the codebase, kActionIconVocabWinner, belongs to a completely
//        different subsystem -- the action-ICON template-match vocabulary
//        -- not to this ground-truth schema).
//    Both are genuine, documented schema gaps, not oversights; a record is
//    still emitted for their event line (to keep the timeline/frame_id
//    sequence complete) with no new field asserted.
//
// 9. hand_id is a simple 0-based counter incremented at each "new hand"
//    line -- an assigned index (like frame_id), not something read off the
//    narration text itself. session_id is the sidecar file's own title
//    (GetFileTitle), a data-provenance identifier, not narration content.
// ---------------------------------------------------------------------------

static int ParseSeatToken(const String& tok, int line_no, const String& raw_line)
{
	if(!tok.StartsWith("seat"))
		throw Exc(Format("line %d: expected a 'seatN' token, got '%s' (in: %s)", line_no, tok, raw_line));
	String num = tok.Mid(4);
	if(num.IsEmpty())
		throw Exc(Format("line %d: 'seat' token missing seat number (in: %s)", line_no, raw_line));
	for(int i = 0; i < num.GetCount(); i++)
		if(!IsDigit(num[i]))
			throw Exc(Format("line %d: seat token '%s' has a non-numeric seat number (in: %s)", line_no, tok, raw_line));
	int n = ScanInt(num);
	if(n < 1 || n > 6)
		throw Exc(Format("line %d: seat number %d out of the expected 1..6 range (in: %s)", line_no, n, raw_line));
	// Sidecar seatN tokens are 1-indexed; TexasHoldemLogicPlayerState.seat is
	// 0-indexed (game/TexasHoldem/Main.cpp:80, `ps.seat = i` with i from 0).
	return n - 1;
}

// x10 fixed-point scaling of a "<N[.D]>BB" chip amount -- see this file's
// header comment, decision (1).
//
// CORRECTION vs. the task file's stated invariant: the task file says
// "every value in this sidecar has exactly one decimal digit", but the REAL
// file (bin/video_record_25min_20260716_203356.txt) falsifies that for
// whole-BB amounts -- "R 00:00:13: seat4 raise 2.5BB (pot 4BB)" (line 19)
// and "R 00:00:47: seat6 call 23BB" (line 34) both have a bare integer BB
// amount with NO decimal point at all, not even ".0". Discovered by running
// the parser against the real file and hitting a genuine "missing decimal
// point" failure on line 19 -- not a hypothetical. Since x10 scaling is
// still exact and lossless for a bare integer (N -> N*10, frac digit 0),
// this is accepted as a second valid form of the same amount grammar rather
// than a sidecar syntax error: the decimal point is OPTIONAL, but if
// present it must carry exactly one digit (rejecting e.g. "2.55BB" or a
// trailing-dot "2.BB", which would be genuinely new/unrecognized syntax).
static int ParseBBAmountX10(const String& tok_with_bb, int line_no, const String& raw_line)
{
	if(!tok_with_bb.EndsWith("BB"))
		throw Exc(Format("line %d: expected a '<N[.D]>BB' chip amount, got '%s' (in: %s)", line_no, tok_with_bb, raw_line));
	String num = tok_with_bb.Left(tok_with_bb.GetCount() - 2);
	int dot = num.Find('.');
	String ip = dot < 0 ? num : num.Left(dot);
	int frac_digit = 0;
	if(dot >= 0) {
		String fp = num.Mid(dot + 1);
		if(fp.GetCount() != 1 || !IsDigit(fp[0]))
			throw Exc(Format("line %d: chip amount '%s' has a decimal point but not exactly one decimal digit after it (in: %s)", line_no, tok_with_bb, raw_line));
		frac_digit = fp[0] - '0';
	}
	if(ip.IsEmpty())
		throw Exc(Format("line %d: chip amount '%s' has no digits before the decimal point/BB suffix (in: %s)", line_no, tok_with_bb, raw_line));
	for(int i = 0; i < ip.GetCount(); i++)
		if(!IsDigit(ip[i]))
			throw Exc(Format("line %d: chip amount '%s' has a non-digit integer part (in: %s)", line_no, tok_with_bb, raw_line));
	return ScanInt(ip) * 10 + frac_digit;
}

// Exact inverse of GameEngineSyncer::FormatCard (reference/VideoGameEngineSyncer/main.cpp) --
// see this file's header comment, decision (4).
static int ParseCard(const String& tok, int line_no, const String& raw_line)
{
	static const char* ranks = "23456789TJQKA";
	static const char* suits = "cdhs";
	if(tok.GetCount() != 2)
		throw Exc(Format("line %d: invalid card token '%s' -- expected exactly 2 chars <rank><suit> (in: %s)", line_no, tok, raw_line));
	const char* rp = strchr(ranks, tok[0]);
	const char* sp = strchr(suits, tok[1]);
	if(!rp || !sp)
		throw Exc(Format("line %d: invalid card token '%s' -- rank must be one of %s, suit one of %s (in: %s)", line_no, tok, ranks, suits, raw_line));
	int rank_index = int(rp - ranks);
	int suit_index = int(sp - suits);
	return suit_index * 13 + rank_index;
}

static Vector<int> ParseCardRun(const String& s, int expect_count, int line_no, const String& raw_line)
{
	if(s.GetCount() != expect_count * 2)
		throw Exc(Format("line %d: expected %d concatenated card(s) (%d chars) but got '%s' (%d chars) (in: %s)",
		                  line_no, expect_count, expect_count * 2, s, s.GetCount(), raw_line));
	Vector<int> out;
	for(int i = 0; i < expect_count; i++)
		out.Add(ParseCard(s.Mid(i * 2, 2), line_no, raw_line));
	return out;
}

static int ParseTimestampSeconds(const String& ts, int line_no, const String& raw_line)
{
	if(ts.GetCount() != 8 || ts[2] != ':' || ts[5] != ':')
		throw Exc(Format("line %d: invalid timestamp '%s' -- expected HH:MM:SS (in: %s)", line_no, ts, raw_line));
	String hh = ts.Mid(0, 2), mm = ts.Mid(3, 2), ss = ts.Mid(6, 2);
	String parts3[3] = { hh, mm, ss };
	for(int p = 0; p < 3; p++)
		for(int i = 0; i < parts3[p].GetCount(); i++)
			if(!IsDigit(parts3[p][i]))
				throw Exc(Format("line %d: invalid timestamp '%s' -- non-digit component (in: %s)", line_no, ts, raw_line));
	int h = ScanInt(hh), m = ScanInt(mm), s = ScanInt(ss);
	if(m > 59 || s > 59)
		throw Exc(Format("line %d: invalid timestamp '%s' -- minutes/seconds out of range (in: %s)", line_no, ts, raw_line));
	return h * 3600 + m * 60 + s;
}

// ---------------------------------------------------------------------------
// Seat-position legend ("top=seat1", ... -- see this recording's own
// top-of-file block). This mapping is local to THIS ONE recording's
// camera/window orientation (not a general convention -- see the task file),
// and is not consumed by the emitted schema at all (no field on
// TexasHoldemLogicState represents on-screen clock position); it is parsed,
// validated, and printed for evidence/documentation purposes only.
// ---------------------------------------------------------------------------
struct SeatLegend {
	VectorMap<String, int> position_to_seat; // position name -> 0-indexed seat
};

static bool IsIgnorableSidecarLine(const String& line)
{
	String trimmed = TrimBoth(line);
	return trimmed.IsEmpty() || trimmed[0] == '#';
}

static SeatLegend ParseLegend(const Vector<String>& lines, int& i)
{
	SeatLegend legend;
	while(i < lines.GetCount()) {
		String line = TrimBoth(lines[i]);
		if(line.IsEmpty()) { i++; break; }
		if(line[0] == '#') { i++; continue; }
		int eq = line.Find('=');
		if(eq < 0)
			throw Exc(Format("line %d: expected legend entry '<position>=seatN', got '%s'", i + 1, line));
		String pos = line.Left(eq);
		String seat_tok = line.Mid(eq + 1);
		int seat = ParseSeatToken(seat_tok, i + 1, line);
		if(legend.position_to_seat.Find(pos) >= 0)
			throw Exc(Format("line %d: duplicate legend position '%s'", i + 1, pos));
		legend.position_to_seat.Add(pos, seat);
		i++;
	}
	if(legend.position_to_seat.GetCount() != 6)
		throw Exc(Format("seat-position legend must define exactly 6 positions, got %d", legend.position_to_seat.GetCount()));
	bool seen[6] = { false, false, false, false, false, false };
	for(int k = 0; k < legend.position_to_seat.GetCount(); k++) {
		int s = legend.position_to_seat[k];
		if(seen[s])
			throw Exc(Format("seat-position legend maps two different positions to seat%d", s + 1));
		seen[s] = true;
	}
	return legend;
}

// ---------------------------------------------------------------------------
// Running (cumulative) parse state -- one RunningPlayer per seat (fixed
// table size of 6, from the legend), plus table-level fields. Emit() below
// snapshots this into a full TexasHoldemLogicState per event line.
// ---------------------------------------------------------------------------
struct RunningPlayer : Moveable<RunningPlayer> {
	int seat = -1;
	bool name_known = false;       String name;
	bool stack_known = false;      int stack = 0;
	bool bet_known = false;        int bet = 0;
	bool active_known = false;     bool active = false;
	bool action_known = false;     int action = 0;
	bool button_known = false;     int button = 0;
	bool hole_cards_known = false; Vector<int> hole_cards;
};

struct RunningState {
	Vector<RunningPlayer> players; // fixed size 6, index == 0-based seat

	bool dealer_seat_known = false; int dealer_seat = -1;
	bool street_known = false;      int street = -1;
	bool pot_known = false;         int pot = 0;
	bool board_cards_known = false; Vector<int> board_cards; // always 5 entries, -1 = undealt
	bool hand_id_known = false;     int hand_id = -1;
	int  next_hand_id = 0;

	// Parser-local helper (NOT part of the emitted schema): the highest
	// total-this-street bet asserted so far this street, used to resolve a
	// bare "call" (no amount) -- see header comment, decision (6).
	bool street_high_bet_known = false;
	int  street_high_bet = 0;
};

// Hold'em rule, not a guess: every player's bet-this-street resets to 0 the
// instant a new street begins (see header comment, decision (6)).
static void ResetStreetBets(RunningState& rs)
{
	for(int i = 0; i < rs.players.GetCount(); i++) {
		rs.players[i].bet_known = true;
		rs.players[i].bet = 0;
	}
	rs.street_high_bet_known = true;
	rs.street_high_bet = 0;
}

static TexasHoldemLogicState Emit(const RunningState& rs, int frame_id, int64 timestamp_ms, const String& session_id)
{
	TexasHoldemLogicState st;
	st.schema = 1;
	st.session_id_known = true;
	st.session_id = session_id;
	st.frame_id = frame_id;
	st.timestamp_ms_known = true;
	st.timestamp_ms = timestamp_ms;
	st.dealer_seat_known = rs.dealer_seat_known;
	st.dealer_seat = rs.dealer_seat;
	st.street_known = rs.street_known;
	st.street = rs.street;
	st.pot_known = rs.pot_known;
	st.pot = rs.pot;
	st.board_cards_known = rs.board_cards_known;
	st.board_cards <<= rs.board_cards;
	st.hand_id_known = rs.hand_id_known;
	st.hand_id = rs.hand_id;

	bool any_player_known = false;
	for(int i = 0; i < rs.players.GetCount(); i++) {
		const RunningPlayer& rp = rs.players[i];
		TexasHoldemLogicPlayerState& p = st.players.Add();
		p.seat = rp.seat;
		p.name_known = rp.name_known;             p.name = rp.name;
		p.stack_known = rp.stack_known;            p.stack = rp.stack;
		p.bet_known = rp.bet_known;                p.bet = rp.bet;
		p.active_known = rp.active_known;          p.active = rp.active;
		p.action_known = rp.action_known;          p.action = rp.action;
		p.button_known = rp.button_known;          p.button = rp.button;
		p.hole_cards_known = rp.hole_cards_known;  p.hole_cards <<= rp.hole_cards;
		if(rp.name_known || rp.stack_known || rp.bet_known || rp.active_known ||
		   rp.action_known || rp.button_known || rp.hole_cards_known)
			any_player_known = true;
	}
	st.players_known = any_player_known;
	return st;
}

// ---------------------------------------------------------------------------
// Per-event-line grammar dispatch. `rest` is everything after
// "<window> HH:MM:SS: ".
// Any syntax not recognized here throws (fails loudly) rather than being
// silently skipped -- see the task file's explicit requirement.
// ---------------------------------------------------------------------------
static void ApplyEvent(RunningState& rs, const String& rest, int line_no, const String& raw_line)
{
	if(rest.StartsWith("windowtitle=")) {
		// Schema gap -- see header comment, decision (8). Recognized, asserts nothing.
		String q = rest.Mid((int)strlen("windowtitle="));
		if(q.GetCount() < 2 || q[0] != '"' || q[q.GetCount() - 1] != '"')
			throw Exc(Format("line %d: windowtitle= value must be a quoted string (in: %s)", line_no, raw_line));
		return;
	}

	if(rest.StartsWith("seat")) {
		int sp = rest.Find(' ');
		if(sp < 0)
			throw Exc(Format("line %d: unrecognized 'seat...' event (no action after the seat token) (in: %s)", line_no, raw_line));
		String seat_tok = rest.Left(sp);
		int seat = ParseSeatToken(seat_tok, line_no, raw_line);
		String action_rest = rest.Mid(sp + 1);
		RunningPlayer& rp = rs.players[seat];

		// A trailing " (pot Y.ZBB)" clause can follow EITHER a bare verb
		// ("call (pot 5.3BB)", line 21 of the real sidecar) OR a
		// verb+amount ("raise 2.5BB (pot 4BB)", line 19) -- strip it up
		// front, independent of which verb produced it, so the verb
		// dispatch below never has to special-case it per-verb. Applying
		// the pot value itself is likewise verb-independent (it is always
		// just "the table pot is now known to be Y.Z"), so it is applied
		// once here rather than duplicated in each verb branch.
		String verb_part = action_rest;
		int paren = verb_part.Find(" (pot ");
		if(paren >= 0) {
			if(!verb_part.EndsWith(")"))
				throw Exc(Format("line %d: malformed trailing '(pot ...)' clause (in: %s)", line_no, raw_line));
			String pot_tok = verb_part.Mid(paren + 6, verb_part.GetCount() - paren - 6 - 1);
			rs.pot_known = true;
			rs.pot = ParseBBAmountX10(pot_tok, line_no, raw_line);
			verb_part = verb_part.Left(paren);
		}

		if(verb_part.StartsWith("name=")) {
			int q1 = verb_part.Find('"');
			if(q1 != 5)
				throw Exc(Format("line %d: expected name=\"...\" (in: %s)", line_no, raw_line));
			int q2 = verb_part.Find('"', q1 + 1);
			if(q2 < 0)
				throw Exc(Format("line %d: unterminated name=\"...\" (in: %s)", line_no, raw_line));
			String name = verb_part.Mid(q1 + 1, q2 - q1 - 1);
			String tail = TrimBoth(verb_part.Mid(q2 + 1));
			if(!tail.StartsWith("balance="))
				throw Exc(Format("line %d: expected balance=X.YBB after name=\"...\" (in: %s)", line_no, raw_line));
			int stack = ParseBBAmountX10(tail.Mid((int)strlen("balance=")), line_no, raw_line);
			rp.name_known = true;  rp.name = name;
			rp.stack_known = true; rp.stack = stack;
			return;
		}
		if(verb_part == "fold") {
			rp.active_known = true; rp.active = false;
			rp.action_known = true; rp.action = 5; // PLAYER_ACTION_FOLD (game/GameRules/GameDefs.h)
			return;
		}
		if(verb_part == "check") {
			rp.action_known = true; rp.action = 1; // PLAYER_ACTION_CHECK
			return;
		}
		if(verb_part == "call" || verb_part.StartsWith("call ")) {
			if(verb_part == "call") {
				rp.action_known = true; rp.action = 2; // PLAYER_ACTION_CALL
				if(!rs.street_high_bet_known)
					throw Exc(Format("line %d: 'call' with no stated amount, but no current street bet is known to match it against (in: %s)", line_no, raw_line));
				rp.bet_known = true; rp.bet = rs.street_high_bet;
			}
			else {
				String amt = verb_part.Mid((int)strlen("call "));
				bool all_in = false;
				if(amt.StartsWith("all-in ")) { all_in = true; amt = amt.Mid((int)strlen("all-in ")); }
				int bet = ParseBBAmountX10(amt, line_no, raw_line);
				// "call all-in" is coded as ALLIN, mirroring the existing
				// "raise all-in" -> ALLIN judgment call (see decision above).
				rp.action_known = true; rp.action = all_in ? 6 : 2; // PLAYER_ACTION_ALLIN : PLAYER_ACTION_CALL
				rp.bet_known = true; rp.bet = bet;
				rs.street_high_bet_known = true; rs.street_high_bet = bet;
			}
			rp.stack_known = false; // stale -- see header comment, decision (6)
			return;
		}
		if(verb_part.StartsWith("bet ")) {
			int bet = ParseBBAmountX10(verb_part.Mid((int)strlen("bet ")), line_no, raw_line);
			rp.action_known = true; rp.action = 3; // PLAYER_ACTION_BET
			rp.bet_known = true; rp.bet = bet;
			rs.street_high_bet_known = true; rs.street_high_bet = bet;
			rp.stack_known = false;
			return;
		}
		if(verb_part.StartsWith("raise ")) {
			String amt = verb_part.Mid((int)strlen("raise "));
			bool all_in = false;
			if(amt.StartsWith("all-in ")) { all_in = true; amt = amt.Mid((int)strlen("all-in ")); }
			int bet = ParseBBAmountX10(amt, line_no, raw_line);
			rp.action_known = true; rp.action = all_in ? 6 : 4; // PLAYER_ACTION_ALLIN : PLAYER_ACTION_RAISE -- see header comment, decision (this is a judgment call: "raise all-in" is coded as ALLIN, not RAISE)
			rp.bet_known = true; rp.bet = bet;
			rs.street_high_bet_known = true; rs.street_high_bet = bet;
			rp.stack_known = false;
			return;
		}
		if(verb_part.StartsWith("shows ")) {
			rp.hole_cards_known = true;
			rp.hole_cards = ParseCardRun(verb_part.Mid((int)strlen("shows ")), 2, line_no, raw_line);
			return;
		}
		if(verb_part == "wins") {
			// Schema gap -- see header comment, decision (8). Recognized, asserts nothing.
			return;
		}
		throw Exc(Format("line %d: unrecognized seat action '%s' (in: %s)", line_no, verb_part, raw_line));
	}

	if(rest.StartsWith("new hand")) {
		Vector<String> parts = Split(rest, ',', false);
		if(parts.GetCount() != 4 || TrimBoth(parts[0]) != "new hand")
			throw Exc(Format("line %d: unrecognized 'new hand' event -- expected 'new hand, dealer=seatN, smallblind=seatN, bigblind=seatN' (in: %s)", line_no, raw_line));
		String dealer_part = TrimBoth(parts[1]);
		String sb_part = TrimBoth(parts[2]);
		String bb_part = TrimBoth(parts[3]);
		if(!dealer_part.StartsWith("dealer=") || !sb_part.StartsWith("smallblind=") || !bb_part.StartsWith("bigblind="))
			throw Exc(Format("line %d: unrecognized 'new hand' event fields (in: %s)", line_no, raw_line));
		int dealer = ParseSeatToken(dealer_part.Mid((int)strlen("dealer=")), line_no, raw_line);
		int sb = ParseSeatToken(sb_part.Mid((int)strlen("smallblind=")), line_no, raw_line);
		int bb = ParseSeatToken(bb_part.Mid((int)strlen("bigblind=")), line_no, raw_line);
		if(dealer == sb || dealer == bb || sb == bb)
			throw Exc(Format("line %d: 'new hand' names the same seat in two different roles (dealer=seat%d smallblind=seat%d bigblind=seat%d) (in: %s)",
			                  line_no, dealer + 1, sb + 1, bb + 1, raw_line));

		rs.hand_id_known = true; rs.hand_id = rs.next_hand_id++;
		rs.street_known = true; rs.street = 0; // GAME_STATE_PREFLOP
		rs.pot_known = false; rs.pot = 0; // blinds post atomically with this event but their size is never restated -- see header comment, decision (6)
		rs.board_cards_known = true;
		rs.board_cards.Clear();
		rs.board_cards.SetCount(5, -1);
		rs.street_high_bet_known = false; rs.street_high_bet = 0;
		rs.dealer_seat_known = true; rs.dealer_seat = dealer;

		for(int i = 0; i < rs.players.GetCount(); i++) {
			RunningPlayer& p = rs.players[i];
			p.active_known = true; p.active = true;
			p.action_known = false; p.action = 0;
			p.hole_cards_known = false; p.hole_cards.Clear();
			p.button_known = false; p.button = 0; // exactly one button per role; reassigned below
			if(i == sb || i == bb) {
				p.bet_known = false; p.bet = 0;   // blind amount never restated -- see header comment, decision (6); value zeroed too so no stale prior-hand number lingers behind the false flag
				p.stack_known = false;            // blind post changes stack by an amount not restated
			}
			else {
				p.bet_known = true; p.bet = 0; // certain: no action taken yet this hand
			}
		}
		rs.players[dealer].button_known = true; rs.players[dealer].button = 1; // GBUTTON_DEALER
		rs.players[sb].button_known = true;     rs.players[sb].button = 2;     // GBUTTON_SMALL_BLIND
		rs.players[bb].button_known = true;     rs.players[bb].button = 3;     // GBUTTON_BIG_BLIND
		return;
	}

	if(rest.StartsWith("flop ")) {
		// Optional trailing "(Y.ZBB)" or "(pot Y.ZBB)" suffix -- some narrated
		// hands state the post-flop pot inline (two different phrasings seen:
		// turn/river use "(pot Y.ZBB)", flop uses bare "(Y.ZBB)"), others (the
		// first hand in this sidecar) don't; only set pot_known when actually
		// stated, never guessed.
		String tail = rest.Mid((int)strlen("flop "));
		String cards_part = tail;
		int paren = tail.Find(" (");
		bool have_pot = false;
		int pot = 0;
		if(paren >= 0) {
			if(!tail.EndsWith(")"))
				throw Exc(Format("line %d: expected 'flop <cards> (Y.ZBB)' or 'flop <cards> (pot Y.ZBB)' (in: %s)", line_no, raw_line));
			cards_part = tail.Left(paren);
			String inner = tail.Mid(paren + 2, tail.GetCount() - paren - 2 - 1); // between '(' and ')'
			if(inner.StartsWith("pot "))
				inner = inner.Mid(4);
			pot = ParseBBAmountX10(inner, line_no, raw_line);
			have_pot = true;
		}
		Vector<int> c = ParseCardRun(cards_part, 3, line_no, raw_line);
		rs.street_known = true; rs.street = 1; // GAME_STATE_FLOP
		rs.board_cards_known = true;
		rs.board_cards.Clear();
		rs.board_cards.SetCount(5, -1);
		rs.board_cards[0] = c[0]; rs.board_cards[1] = c[1]; rs.board_cards[2] = c[2];
		if(have_pot) { rs.pot_known = true; rs.pot = pot; }
		ResetStreetBets(rs);
		return;
	}
	if(rest.StartsWith("turn ")) {
		String tail = rest.Mid((int)strlen("turn "));
		int paren = tail.Find(" (pot ");
		if(paren < 0 || !tail.EndsWith(")"))
			throw Exc(Format("line %d: expected 'turn <card> (pot Y.ZBB)' (in: %s)", line_no, raw_line));
		int card = ParseCard(tail.Left(paren), line_no, raw_line);
		int pot = ParseBBAmountX10(tail.Mid(paren + 6, tail.GetCount() - paren - 6 - 1), line_no, raw_line);
		if(!rs.board_cards_known || rs.board_cards.GetCount() != 5)
			throw Exc(Format("line %d: 'turn' event before any 'flop' event was seen (in: %s)", line_no, raw_line));
		rs.street_known = true; rs.street = 2; // GAME_STATE_TURN
		rs.board_cards[3] = card;
		rs.pot_known = true; rs.pot = pot;
		ResetStreetBets(rs);
		return;
	}
	if(rest.StartsWith("river ")) {
		String tail = rest.Mid((int)strlen("river "));
		int paren = tail.Find(" (pot ");
		if(paren < 0 || !tail.EndsWith(")"))
			throw Exc(Format("line %d: expected 'river <card> (pot Y.ZBB)' (in: %s)", line_no, raw_line));
		int card = ParseCard(tail.Left(paren), line_no, raw_line);
		int pot = ParseBBAmountX10(tail.Mid(paren + 6, tail.GetCount() - paren - 6 - 1), line_no, raw_line);
		if(!rs.board_cards_known || rs.board_cards.GetCount() != 5)
			throw Exc(Format("line %d: 'river' event before any 'flop'/'turn' event was seen (in: %s)", line_no, raw_line));
		rs.street_known = true; rs.street = 3; // GAME_STATE_RIVER
		rs.board_cards[4] = card;
		rs.pot_known = true; rs.pot = pot;
		ResetStreetBets(rs);
		return;
	}

	throw Exc(Format("line %d: unrecognized event syntax (in: %s)", line_no, raw_line));
}

static int SidecarWindowIndex(char window, int line_no, const String& raw_line)
{
	if(window == 'R') return 0;
	if(window == 'L') return 1;
	throw Exc(Format("line %d: unsupported window '%c'; expected R or L (in: %s)",
	                 line_no, window, raw_line));
}

static void SplitEventLine(const String& raw_line, int line_no, char& window,
	                       String& ts_str, String& rest)
{
	if(raw_line.GetCount() < 2 || raw_line[1] != ' ')
		throw Exc(Format("line %d: expected event line to start with 'R ' or 'L ' (in: %s)",
		                 line_no, raw_line));
	window = raw_line[0];
	SidecarWindowIndex(window, line_no, raw_line);
	String after_window = raw_line.Mid(2);
	if(after_window.GetCount() < 10 || after_window[8] != ':' || after_window[9] != ' ')
		throw Exc(Format("line %d: expected '<R|L> HH:MM:SS: <event>' (in: %s)",
		                 line_no, raw_line));
	ts_str = after_window.Left(8);
	rest = after_window.Mid(10);
	if(rest.IsEmpty())
		throw Exc(Format("line %d: empty event after timestamp (in: %s)", line_no, raw_line));
}

static void InitializeRunningState(RunningState& rs)
{
	rs.players.SetCount(6);
	for(int seat = 0; seat < 6; seat++) rs.players[seat].seat = seat;
	rs.board_cards.SetCount(5, -1);
	rs.board_cards_known = false;
}

struct HandAudit : Moveable<HandAudit> {
	char window = 0;
	int hand = 0;
	int start_line = 0;
	int start_second = 0;
	bool open = false;
	bool named[6] = {};
	bool balance[6] = {};
	bool folded[6] = {};
	int actions[4] = {};
	int street = 0;
	int street_bet = 0;
	int last_action_seat = -1;
	int last_action_second = -1;
	int shows = 0;
	int winners = 0;
	Vector<String> warnings;
	Vector<String> errors;
};

static void AddAuditWarning(HandAudit& h, const String& text)
{
	for(const String& old : h.warnings)
		if(old == text) return;
	h.warnings.Add(text);
}

static void AddAuditError(HandAudit& h, const String& text)
{
	for(const String& old : h.errors)
		if(old == text) return;
	h.errors.Add(text);
}

static void FinishHandAudit(HandAudit& h, Vector<HandAudit>& out)
{
	if(!h.open) return;
	int missing = 0;
	for(int seat = 0; seat < 6; seat++)
		missing += !h.named[seat] || !h.balance[seat];
	if(missing)
		AddAuditWarning(h, Format("missing seat metadata for %d seat(s)", missing));
	if(h.actions[0] < 2)
		AddAuditWarning(h, Format("preflop action trace incomplete: only %d action(s)", h.actions[0]));
	if(h.street >= 1 && h.actions[1] < 2)
		AddAuditWarning(h, Format("flop action trace incomplete: only %d action(s)", h.actions[1]));
	if(h.street >= 2 && h.actions[2] < 2)
		AddAuditWarning(h, Format("turn action trace incomplete: only %d action(s)", h.actions[2]));
	if(h.street >= 3 && h.actions[3] < 2)
		AddAuditWarning(h, Format("river action trace incomplete: only %d action(s)", h.actions[3]));
	if(h.shows && !h.winners)
		AddAuditWarning(h, "showdown detected without an explicit winner");
	if(!h.winners)
		AddAuditWarning(h, "winner line missing");
	if(h.errors.IsEmpty() && h.warnings.IsEmpty())
		AddAuditWarning(h, "no independent legality proof: observation trace is incomplete");
	out.Add(pick(h));
	h = HandAudit();
}

static void AuditSidecarEvent(Vector<HandAudit>& audits, HandAudit& h, char window,
	                           int hand, int second, const String& rest,
	                           int line_no, const String& raw_line,
	                           bool known_name[6], bool known_balance[6])
{
	if(rest.StartsWith("new hand")) {
		FinishHandAudit(h, audits);
		h.window = window;
		h.hand = hand;
		h.start_line = line_no;
		h.start_second = second;
		h.open = true;
		h.street = 0;
		for(int seat = 0; seat < 6; seat++) {
			h.named[seat] = known_name[seat];
			h.balance[seat] = known_balance[seat];
		}
		return;
	}
	if(!h.open) {
		if(rest.StartsWith("seat")) {
			int sp = rest.Find(' ');
			if(sp >= 0) {
				int seat = ParseSeatToken(rest.Left(sp), line_no, raw_line);
				String event = rest.Mid(sp + 1);
				if(event.StartsWith("name=")) {
					known_name[seat] = true;
					known_balance[seat] = event.Find("balance=") >= 0;
				}
			}
		}
		return;
	}
	if(rest.StartsWith("seat")) {
		int sp = rest.Find(' ');
		if(sp < 0) return;
		int seat = ParseSeatToken(rest.Left(sp), line_no, raw_line);
		String event = rest.Mid(sp + 1);
		if(event.StartsWith("name=")) {
			h.named[seat] = true;
			h.balance[seat] = event.Find("balance=") >= 0;
		known_name[seat] = h.named[seat];
		known_balance[seat] = h.balance[seat];
			return;
		}
		if(event == "wins") {
			h.winners++;
			return;
		}
		if(event.StartsWith("shows ")) {
			h.shows++;
			return;
		}
		if(event == "fold" || event == "check" || event.StartsWith("call")
		   || event.StartsWith("bet ") || event.StartsWith("raise ")) {
			if(h.last_action_seat == seat && h.last_action_second == second)
				AddAuditError(h, Format("duplicate action for seat%d at %02d:%02d:%02d",
			                              seat + 1, second / 3600, (second / 60) % 60, second % 60));
			if(event == "check" && h.street_bet > 0)
				AddAuditError(h, Format("seat%d checked while a street bet of %d x0.1BB was outstanding", seat + 1, h.street_bet));
			if(event.StartsWith("bet ") && h.street_bet > 0)
				AddAuditError(h, Format("seat%d bet after a street bet already existed; expected raise", seat + 1));
			if(event.StartsWith("raise ") && h.street_bet == 0)
				AddAuditError(h, Format("seat%d raised without an outstanding bet", seat + 1));
			if(event.StartsWith("bet ") || event.StartsWith("raise ") || event.StartsWith("call ")) {
				int p = event.Find(' ');
				if(p >= 0) {
					String amount = TrimBoth(event.Mid(p + 1));
					int paren = amount.Find(" (pot ");
					if(paren >= 0) amount = amount.Left(paren);
					if(amount.StartsWith("all-in ")) amount = amount.Mid(7);
					try {
						int value = ParseBBAmountX10(amount, line_no, raw_line);
						if(event.StartsWith("bet ") || event.StartsWith("raise ")) h.street_bet = value;
					}
					catch(const Exc&) {}
				}
			}
			h.actions[min(h.street, 3)]++;
			h.last_action_seat = seat;
			h.last_action_second = second;
		}
		return;
	}
	if(rest.StartsWith("flop ")) {
		if(h.street != 0) AddAuditError(h, "flop event out of street order");
		h.street = 1;
		h.street_bet = 0;
		return;
	}
	if(rest.StartsWith("turn ")) {
		if(h.street != 1) AddAuditError(h, "turn event out of street order");
		h.street = 2;
		h.street_bet = 0;
		return;
	}
	if(rest.StartsWith("river ")) {
		if(h.street != 2) AddAuditError(h, "river event out of street order");
		h.street = 3;
		h.street_bet = 0;
	}
}

static void PrintHandAudits(const Vector<HandAudit>& audits)
{
	int valid = 0, warnings = 0, errors = 0;
	for(const HandAudit& h : audits) {
		bool is_valid = h.errors.IsEmpty() && h.warnings.IsEmpty();
		valid += is_valid;
		warnings += h.warnings.GetCount();
		errors += h.errors.GetCount();
		Cout() << Format("hand_audit window=%c hand=%d valid=%d warnings=%d errors=%d actions=%d/%d/%d/%d shows=%d winners=%d\n",
		                 h.window, h.hand, is_valid, h.warnings.GetCount(), h.errors.GetCount(),
		                 h.actions[0], h.actions[1], h.actions[2], h.actions[3], h.shows, h.winners);
		for(const String& issue : h.warnings) Cout() << "  warning: " << issue << "\n";
		for(const String& issue : h.errors) Cout() << "  error: " << issue << "\n";
	}
	Cout() << Format("hand_audit_summary valid=%d warnings=%d errors=%d total=%d\n",
	                 valid, warnings, errors, audits.GetCount());
	Cout().Flush();
}

static Vector<TexasHoldemLogicState> ParseSidecar(const String& path, String& out_legend_summary,
	                                              String& out_window_summary,
	                                              Vector<HandAudit>& out_audits,
	                                              bool validate_hands)
{
	String content = LoadFile(path);
	if(content.IsEmpty())
		throw Exc(Format("sidecar file is empty or unreadable: %s", path));
	Vector<String> raw_lines = Split(content, '\n', false);
	for(String& l : raw_lines)
		if(l.GetCount() > 0 && l[l.GetCount() - 1] == '\r')
			l = l.Left(l.GetCount() - 1);

	int i = 0;
	SeatLegend legend = ParseLegend(raw_lines, i);

	out_legend_summary.Clear();
	for(int k = 0; k < legend.position_to_seat.GetCount(); k++) {
		if(k > 0) out_legend_summary << ", ";
		out_legend_summary << legend.position_to_seat.GetKey(k) << "=seat" << (legend.position_to_seat[k] + 1);
	}

	bool window_present[2] = { false, false };
	for(int line_index = i; line_index < raw_lines.GetCount(); line_index++) {
		String line = TrimBoth(raw_lines[line_index]);
		if(IsIgnorableSidecarLine(line) || line.GetCount() < 2 || line[1] != ' ')
			continue;
		int window_index = SidecarWindowIndex(line[0], line_index + 1, line);
		window_present[window_index] = true;
	}
	bool multi_window = window_present[0] && window_present[1];
	RunningState window_state[2];
	InitializeRunningState(window_state[0]);
	InitializeRunningState(window_state[1]);

	String session_id = GetFileTitle(path);

	Vector<TexasHoldemLogicState> out;
	bool have_base_ts[2] = { false, false };
	int base_ts[2] = { 0, 0 };
	int prev_frame_id[2] = { -1, -1 };
	int window_records[2] = { 0, 0 };
	HandAudit audits[2];
	int hand_numbers[2] = { 0, 0 };
	bool known_name[2][6] = {};
	bool known_balance[2][6] = {};

	for(; i < raw_lines.GetCount(); i++) {
		String raw_line = TrimBoth(raw_lines[i]);
		if(IsIgnorableSidecarLine(raw_line))
			continue;
		int line_no = i + 1;

		char window = 0;
		String ts_str, rest;
		SplitEventLine(raw_line, line_no, window, ts_str, rest);
		int window_index = SidecarWindowIndex(window, line_no, raw_line);
		RunningState& rs = window_state[window_index];
		int ts_seconds = ParseTimestampSeconds(ts_str, line_no, raw_line);
		if(!have_base_ts[window_index]) {
			base_ts[window_index] = ts_seconds;
			have_base_ts[window_index] = true;
		}

		int elapsed = ts_seconds - base_ts[window_index];
		int frame_id = max(elapsed, prev_frame_id[window_index] + 1);
		prev_frame_id[window_index] = frame_id;

		if(validate_hands) {
			if(rest.StartsWith("new hand")) hand_numbers[window_index]++;
			AuditSidecarEvent(out_audits, audits[window_index], window, hand_numbers[window_index],
			                  ts_seconds, rest, line_no, raw_line,
			                  known_name[window_index], known_balance[window_index]);
		}
		ApplyEvent(rs, rest, line_no, raw_line);

		String event_session_id = session_id;
		if(multi_window) event_session_id << ":" << window;
		TexasHoldemLogicState st = Emit(rs, frame_id, (int64)ts_seconds * 1000, event_session_id);
		out.Add(pick(st));
		window_records[window_index]++;
	}
	if(validate_hands) {
		FinishHandAudit(audits[0], out_audits);
		FinishHandAudit(audits[1], out_audits);
	}
	out_window_summary = Format("R=%d, L=%d", window_records[0], window_records[1]);
	return out;
}

END_UPP_NAMESPACE

using namespace Upp;

GUI_APP_MAIN
{
#ifdef PLATFORM_WIN32
	AttachConsole(ATTACH_PARENT_PROCESS);
#endif
	SetVppLogName(AppendFileName(GetCurrentDirectory(), "VideoSidecarGroundTruthParser.log"));
	RLOG("VideoSidecarGroundTruthParser started");

	const Vector<String>& args = CommandLine();
	String sidecar_path, out_jsonl_path;
	bool validate_hands = true;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--sidecar" && i + 1 < args.GetCount()) sidecar_path = args[++i];
		else if(args[i] == "--out-jsonl" && i + 1 < args.GetCount()) out_jsonl_path = args[++i];
		else if(args[i] == "--no-validate-hands") validate_hands = false;
	}

	if(sidecar_path.IsEmpty() || out_jsonl_path.IsEmpty()) {
		Cout() << "VideoSidecarGroundTruthParser CLI (Task 0262)\n"
		       << "Usage: VideoSidecarGroundTruthParser --sidecar <sidecar.txt> --out-jsonl <out.jsonl> [--no-validate-hands]\n";
		SetExitCode(1);
		return;
	}
	if(!FileExists(sidecar_path)) {
		Cerr() << "Input sidecar file does not exist: " << sidecar_path << "\n";
		SetExitCode(1);
		return;
	}

	Vector<TexasHoldemLogicState> records;
	Vector<HandAudit> audits;
	String legend_summary, window_summary;
	try {
		records = ParseSidecar(sidecar_path, legend_summary, window_summary, audits, validate_hands);
	}
	catch(const Exc& e) {
		Cerr() << "PARSE ERROR: " << e << "\n";
		SetExitCode(1);
		return;
	}

	Cout() << "Seat-position legend (local to this recording, not consumed by the emitted schema): " << legend_summary << "\n";
	Cout() << "Window records: " << window_summary << "\n";
	Cout() << "Parsed " << records.GetCount() << " event line(s) from " << sidecar_path << "\n";
	if(validate_hands)
		PrintHandAudits(audits);
	Cout().Flush();

	String out_text;
	for(const TexasHoldemLogicState& st : records)
		out_text << StoreAsJson(st) << "\n";

	RealizeDirectory(GetFileFolder(out_jsonl_path));
	if(!SaveFile(out_jsonl_path, out_text)) {
		Cerr() << "Failed to write output JSONL file: " << out_jsonl_path << "\n";
		SetExitCode(1);
		return;
	}
	Cout() << "Wrote " << records.GetCount() << " record(s) to " << out_jsonl_path << "\n";
}
