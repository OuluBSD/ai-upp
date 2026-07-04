# Dynamic OCR expected_text (task 0070, gap #4)

## Problem

`VsmOcrRule.expectation` (`uppsrc/VisualStateModel/OcrLayer.h`) used to carry a
single static `expected_text` string per rule. That works for a UI label that
never changes, but Hearts' own `.gamestate` `ocr_expected` values (e.g.
`"You  T:0  R:+0  C:13"`, produced by `update_hud()` in
`uppsrc/ScriptIDE/reference/Hearts/main.py`) are only valid for the just-dealt
frame — the card count and scores change every trick. There was no facility
to regenerate `expected_text` from a live model snapshot mid-round.

## Mechanism

`VsmTextExpectMode` gained a third mode, `VSM_EXPECT_DYNAMIC`, alongside
`VSM_EXPECT_EXACT` and `VSM_EXPECT_CONTAINS`. `VsmTextExpectation` gained a
`template_text` field, used instead of `expected_text` when `mode ==
VSM_EXPECT_DYNAMIC`.

A free function resolves the template against a parsed `state_json` `Value`:

```cpp
String VsmResolveDynamicText(const String& template_text, const Value& state,
                              Vector<String>* missing_fields = nullptr);
```

### Template syntax

Deliberately minimal — this is field substitution, not an expression
language:

- `{field}` — substitutes the scalar value of `state[field]` (via
  `Value::ToString()`).
- `{field[N]}` — substitutes the Nth element of the array-valued
  `state[field]` (matches the 3-tier `state_json` schema's shape, e.g.
  `{round_scores[0]}`).

Example: template `"Score: {round_scores[2]}, phase={phase}"` against
`{"round_scores":[1,0,7,4],"phase":"PLAYING"}` resolves to
`"Score: 7, phase=PLAYING"`.

### Missing fields are loud, not silent

If a referenced field is absent from `state` (unknown key, out-of-range
index, or `state` itself null/empty), the original `{...}` placeholder text
is left untouched in the output — it does **not** silently collapse to an
empty string. Each unresolved placeholder is also:

- logged via `RLOG` (`VsmResolveDynamicText: unresolved field '...' ...`), and
- appended to the caller-supplied `missing_fields` out-param, if one is
  passed.

Either signal lets a caller detect that a template didn't fully resolve.

## Wiring into `VsmOcrExecutor::Compare()`

```cpp
VsmOcrComparison Compare(const VsmOcrResult& result, const VsmOcrRule& rule,
                          const Value& live_state = Value());
```

When `rule.expectation.mode == VSM_EXPECT_DYNAMIC`, `Compare()` resolves
`VsmResolveDynamicText(rule.expectation.template_text, live_state)` to get the
effective expected text, then compares it to `result.text` exactly (same as
`VSM_EXPECT_EXACT`). Any unresolved fields are logged as a warning but do not
by themselves fail the comparison — the placeholder-preserved text simply
won't match the OCR text (turning it into a normal `VSM_OCR_WARNING`
mismatch) unless it does.

If `live_state` is null/empty while `mode == VSM_EXPECT_DYNAMIC`, that is
treated as a rule-configuration error: `Compare()` returns `VSM_OCR_ERROR_S`
immediately (logged via `LogError`) rather than silently passing or falling
back to a stale static string.

## Scope note / follow-up for Hearts

This task is the generic, headless mechanism only
(`uppsrc/VisualStateModel/OcrLayer.h`/`.cpp`). It intentionally does **not**:

- wire this into `VsmHeartsSource` or `reference/CardGameStateAdapter/`, or
- add a `hand_count` (per-player hand-card-count) field to
  `docs/VisualStateModel/CARD_GAME_STATE_SCHEMA.md`.

Hearts' HUD label format is `"{name}  T:{scores}  R:+{round_scores}
C:{hand_count}"`. The `hand_count` value has no home in the current
3-tier `state_json` schema (`CARD_GAME_STATE_SCHEMA.md` only has
`round_scores`/`scores` arrays, no per-player hand-card-count field). Whoever
wires OCR into `VsmHeartsSource` (task 0073) needs to either add that schema
field or derive hand count from existing data before using this mechanism to
reconstruct the exact Hearts HUD string end-to-end.
