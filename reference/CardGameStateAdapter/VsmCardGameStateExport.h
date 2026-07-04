// Adapter: CardGameDocumentHost's live Python `GameState` -> VSM state_json.
// Field sets must match docs/VisualStateModel/CARD_GAME_STATE_SCHEMA.md exactly
// (card_play / trick / round tiers). See docs/VisualStateModel/CARD_GAME_ADAPTER.md
// for the field-mapping notes and known limitations.

class VsmCardGameStateExport {
public:
	// Tier 1 ("card_play"): one event per card played. `player`/`card_played`
	// describe the play that was just made (the adapter has no way to observe
	// this itself — GameState does not record "last card played"); every other
	// field is read live from the Python `state` object driving `host`.
	// `trick_number` is derived internally (see TrackTrickNumber()) because
	// `GameState` has no trick-sequence counter of its own.
	String ExportCardPlayState(CardGameDocumentHost& host, int player, const String& card_played);

	// Tier 2 ("trick"): one event per resolved trick. `trick_number`/
	// `trick_winner`/`trick_points` are supplied by the caller (the driver
	// calling this already knows exactly which trick just resolved and why);
	// `round_scores` is read live from `state.round_scores` (the running,
	// in-progress-round tally, per the schema's Tier 2 field description).
	String ExportTrickState(CardGameDocumentHost& host, int trick_number, int trick_winner, int trick_points);

	// Tier 3 ("round"): one event at resolve_round(). `round_number` is
	// supplied by the caller; `round_scores`/`scores`/`moon_shooter`/
	// `game_over` are read live from `state` — note this tier's
	// `round_scores` reads `state.last_round_scores` (the final,
	// post-shoot-the-moon tally captured by resolve_round()), NOT the same
	// `state.round_scores` field Tier 2 reads, per the schema's Tier 3
	// description ("post shoot-the-moon adjustment").
	String ExportRoundState(CardGameDocumentHost& host, int round_number);

private:
	// Best-effort trick_number tracking for Tier 1, which has no equivalent
	// caller-supplied parameter. Counts resolved tricks by watching
	// `state.last_trick_winner` for changes across calls.
	//
	// KNOWN LIMITATION (documented, not silently patched): if the same
	// player wins two consecutive tricks, `last_trick_winner` does not
	// change between those two resolutions, so this under-counts by one.
	// `hearts/logic.py::GameState` has no trick-sequence counter that could
	// disambiguate "still trick N" from "already trick N+1, same winner" —
	// confirmed absent by reading the whole file before adding this
	// workaround. A real fix requires adding a counter field to GameState
	// itself (out of scope for this adapter; noted as follow-up in
	// docs/VisualStateModel/CARD_GAME_ADAPTER.md).
	int last_seen_trick_winner = -1;
	int resolved_trick_count   = 0;
	void TrackTrickNumber(const PyValue& state);
};
