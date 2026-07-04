# CardGameDocumentHost → `state_json` Adapter

Closes `plan/VisualStateModel/0068_cardgame_state_adapter.md`. Builds the
"thin C++ adapter" `docs/VisualStateModel/HEARTS_SOURCE_INVESTIGATION.md`'s
chosen seam (c) needs: converting `CardGameDocumentHost`'s live Python
`GameState` object into `docs/VisualStateModel/CARD_GAME_STATE_SCHEMA.md`'s
three tiers (`card_play`/`trick`/`round`), so task 0069 (`VsmHeartsSource`)
has something concrete to call.

## Where it lives, and why

`reference/CardGameStateAdapter/` — a new buildable `reference/` package,
**not** `uppsrc/VisualStateModel/`. `uppsrc/VisualStateModel/` must stay
headless (`uses Core;` only). This adapter needs `CardGameDocumentHost`
(`uppsrc/ScriptIDE/CardGamePlugin.h`), and every one of `ScriptIDE`'s
mainconfigs requires `GUI` (`bin/build --list-mainconfigs ScriptIDE` →
`GUI`/`USEMALLOC`/`X11`/`X11_USEMALLOC` variants, all GUI), so this
dependency could not be avoided by choosing a different integration shape —
confirmed before writing any code, per the 0068 plan file.

Files:
- `reference/CardGameStateAdapter/CardGameStateAdapter.upp` — `uses
  VisualStateModel, ScriptCommon, ScriptIDE;`, one `mainconfig "" = "GUI";`
  (mirrors `ScriptIDE.upp`'s own minimal GUI mainconfig).
- `CardGameStateAdapter.h` — main header (BLITZ aggregator).
- `VsmCardGameStateExport.h`/`.cpp` — the adapter class.
- `main.cpp` — headless demo/test (`GUI_APP_MAIN`, see below).

## API

```cpp
class VsmCardGameStateExport {
public:
    String ExportCardPlayState(CardGameDocumentHost& host, int player, const String& card_played);
    String ExportTrickState(CardGameDocumentHost& host, int trick_number, int trick_winner, int trick_points);
    String ExportRoundState(CardGameDocumentHost& host, int round_number);
    // ... internal trick_number tracking state, see header comments.
};
```

Each method builds a `ValueMap` matching one schema tier exactly and returns
`AsJSON(v)` — no new C++ struct, per the plan's requirement 3 (`state_json`
stays a plain serialized-JSON `String`, same as any other producer of
`VsmModelStateRef::state_json`).

## Deviation #1 (important): `state` is not reachable via `PyVM::GetGlobals()`

The 0068 plan file suggested:

```cpp
PyValue globals = vm->GetGlobals();
PyValue state = globals.GetItem(PyValue("state"));
```

**This does not work**, and the adapter does not use it. Root cause, found
by reading `CardGamePlugin::Execute()` (`uppsrc/ScriptCommon/CardGamePlugin.cpp:204-414`)
and `PyVM::LoadModule()` (`uppsrc/ByteVM/Python/PyVM.cpp:2073-2156`) in full,
not assumed:

1. `Execute()` loads the `.gamestate`'s entry script (`main.py`) via
   `vm->LoadModule(entry_module_name, ...)`. `LoadModule()` runs the
   module's top-level code against **its own module dict** (`mod_dict`) as
   that frame's globals (`f.globals = mod_dict;`), and stores `mod_dict` in
   `sys.modules[entry_module_name]`.
2. At `main.py`'s top level, `state = None`. `GameState()` is only
   constructed later, inside `start()` (main.py:127), against `main.py`'s
   own `mod_dict` — `start()` is not called yet at this point.
3. **After** module-level execution finishes but **before** calling
   `entry_function` ("start"), `Execute()` copies `mod_dict`'s entries into
   `vm->GetGlobals()` once, key-by-key (`globals.GetAdd(items.GetKey(i)) =
   items[i];`). At this moment, `state` is still `None`, so
   `vm->GetGlobals()["state"]` becomes an independent copy of that `None`
   scalar.
4. `Execute()` then calls `start()`, which reassigns `mod_dict["state"] =
   GameState()` — a rebind of one entry in `mod_dict`'s own internal
   `VectorMap`. Because step 3 already copied the *old* `None` value out by
   value into a separate dict object, this reassignment has no way to reach
   `vm->GetGlobals()["state"]`, which stays stuck on the stale `None`
   forever after.

