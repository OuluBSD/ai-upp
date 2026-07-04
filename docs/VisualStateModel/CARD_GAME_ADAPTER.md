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

---

# VsmHeartsSource (task 0069)

Closes `plan/VisualStateModel/0069_vsm_hearts_source.md`. Adds
`VsmHeartsSource : VsmSteppedFrameSource` (`reference/CardGameStateAdapter/VsmHeartsSource.h`/`.cpp`),
which drives `uppsrc/ScriptIDE/reference/Hearts/game.gamestate` one logical
step at a time and captures a frame + `state_json` (via the
`VsmCardGameStateExport` above) after each step. Lives in the same
`reference/CardGameStateAdapter/` package (extended, not a new sibling
package) since it builds directly on `VsmCardGameStateExport::FindEntryModuleDict()`,
now made a public static method (was file-local `static` in
`VsmCardGameStateExport.cpp`) precisely so this file could reuse it instead
of re-deriving the `sys.modules[entry_module_name]` lookup.

## Correcting 0067's own plan assumption

0067 assumed 0069 would need a new Python-side stepping entry point. It
does not: `main.py`'s existing `ai_step()` (`main.py:828-897`) already is
the one-step unit; it is normally invoked via a `cardgame_view.set_timeout()`
timer chain, which never fires under `ExecuteSync()` (no event loop pumps
timers). `PyVM::Call(fn, {})` on `ai_step` obtained from the live module
dict **is** the step mechanism — confirmed by reading `PY_MAKE_FUNCTION`
(`PyVM.cpp`, `l2->globals = frame.globals`) and `PyVM::Call()`
(`f.globals = l.globals.IsNone() ? globals : l.globals`) in full: every
function defined at `main.py`'s top level captures that same module dict as
its globals at definition time, so writing into the module dict (e.g.
forcing `autoplay_enabled = True`) is visible to `ai_step()` on the very
next call, with no VM-level indirection to work around.

## The four `set_timeout` call sites, and what `Step()` does about each

Grepped exhaustively (`grep -n "cardgame_view.set_timeout(" main.py`):

| Call site | Schedules | `Step()`'s handling |
|---|---|---|
| `schedule_ai_step()` (main.py:716) | `ai_step` | This *is* the driver's main loop unit — called directly, once per `Step()`. |
| `start_pass_animation()` (main.py:759) | `finish_pass_animation` | `start_pass_animation()` itself runs *synchronously* inside the `ai_step()` call that completes the 4th player's pass (main.py:867); `Step()` checks `pass_animating` in the module dict right after that `ai_step()` call and, if true, calls `finish_pass_animation()` itself, in the same `Step()`. |
| `start_trick_collect()` (main.py:791) | `finish_trick_collect` | `start_trick_collect()` runs on the *next* `ai_step()` entry after the 4th card of a trick is played (main.py:840-843), setting `collecting_trick=True` and returning without resolving. `Step()` checks `state.trick_pending` after that `ai_step()` call and, if still true, calls `finish_trick_collect()` itself, in the same `Step()`. |
| `finish_trick_collect()` (main.py:811) | `next_round` | **Deliberately never driven.** `next_round()` only matters for beginning round 2+ (`begin_next_round()` → `deal()`), which is out of this task's one-round scope (`hearts/logic.py::resolve_trick()` already calls `resolve_round()` *synchronously* — main.py:251-252 — the instant the round's last trick resolves, so every Tier-3 field is populated in `state` before `next_round()` would even be scheduled). Not calling it does not stall anything: `ai_step()`'s own guard (`if state.phase != 'PLAYING': return`, reached once `phase` is `ROUND_END`) makes every subsequent `ai_step()` call a safe no-op. |

