# CardGameStateAdapter

Thin C++ adapter that converts a live `CardGameDocumentHost` (ScriptIDE's
embedded card-game Python runtime, `uppsrc/ScriptIDE/CardGamePlugin.h`) into
`VisualStateModel`'s `state_json` schema for card-play/trick/round tiers
(`docs/VisualStateModel/CARD_GAME_STATE_SCHEMA.md`).

Built from `plan/VisualStateModel/0068_cardgame_state_adapter.md`; see
`docs/VisualStateModel/CARD_GAME_ADAPTER.md` for what it exposes and the
field-mapping/derivation notes (in particular: why `trick_number` is
adapter-derived, and why the live Python `state` object is read via
`sys.modules[<entry module>]`, not `PyVM::GetGlobals()` directly).

## Why this is a separate `reference/` package, not `uppsrc/VisualStateModel/`

`uppsrc/VisualStateModel/` must stay headless (`uses Core;` only, no GUI
deps). This adapter depends on `ScriptIDE`, which is GUI-only in every
mainconfig it ships (`bin/build --list-mainconfigs ScriptIDE` — all entries
include `GUI`). So the adapter lives here instead, `uses`-ing both
`VisualStateModel` (headless, for `AsJSON`/JSON conventions) and `ScriptIDE`
(for `CardGameDocumentHost`) plus `ScriptCommon` (for `PyVM`/`PyValue`
visibility and `CardGameSprite`).

## Files

- `VsmCardGameStateExport.h`/`.cpp` — the adapter class.
- `main.cpp` — headless demo/test: loads
  `uppsrc/ScriptIDE/reference/Hearts/game.gamestate`, runs `ExecuteSync()`,
  exports one `card_play`, one `trick`, and one `round` tier `state_json`,
  and self-checks each against `VsmCanonicalJsonEqual()` and the schema's
  required field set.

## Header include policy

Only `CardGameStateAdapter.h` (the main header) includes other headers,
before entering `NAMESPACE_UPP`, per the repo's BLITZ convention
(`AGENTS.md` "Header Include Policy").
