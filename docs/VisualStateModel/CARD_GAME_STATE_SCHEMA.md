# Card-Game `state_json` Schema (card-play / trick / round)

This document closes part of `plan/VisualStateModel/0066_card_game_state_schema_and_canonical_json_compare.md`,
which follows on from `docs/VisualStateModel/HEARTS_SOURCE_INVESTIGATION.md`
(task 0055). It defines the canonical field set for `VsmModelStateRef::state_json`
/ `state_snapshot` events produced by card-game ground-truth sources
(`source_type: "game_export"`), starting with Hearts. It does not introduce
any new C++ type — `state_json` remains a serialized JSON object string on
the existing `VsmModelStateRef`/`state_snapshot` shape defined in
`GROUND_TRUTH.md`.

Field **order** inside `state_json` is not significant. Comparisons must go
through `VsmCanonicalJsonEqual()` (`uppsrc/VisualStateModel/GroundTruth.h`),
which compares JSON objects structurally, independent of key order and
whitespace, so producers do not need to agree on emission order — only on
field names and value shapes.

Derived from `hearts/logic.py::GameState`
(`uppsrc/ScriptIDE/reference/Hearts/hearts/logic.py`), the only complete,
readable rules implementation in-tree today (see Section 3 of
`HEARTS_SOURCE_INVESTIGATION.md` for the full derivation).

---

## Tier 1 — `card_play`

One event per individual card played (up to 52 per round, fewer on a
hold-round with skipped passing). This is the finest granularity; use it when
a divergence needs to be pinned to a specific play, not just a trick.

```json
{
  "tier": "card_play",
  "round_number": 3,
  "phase": "PLAYING",
  "turn": 1,
  "trick_number": 5,
  "leading_suit": "hearts",
  "hearts_broken": true,
  "player": 0,
  "card_played": "QS",
  "hand_counts": [8, 9, 9, 9]
}
```

| Field | Type | Meaning |
|---|---|---|
| `tier` | string | Always `"card_play"` for this event shape. |
| `round_number` | int | 1-based round counter (matches `GameState.round_number`). |
| `phase` | string | `"PASSING"` \| `"PLAYING"` \| `"ROUND_END"` \| `"GAME_OVER"`. |
| `turn` | int | Player index (0–3) whose turn it is *after* this play resolves. |
| `trick_number` | int | 1-based trick counter within the round (1–13). |
| `leading_suit` | string | Suit of the trick's first card, or `""` before any card is played. |
| `hearts_broken` | bool | Whether hearts have been broken this round. |
| `player` | int | Player index (0–3) who just played `card_played`. |
| `card_played` | string | Two-character card code, rank+suit (e.g. `"QS"` = Queen of Spades). |
| `hand_counts` | array[4], optional | Per-player remaining-card count, `len(state.players[i])` for each `i` (0–3), read live at the same instant as the other fields. Optional/additive (task 0073) — closes task 0070's flagged follow-up, needed to validate the on-screen HUD label's `"C:<n>"` segment (`main.py:394-407`'s `update_hud()`) without requiring a new schema tier. Producers that don't emit it are still schema-valid; consumers must not require it. |

## Tier 2 — `trick`

One event per resolved trick (13 per round). This is the natural unit for
`VsmDivergence` — "expected winner" vs "observed winner" is meaningful and
cheap to assert here.

```json
{
  "tier": "trick",
  "round_number": 3,
  "trick_number": 5,
  "trick_winner": 2,
  "trick_points": 3,
  "round_scores": [4, 0, 9, 2]
}
```

| Field | Type | Meaning |
|---|---|---|
| `tier` | string | Always `"trick"`. |
| `round_number` | int | Same as Tier 1. |
| `trick_number` | int | 1-based trick counter (1–13) that just resolved. |
| `trick_winner` | int | Player index (0–3) who won this trick. |
| `trick_points` | int | Points collected in this trick (hearts=1 each, queen of spades=13). |
| `round_scores` | array[4] | Running point total per player for the current round, post-trick. |

## Tier 3 — `round`

One event at `resolve_round()` — the natural unit for a `replay_checkpoint`
(see `GROUND_TRUTH.md`'s `replay_checkpoint` event type).

```json
{
  "tier": "round",
  "round_number": 3,
  "round_scores": [4, 0, 22, 2],
  "scores": [30, 12, 22, 40],
  "moon_shooter": -1,
  "game_over": false
}
```

| Field | Type | Meaning |
|---|---|---|
| `tier` | string | Always `"round"`. |
| `round_number` | int | Round that just ended. |
| `round_scores` | array[4] | Final per-player points for this round (post shoot-the-moon adjustment). |
| `scores` | array[4] | Cumulative game score per player after this round. |
| `moon_shooter` | int | Player index who shot the moon this round, or `-1` if none. |
| `game_over` | bool | `true` once any player's `scores[i] >= 100`. |

---

## Usage notes

- All three tiers share `round_number` so a divergence tool can correlate a
  card-play-level mismatch back to its containing trick/round without a
  separate lookup table.
- `state_json` for a given tier must contain exactly the fields listed above
  (no additional required fields); optional producer-specific fields may be
  added and are ignored by `VsmCanonicalJsonEqual()`-based comparisons as
  long as both sides carry them consistently — but note unstated fields
  present on only one side will still fail the comparison, since it is a
  full structural match, not a subset match. Producers and consumers must
  agree on the full field set they emit.
- This schema is Hearts-shaped today but intentionally generic
  (`card_played`, not `hearts_card_played`) so other trick-taking card games
  can reuse it without a schema bump, as long as they map onto the same
  three tiers. A different game genre (non-trick-taking) should propose a
  new tier set rather than overloading this one.
- Card code format: `<rank><suit>`, rank ∈ `{2-9,T,J,Q,K,A}`, suit ∈
  `{C,D,H,S}` (Clubs/Diamonds/Hearts/Spades), e.g. `"TH"` = Ten of Hearts.