This is one correction beyond the plan's own gotcha list: the plan's gotcha
#2 focused on trick collection; `next_round()` needed its own explicit
scoping decision (drive it vs. don't), documented here rather than
discovered by accident.

## Step granularity and `state_json` emission

One `Step()` = one `ai_step()` call, plus whichever of the two synchronous
follow-ups (`finish_pass_animation`/`finish_trick_collect`) that same
`ai_step()` call requires to avoid stalling. Concretely, for one full round
of `game.gamestate` (round 1, passing not skipped): 56 `Step()` calls — 4
consumed by the passing phase (no schema tier; `select_pass()` isn't one of
`card_play`/`trick`/`round`), 52 producing `card_play` events (one per
`state.play_card()`, including each trick's 4th/triggering card), 13
producing `trick` events, and the last of those 13 *also* producing the
`round` event in the same `Step()` (13th trick's resolution and
`resolve_round()` happen inside the same Python call chain). A single
`Step()` can therefore emit more than one tier event — `GetLastStepEvents()`
returns all of them in emission order; `GetLastStateJson()` is a
convenience returning the last (most significant) one, or `""` for a
passing-phase sub-step.

`RunFromSteppedSource()` (`uppsrc/VisualStateModel/PipelineRunner.cpp`) was
read in full before deciding how to wire this up: its loop only calls
`Step()`/`ReadFrame()`/`ProcessSourceFrame()` — there is no hook for a
stepped source's own `state_json` at all, so routing through it today would
silently drop every event this task exists to produce. This is a real API
gap (worth closing in a future task), not a reason to call direct driving
"premature" — the demo drives `Step()`/`HasMoreSteps()`/`ReadFrame()`
directly instead, per the plan's own sanctioned fallback.

`trick_number` for the `trick` tier is caller-supplied (via
`VsmCardGameStateExport::ExportTrickState()`) and tracked *exactly* by
`VsmHeartsSource` itself (incremented once per `finish_trick_collect()`
call the driver makes) — this sidesteps the `card_play`-tier's own
known same-winner-twice-in-a-row undercount (documented above), since the
driver always knows precisely when a trick resolved, unlike the adapter's
external `last_trick_winner`-diffing heuristic. That heuristic limitation
did surface exactly as documented in a real run (round 1 had player 3 win
tricks 5 and 6 back to back; the `card_play` events during trick 6 show
`trick_number:5`, stuck) — confirming the known limitation is real and
unrelated to anything `VsmHeartsSource` does, and that `VsmHeartsSource`'s
own `trick`-tier counter is unaffected by it.

`ReadFrame()` reuses the same `Image` → `VsmImageBuffer` RGB conversion as
`reference/VisualStateWorkbench/JpegSequenceImporter.cpp`'s
`ImageToVsmBuffer()` (no separate general-purpose helper existed to import
instead).

## A confirmed, orthogonal `CardGameDocumentHost` finding (not fixed here)

Driving `ai_step()`/`refresh_ui()` manually more than once on a
`CardGameDocumentHost` trips its own internal `CheckExpectedSpriteCounts()`
self-check (`CardGamePlugin.cpp`), logging `"AssertionError: render sprite
count mismatch in hand_self: expected N, got 0"`-style messages once
`log_to_stdout` is on. Verified this is **not** specific to
`VsmHeartsSource` or to running two `CardGameDocumentHost` instances in one
process: reproduced identically by adding one extra manual `ai_step()` call
to 0068's own single-`ExecuteSync()` demo host. It does not throw, does not
corrupt `state` (a full round completed with mathematically correct scores
summing to 26), and does not affect `CaptureRecordFrame()`'s pixel output
(every `ReadFrame()` in a 56-step run returned a non-empty frame). It
appears to be a sprite-positioning bookkeeping issue in
`CardGamePlugin.cpp`'s `ApplySetCard()`/`CheckExpectedSpriteCounts()` path
that simply had never been exercised by repeated manual driving before this
task (0068's demo only ever calls `ExecuteSync()` once). Left alone — it's
squarely inside `ScriptIDE`'s own GUI/sprite-rendering internals, outside
this task's scope, and orthogonal to `state_json`/frame correctness; the
demo works around the *symptom* only, by setting
`CardGameDocumentHost::log_to_stdout = false` before driving (that static
flag, left on from 0068's own demo, would otherwise make every mismatch
re-log the *entire* accumulated game log, compounding into an unusably
large log — a separate, easy-to-fix-locally side effect of the same root
cause). Flagged here as a follow-up for whoever next touches
`CardGamePlugin.cpp`'s sprite pipeline or task 0073's OCR wiring.

## Demo / test

Extends `reference/CardGameStateAdapter/main.cpp`'s existing `GUI_APP_MAIN`
(same binary, no new package): after the existing `VsmCardGameStateExport`
checks, opens a second `CardGameDocumentHost` via `VsmHeartsSource`, drives
it with a `kMaxSteps = 200` hard cap (`ASSERT_`s loudly if exceeded without
reaching a `"tier":"round"` event — never silently hangs), and asserts the
final event's field set against the schema exactly.

Build & run:

```
bin/build --mainconfig 0 --release --jobs 12 CardGameStateAdapter
bin/CardGameStateAdapter
```

Sample output (real run, tail):

```
--- VsmHeartsSource: driving one full round ---
VsmHeartsSource opened: VsmHeartsSource:/home/sblo/Dev/ai-upp/uppsrc/ScriptIDE/reference/Hearts/game.gamestate (1024x702)
  [5] {"tier":"card_play","round_number":1,"phase":"PLAYING","turn":2,"trick_number":1,"leading_suit":"clubs","hearts_broken":false,"player":1,"card_played":"2C"}
  ...
  [56] {"tier":"trick","round_number":1,"trick_number":13,"trick_winner":2,"trick_points":1,"round_scores":[0,0,19,7]}
  [56] {"tier":"round","round_number":1,"round_scores":[0,0,19,7],"scores":[0,0,19,7],"moon_shooter":-1,"game_over":false}
VsmHeartsSource: 56 Step() calls, 52 card_play events, 13 trick events, 1 round event. Final round_json: {"tier":"round","round_number":1,"round_scores":[0,0,19,7],"scores":[0,0,19,7],"moon_shooter":-1,"game_over":false}
VsmHeartsSource one-round drive: OK.
```

## Non-goals / what this does not do

- Full-game (multi-round) driving — `next_round()` is deliberately never
  called (see table above).
- OCR wiring (task 0073).
- Cardinality/`expected_child_count` (task 0071).
- Self-consistency validation (task 0072).
- Fixing the `CardGameDocumentHost` sprite-count self-check finding above.
