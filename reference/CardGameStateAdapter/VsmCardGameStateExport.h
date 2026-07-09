// Adapter: CardGameDocumentHost's live Python `GameState` -> VSM state_json.
// Field sets must match Manager/2-plan/ai-upp/root/VisualStateModel/docs/CARD_GAME_STATE_SCHEMA.md exactly
// (card_play / trick / round tiers). See Manager/2-plan/ai-upp/root/VisualStateModel/docs/CARD_GAME_ADAPTER.md
// for the field-mapping notes and known limitations.

class VsmCardGameStateExport {
public:
	// Tier 1 ("card_play"): one event per card played. `player`/`card_played`
	// describe the play that was just made (the adapter has no way to observe
	// this itself — GameState does not record "last card played"); every other
	// field is read live from the Python `state` object driving `host`.
	// `hand_counts` (task 0073, CARD_GAME_STATE_SCHEMA.md) is optional/additive:
	// per-player `len(state.players[i])`, read live the same way
	// `round_scores`/`scores` already are for the other tiers.
	//
	// `trick_number` is caller-supplied, same as ExportTrickState()/
	// ExportRoundState() below (follow-up fix, post-0073): this used to be
	// derived internally via TrackTrickNumber()'s last_trick_winner-diff
	// heuristic, which undercounts by one whenever the same player wins two
	// consecutive tricks in a row (no equivalent counter exists on
	// `GameState` itself). A driver like VsmHeartsSource::Step() already
	// knows the exact trick count synchronously — it is the one calling
	// finish_trick_collect() — so there is no need to infer it from polling
	// Python state at all; removed the heuristic in favor of that.
	String ExportCardPlayState(CardGameDocumentHost& host, int player,
	                           const String& card_played, int trick_number);

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

	// Locates the live `sys.modules[<entry module name>]` dict for `host` --
	// the same module dict `main.py`'s top-level code and `start()` mutate
	// (see VsmCardGameStateExport.cpp's header comment for why this, and
	// not PyVM::GetGlobals(), is the correct place to read/write `state`
	// and module-level globals like `autoplay_enabled`). Exposed publicly
	// (task 0069's VsmHeartsSource reuses this exact lookup rather than
	// re-deriving it) after having been a file-local static here (task 0068).
	static PyValue FindEntryModuleDict(CardGameDocumentHost& host, PyVM& vm);
};