**Fix used**: read `state` from `sys.modules[<entry module name>]` (the
actual `mod_dict` object `start()` mutates), not from `vm->GetGlobals()`
directly. `<entry module name>` is `GetFileTitle()` of the `.gamestate`'s
`entry_script` field (`"main.py"` → `"main"`) — the same derivation
`CardGamePlugin::Execute()` itself uses (`entry_module_name =
GetFileTitle(entry_script)`, `CardGamePlugin.cpp:348`). `CardGamePlugin`
keeps `entry_module_name` `protected`, and `CardGameDocumentHost`'s
`plugin`/`runtime_plugin` members are private, so neither is reachable
directly from a `CardGameDocumentHost&`; the adapter re-derives it instead
by re-reading the `.gamestate` JSON via the already-public
`CardGameDocumentHost::GetPath()`. This isn't a workaround for something
broken — the `.gamestate` JSON is public, on-disk data, and this replicates
exactly the same one-line lookup `Execute()` already does internally; no
changes to `CardGameDocumentHost`/`CardGamePlugin` were made or needed.
See `FindEntryModuleDict()` in `VsmCardGameStateExport.cpp` for the full
derivation and comments.

## Deviation #2: `PyValue::GetAttr()` does not exist

The plan file also suggested `state.GetAttr("phase")`. `PyValue` (
`uppsrc/ByteVM/Python/PyValue.h`) has no `GetAttr()` method — the only
`GetAttr()` in the codebase is a *virtual method on `PyUserData`*
(`PyValue.h:204`), used for C++-backed Python objects (bound host
functions/modules etc.), not for plain Python class instances. Plain Python
class instances (like `GameState`/`Card`) are represented as `PY_DICT`
`PyValue`s carrying a `"__class__"` entry (see the `PY_CALL`
class-instantiation code and `PY_LOAD_ATTR`'s dict-instance branch,
`uppsrc/ByteVM/Python/PyVM.cpp:2595-2618`), and attribute reads go through
`GetItem()`, with a fallback to the class dict for class-level
defaults/methods. The adapter's `GetAttr()` helper
(`VsmCardGameStateExport.cpp`) mirrors that exact fallback rather than
assuming plain `GetItem()` always suffices (it happens to, for every field
this adapter reads, since `GameState.__init__` sets them all as instance
attributes — but the helper does the fully-general thing anyway).

## Field mapping

| Schema tier | Schema field | Source |
|---|---|---|
| card_play | `round_number`, `phase`, `turn`, `leading_suit`, `hearts_broken` | live `state.<field>` (same names) |
| card_play | `player`, `card_played` | caller-supplied (GameState has no "last play" record) |
| card_play | `trick_number` | **adapter-derived**, see below |
| trick | `round_number` | live `state.round_number` |
| trick | `trick_number`, `trick_winner`, `trick_points` | caller-supplied |
| trick | `round_scores` | live `state.round_scores` (running in-round tally) |
| round | `round_number` | caller-supplied |
| round | `round_scores` | live `state.last_round_scores` (**not** `state.round_scores`; see note below) |
| round | `scores` | live `state.scores` |
| round | `moon_shooter` | live `state.last_round_moon_shooter` |
| round | `game_over` | live `state.game_over` |

**`round_scores` note**: Tier 2's `round_scores` ("running point total …
post-trick") and Tier 3's `round_scores` ("final … post shoot-the-moon
adjustment") are two different `GameState` fields, despite sharing a schema
field name. `resolve_round()` (`hearts/logic.py:254-290`) computes the
shoot-the-moon adjustment into `self.round_scores` in place, then snapshots
it into `self.last_round_scores` — but `self.round_scores` itself is not
reset to `[0,0,0,0]` until the *next* `deal()`. Using `last_round_scores`
for Tier 3 is correct regardless of how much time/how many calls pass
between `resolve_round()` and the export call; using live `round_scores`
would coincidentally also work immediately after `resolve_round()` but
silently break once another round starts. Not used, on purpose.

**`trick_number` derivation** (Tier 1 only): `hearts/logic.py::GameState`
has no trick-sequence counter (confirmed by reading the whole file — the
closest field is `last_trick_winner`, set only when a trick resolves). The
adapter tracks `resolved_trick_count` privately, incrementing it whenever
`state.last_trick_winner` changes value between calls to
`ExportCardPlayState()`; `trick_number` is emitted as
`resolved_trick_count + 1` (the trick currently in progress).

**Known limitation, documented rather than hidden**: if the same player
wins two consecutive tricks, `last_trick_winner` does not change between
those two resolutions, so the tracker misses the second increment and
`trick_number` under-counts by one from that point on for the rest of the
round. There is no way to detect this from `GameState`'s current fields
alone. A real fix means adding an explicit trick-sequence counter field to
`hearts/logic.py::GameState` itself, which is out of this adapter's scope
(it would change the ground-truth Python engine, not just the C++ read
side) — flagged here as a follow-up for whoever next touches
`hearts/logic.py`, rather than silently accepted or paved over.

## `PyValue::ToValue()` recursion — verified, not assumed

Read `PyValue::ToValue()`'s implementation (`uppsrc/ByteVM/Python/PyValue.cpp:300-321`)
before relying on it, per the plan's instruction. It recurses correctly for
`PY_LIST` (each element re-dispatched through `ToValue()`, producing a
`ValueArray`) and for `PY_DICT` (a `ValueMap`) — sufficient for this
adapter's `list[int]` fields (`round_scores`, `scores`, `last_round_scores`).
It does **not** handle `PY_TUPLE` (falls into the `default: return Value();`
branch, silently becoming null) — irrelevant to this adapter's three tiers,
but relevant if a future tier ever needs `GameState.trick` (a `list` of
`(player_index, Card)` tuples): that would need a manual
`GetItem(0)`/`GetItem(1)` loop, not `.ToValue()`.

## Demo / test

`reference/CardGameStateAdapter/main.cpp` (`GUI_APP_MAIN` — required because
`CardGameDocumentHost` derives from `Ctrl`, even though nothing is ever
displayed; `ExecuteSync()`/`CaptureRecordFrame()` both run the
offscreen-`SImageDraw` path with no display-server dependency, per
`HEARTS_SOURCE_INVESTIGATION.md`'s 2026-07-04 addendum):

1. Loads `uppsrc/ScriptIDE/reference/Hearts/game.gamestate` into a
   `CardGameDocumentHost`, calls `SetFixedArea()`, `Load()`, `ExecuteSync()`.
2. Exports one `card_play`, one `trick`, and one `round` tier `state_json`.
3. Asserts each result's field set matches
   `docs/VisualStateModel/CARD_GAME_STATE_SCHEMA.md` exactly (no
   missing/extra fields), asserts score arrays are length 4, and asserts
   each JSON string round-trips through `VsmCanonicalJsonEqual()` against
   itself.

Since `game.gamestate`'s entry function (`start()`) runs once, synchronously,
with no `--autoplay` argv and no event loop (`ExecuteSync()` never pumps
`SetTimeout`-scheduled callbacks), the observed game state after
`ExecuteSync()` returns is simply "round 1, freshly dealt, `PASSING` phase"
— `round_scores`/`scores` are `[0,0,0,0]`, `leading_suit` is `None` (emitted
as `""` per schema), `last_trick_winner`/`last_round_moon_shooter` are `-1`.
This is still a fully valid instance of each schema tier (the schema does
not require nonzero values), and exercises every field-mapping/derivation
path described above with real data read through the real
`CardGameDocumentHost`/`PyVM` plumbing — it does not fabricate a
`GameState`.

Build & run:

```
bin/build --list-mainconfigs CardGameStateAdapter   # [0] GUI = GUI
bin/build --mainconfig 0 --release --jobs 12 CardGameStateAdapter
bin/CardGameStateAdapter
```

Sample output (real run):

```
card_play: {"tier":"card_play","round_number":1,"phase":"PASSING","turn":0,"trick_number":1,"leading_suit":"","hearts_broken":false,"player":0,"card_played":"2C"}
trick: {"tier":"trick","round_number":1,"trick_number":1,"trick_winner":2,"trick_points":5,"round_scores":[0,0,0,0]}
round: {"tier":"round","round_number":1,"round_scores":[0,0,0,0],"scores":[0,0,0,0],"moon_shooter":-1,"game_over":false}
All CardGameStateAdapter checks passed.
```

(Debug mainconfigs also work but trip the debug heap-leak checker's `PANIC`
at process exit — this is `CardGameDocumentHost`'s own Ctrl/Image/PyVM
teardown path, unrelated to this adapter's own allocations, and matches
`AGENTS.md`'s documented "debug builds may show false positives; always use
Valgrind for confirmation" guidance. The `--release` build above exits 0
cleanly.)

## Non-goals / what this does not do

- Does not drive the game forward (that's task 0069's `VsmHeartsSource`,
  using 0067's step-driving mode).
- Does not touch `CardGameDocumentHost`/`CardGamePlugin` — everything is
  read through already-public methods (`GetVM()`, `GetPath()`) plus one
  re-derivation of data (`entry_module_name`) that those classes already
  compute internally but do not expose.
- Does not add a trick-sequence counter to `hearts/logic.py::GameState`
  (flagged above as a real follow-up, not done here).
