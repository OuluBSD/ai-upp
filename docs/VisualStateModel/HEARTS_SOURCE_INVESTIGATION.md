# Investigation: Hearts As A Controlled/Synthetic VsmFrameSource

Status: **investigation only**. No frame source code was written. This
document is the output of task `plan/VisualStateModel/0055_hearts_controlled_source_investigation.md`.

---

## Addendum (2026-07-04): `game/CardGame` restored — decision confirmed, not overturned

`game/CardGame` and `game/CardGameTest` were restored on `master`
(commit `30028cbdf`, after this investigation was written). `Hearts.exe`
and `CardGameTest.exe` both build and pass their tests today. This removes
the investigation's stated reason for rejecting option (a) outright
("It does not build" — see §2 below), so the choice was re-examined rather
than assumed still valid. Two follow-up checks (task `0066`'s continuation)
confirm the original chosen seam (option c) is still the right near-term
target, on cost grounds, not on the build-failure grounds this doc
originally used:

1. **`CaptureRecordFrame()`'s offscreen path is real and already proven in
   production**, not a future risk: it renders to `SImageDraw`
   (`uppsrc/Draw/SImageDraw.cpp` — pure in-memory `ImageBuffer`, no X11/GL),
   confirmed working with **no display server** in this environment
   (`DISPLAY` was set but `xdpyinfo` fails; capture works anyway). ScriptIDE
   already ships a `--headless` mode (`uppsrc/ScriptIDE/Main.cpp`) built on
   exactly this path, and `SaveSnapshot()`
   (`CardGamePlugin.cpp:2720-2739`) exercises it end-to-end today.
   `game/Hearts::HeartsCtrl` has no equivalent capture path yet — the gap
   noted in §1 below still stands; building one for option (a) would mean
   replicating this pattern from scratch, not a blocked/unknown risk, but
   real net-new work.
2. **`ExecuteSync()` is genuinely synchronous** (`CardGamePlugin.cpp:2712-2718`
   → `CardGamePlugin::Execute()` → one `vm->Call()` to completion, no thread,
   no event loop) and **`paused_globals` is already a structured, directly
   walkable `VectorMap<PyValue, PyValue>`** mirroring the VM's globals dict
   (`CapturePausedDebugState()`, `CardGamePlugin.cpp:1691-1698`) — not a
   string blob needing parsing. A `paused_globals`/`GetSprites()` →
   `state_json` adapter (task `0068`) is estimated at roughly 200-300 lines
   of field-projection glue, not state reconstruction, because
   `hearts/logic.py::GameState`'s fields already match
   `CARD_GAME_STATE_SCHEMA.md`'s tiers almost one-to-one.
3. **Gap #1 (no step-driving mode, task `0067`) applies to both options
   equally** — `ExecuteSync()` has no built-in step mode today (AI delays
   are Python-level `set_timeout()` calls that only fire with a running
   event loop, which `ExecuteSync()` doesn't have), and `game/Hearts`
   has no step hook either. Choosing (a) over (c) would not avoid this work.

**Decision: keep option (c) as originally chosen for `0066`-`0074`.** The
underlying reason changes — no longer "(a) doesn't build" but "(c) has a
proven, already-headless capture path and directly-walkable state today,
while (a) requires building both a capture path and a state-export API from
nothing, on top of the same step-driving gap either option needs." Do not
re-defer option (a) to an unscheduled someday, though: once `0066`-`0074`
ship and prove the `VsmHeartsSource`/schema/OCR-pipeline loop end-to-end
against the Python engine, revisit driving `game/Hearts::HeartsCtrl`
directly as an explicit next phase — its long-term value (validating the
actual first-party C++ engine users run, not a parallel Python
reimplementation) is real and unaffected by this cost comparison. That
phase is engineering work now, not blocked-on-a-missing-package risk.

---

## Sources actually read

- `game/Hearts/Hearts.upp`, `Hearts.h`, `HeartsCtrl.h`, `HeartsCtrl.cpp`, `main.cpp`
- `uppsrc/ScriptIDE/CardGamePlugin.h` (declarations: `CardGamePluginGUI`, `CardGameDocumentHost`, `CardGameOverlay`, `CardSpriteCtrl`)
- `uppsrc/ScriptIDE/CardGamePlugin.cpp` (`Load`, `Layout`, `DumpScene`, `ExportStandalonePackage`, VM thread plumbing)
- `uppsrc/ScriptCommon/CardGamePlugin.h`/`.cpp` (the actual `cardgame_view` / `ocr_verify` / `strategy_bridge` Python module bindings, and `CardGameSprite`, `IHeartsView` in `PluginInterfaces.h`)
- `uppsrc/ScriptIDE/reference/Hearts/`: `game.gamestate`, `visual_ci.gamestate`, `table.form`, `main.py`, `hearts/logic.py`, `hearts/ai.py`, `ci_verify.py`, `test_ocr.py`, `test_logic.py`, `test_import.py`
- `uppsrc/VisualStateModel/FrameSource.h`, `GroundTruth.h`, `Annotation.h`, `Types.h`, `OcrLayer.h`, `SessionDiff.h`, `CaptureSink.h`, `TestFixtures.h`
- `docs/VisualStateModel/GROUND_TRUTH.md` (the `.vsm.json` event schema)
- `plan/VisualStateModel/ai-upp_handoff_2026-07-02/DESIGN_PRINCIPLES.md`

Also confirmed empirically: `bin/build.exe -m 7 -j12 Hearts` currently fails
with `Missing package(s): CardGame` — see "A build-level surprise" below.

---

## 0. A build-level surprise that reframes the whole investigation

`game/Hearts/Hearts.upp` declares `uses CtrlLib, CardGame, plugin/png;` and
`HeartsCtrl.h`/`.cpp` use a `GameState state;` member (from `CardGame/CardGame.h`).
**No `CardGame` package exists anywhere in this checkout** (`uppsrc/`, `game/`,
or elsewhere) — confirmed by `find` returning nothing and by
`bin/build.exe -m 7 -j12 Hearts` failing immediately with
`Missing package(s): CardGame`. `game/Hearts` cannot be built, embedded, or
subprocessed today; it is a UI shell (`HeartsCtrl`, `main.cpp`) sitting on top
of a game-logic package that isn't in the tree.

Separately, `uppsrc/ScriptIDE/reference/Hearts/` is **not** a config file that
drives `game/Hearts`. It is a **second, independent implementation** of Hearts
rules, written in Python (`hearts/logic.py::GameState`, `hearts/ai.py`),
executed by ScriptIDE's embedded `PyVM` inside `CardGameDocumentHost`
(`uppsrc/ScriptIDE/CardGamePlugin.h/.cpp`). The two implementations are
behaviorally close (same field names — `phase`, `trick`, `passed_cards`,
`round_scores`, `hearts_broken`, `trick_pending`, `pending_trick_winner`,
`last_round_moon_shooter`, etc. — strongly suggesting the Python version was
ported from the C++ `CardGame::GameState` that `HeartsCtrl` uses), but they
are two separate code paths, not one game driven two ways.

This matters directly for requirement 2: it means options (a) and (b) are
not just "different plumbing to the same game," they are **different games**
(one currently unbuildable, one real and running).

---

## 1. Concept mapping

| VSM concept | Closest Hearts/ScriptIDE equivalent | Divergence |
|---|---|---|
| `VsmFrameSource` (`FrameSource.h`) — pull-based `Open/Close/IsReady/ReadFrame` | `CardGameDocumentHost` (`uppsrc/ScriptIDE/CardGamePlugin.h`), which is already an `IVideoRenderSource` with `CaptureRecordFrame()`, `GetRecordFrameSize()`, and a synchronous `ExecuteSync()` entry point (no GUI event loop needed) | `VsmFrameSource` is frame-only and stateless about "ground truth"; `CardGameDocumentHost` bundles rendering, VM execution, and state export into one class. A `VsmHeartsSource` would need to split that bundle back into a frame-pull side and a state-pull side. `game/Hearts::HeartsCtrl` has **no** equivalent hook at all — no capture/export API, just `Ctrl::Paint()`. |
| `VsmGroundTruthSession` / `VsmDivergence` (`GroundTruth.h`, `Types.h`) — ordered `frames/changes/regions/ocr_results/state_snapshots/divergences`, `VsmDivergence{frame, severity, message, region_id, expected_json, observed_json}` | The Python `GameState` object itself (`hearts/logic.py`) — `scores`, `round_scores`, `last_trick_winner`, `last_trick_points`, `last_round_scores`, `last_round_moon_shooter`, `game_over` are exactly a per-event state snapshot. The `.gamestate` file's `metadata.ocr_expected` dict is a *much* weaker, static analogue. | `ocr_expected` is authored once for the initial dealt frame; it has no timeline, no frame index, no notion of "after trick 6" vs "after trick 1". `VsmDivergence` assumes an independent observed-vs-expected pair; nothing in the Hearts stack today produces the "observed" half — there is no vision/OCR pipeline reading the rendered Hearts table back into structured data. A Hearts source can populate `expected_json` for free; `observed_json` still has to come from VSM's own pipeline running against captured frames. |
| `VsmAnnotationLayer` / `VsmRegionAnnotation` (`Annotation.h`) — `id, parent_id, x/y/w/h, binding{FIXED/HORIZONTAL/VERTICAL/REFERENCE}, anchors, hotspots` | `table.form`'s `FormObject`s (`Variable`, `UserClass` ∈ {CONTAINER, TRICK, HAND, LABEL, BUTTON}, `Anchor` ∈ {TOP_LEFT, BOTTOM_CENTER, CENTER_LEFT, ...}, `Parent`), materialized at runtime as `CardGameDocumentHost::form_items` (`id/anchor/user_class/design_rect`) and dumped verbatim by `DumpScene()` as `form_item <id> class=<UserClass> anchor=<Anchor> rect=<x,y w×h>` — this is the `tmp/hearts-scene-dump*.txt` format referenced in the task context. | Genuinely close conceptually (named zone + anchor + rect + parent), but `.form`'s `Anchor` is a **fixed enum tied to the U++ layout engine** (9 literal strings), not `VsmRegionBinding`'s general `{type, reference_id}` model — converting one to the other needs an explicit translation table that doesn't exist. Also: `HAND`/`TRICK` zones hold a *variable count* of individually-positioned card sprites (0–13), and `VsmRegionAnnotation` models one fixed rect per id with no cardinality concept (see gap #3 below). `game/Hearts::HeartsCtrl` has no `.form` equivalent at all — its zone geometry is hardcoded arithmetic inside `Paint()` (`GetHandCenter`/`GetTrickCenter`). |
| OCR observation layer (`VsmOcrRule`/`VsmOcrLayer`, `Types.h`/`OcrLayer.h`) — `VsmOcrRule{rule_id, annotation_id, pipeline_id, expectation, confidence_threshold}`, `VsmOcrExecutor::RunRequest/Compare` against a pluggable `VsmOcrEngine` | `test_ocr.py`'s `ocr_verify.read_zone(zone_id)` / `ocr_verify.compare(expected_dict)` — conceptually a near-exact match (read text from a named zone, compare to expectation, get back per-zone match/hint). | **This is a stub, not a working pipeline.** Read `uppsrc/ScriptCommon/CardGamePlugin.cpp` lines 6–17 and 700–791: the real C++ binding for `ocr_verify` only registers `set_models`, `read_cards`, `last_error`, `compare` — and `ov_compare()` unconditionally returns `{"pass":true,"signal":"pass","zones":[]}`. There is **no `capture_frame` and no `read_zone` function registered anywhere**, in headless mode *or* in GUI-hosted mode (the `view`-truthy branch only registers `cardgame_view` and `strategy_bridge`, not `ocr_verify` at all). `test_ocr.py`/`ci_verify.py` calling `ocr_verify.capture_frame()` or `ocr_verify.read_zone(...)` would fail today if actually executed through this binding. VSM's own `VsmOcrExecutor`/`VsmOcrEngine`/`VsmFakeOcrEngine` is the only OCR abstraction in the whole tree that is real and working end-to-end. |

---

## 2. Chosen integration shape

**Chosen: (c), a variant closer to (b) than to (a) — wrap `CardGameDocumentHost`
(ScriptIDE/ScriptCommon), not `game/Hearts`, and take ground truth from its
already-real state-export surface, while routing actual OCR through VSM's own
`VsmOcrExecutor` rather than the stubbed `ocr_verify` module.**

### Against (a): drive `game/Hearts` directly

Rejected as the near-term shape, for concrete reasons found during this
investigation, not by default:

1. **It does not build.** `Missing package(s): CardGame` is not a hypothetical
   risk — it is the actual output of `bin/build.exe -m 7 -j12 Hearts` right
   now. Any `VsmHeartsSource` that embeds or subprocesses `game/Hearts` first
   requires restoring/vendoring the `CardGame` package, which is a
   substantial, unrelated prerequisite outside this task's (and arguably this
   plan area's) scope.
2. **Zero export substrate exists.** `HeartsCtrl` has no `.form`, no
   `DumpScene()`, no `GetSprites()`, no `CaptureRecordFrame()`, no zone
   concept — it is a single `Ctrl::Paint()` with inline pixel math. Every
   piece of "ground truth for free" infrastructure that requirement 2 hopes
   to reuse would have to be built from nothing on this path, which defeats
   the "for free" premise of a controlled source.
3. The upside of (a) — first-party fidelity to the "real" Hearts widget users
   would actually run — is real, but it's a future-phase concern once
   `CardGame` is restored, not a reason to block this exercise now.

### Against (b) as literally stated: reuse the Python `ocr_verify`/`cardgame_view` harness wholesale

Rejected in its literal form, though its instinct (reuse what's real) is
correct:

1. The **zone-capture and expected-value-dictionary half of (b) is real and
   reusable**: `.gamestate`/`.form`, `CardGameDocumentHost::GetSprites()`,
   `DumpScene()`, `SaveSnapshot()`, and `CaptureRecordFrame()` are all working
   C++ code today (proven by the existing `render.mp4` recording in
   `uppsrc/ScriptIDE/reference/Hearts/`).
2. The **OCR half of (b) is not reusable as-is**, because `ocr_verify` is a
   stub (see mapping table above). A "thin C++ adapter" that wraps
   `ocr_verify.compare()` today would just be wrapping a function that always
   returns `pass=true` with zero zones — an adapter around a no-op, which
   would make ground truth trivially "pass" and defeat its entire purpose
   (catching divergence). Fixing `ocr_verify` first would mean building a
   second working OCR pipeline in C++/Python glue, duplicating work that
   `VsmOcrExecutor`/`VsmOcrEngine` already does — which the task's own
   non-goals explicitly warn against ("do not duplicate the zone-matching/
   OCR-comparison logic").

### The chosen seam

- Build the eventual `VsmHeartsSource` against **`CardGameDocumentHost`**
  (`uppsrc/ScriptIDE/CardGamePlugin.h`), using its `ExecuteSync()` +
  `CaptureRecordFrame()` for the `VsmFrameSource::ReadFrame()` half, and its
  `GetSprites()` / `DumpScene()` / the underlying Python `GameState` (via
  `PopulateDebugState`/`paused_globals`, which already captures VM globals for
  the debugger — see `CapturePausedDebugState()`) for the ground-truth half.
  This reuses `uppsrc/ScriptIDE/reference/Hearts/` (`game.gamestate`,
  `table.form`, `hearts/logic.py`) as the actual scenario content, matching
  what requirement 2(b) intended, without inheriting the OCR stub.
- Route text verification through **VSM's own `VsmOcrExecutor`/`VsmOcrEngine`**
  running against frames captured via `CaptureRecordFrame()`, instead of
  through `ocr_verify`. This means Hearts contributes ground truth and
  rendered pixels; VSM contributes the (already real) OCR comparison. Nobody
  writes a third OCR implementation.
- This also lines up with `docs/VisualStateModel/GROUND_TRUTH.md`, which
  *already* reserves `"source_type": "game_export"` in the session schema —
  the seam this document proposes is exactly the producer of `game_export`
  sessions the schema was designed to accept, and `state_snapshot` events are
  exactly the shape `GameState`'s per-trick/per-round fields need.
- `game/Hearts` (the C++ widget) is explicitly deferred, not abandoned: once
  `CardGame` is restored/vendored (separate work, outside VSM), option (a)
  becomes attractive again as a second, first-party-fidelity Hearts source
  sharing the same `VsmGroundTruthSession` shape defined here.

---

## 3. What "one full round" means for Hearts

Confirmed from `hearts/logic.py::GameState` (the only complete, readable
implementation of the rules in-tree — `game/Hearts`'s own `GameState` lives in
the missing `CardGame` package, but `HeartsCtrl.cpp` calls the identically
named/shaped methods, `state.Deal()`, `state.SelectPass()`, `state.PlayCard()`,
`state.ResolveTrick()`, `state.BeginNextRound()`, confirming the same round
boundary applies to both implementations):

1. **`deal()`** — `round_number` increments; 52-card deck shuffled and dealt
   13 cards to each of 4 players. `phase` becomes `PASSING` unless
   `round_number % 4 == 0`, in which case pass is skipped and `phase` becomes
   `PLAYING` directly (`start_play_phase()` finds whoever holds 2♣ and sets
   `turn`).
2. **Passing phase (`PASSING`, skipped on hold rounds)** — each of the 4
   players calls `select_pass(player_index, 3 cards)`; once all 4 have
   passed, `execute_pass()` redistributes cards by `round_number % 4`
   direction (1=left, 2=right, 3=across) and flips `phase` to `PLAYING`.
3. **Playing phase (`PLAYING`)** — exactly 13 tricks. Each trick is 4
   `play_card()` calls (one per player in turn order, following-suit/hearts-
   broken/first-trick rules enforced by `validate_play()`); on the 4th card,
   `trick_pending=True` + `pending_trick_winner/points` are computed by
   `get_trick_result()`; `resolve_trick()` commits `round_scores[winner]`,
   resets `trick`/`leading_suit`, and sets `turn = winner`.
4. **Round end** — after the 13th trick (`all(len(hand)==0)`),
   `resolve_trick()` calls `resolve_round()`: checks shoot-the-moon
   (`round_scores[i] == 26` ⇒ that player gets 0, everyone else gets 26),
   commits `round_scores` into `scores`, and sets `phase` to `GAME_OVER`
   (any `scores[i] >= 100`) or `ROUND_END` otherwise.
5. **`begin_next_round()`** calls `deal()` again unless `game_over`.

**One full round = steps 1–4** (deal through `resolve_round()`), i.e. exactly
what `docs/VisualStateModel/GROUND_TRUTH.md` would call one
`replay_checkpoint`-bounded segment. For VSM's comparison tooling
(`VsmGroundTruthComparison`, `VsmSessionDiff`), ground truth for one round
needs, at minimum:

- **Frame/event granularity**: one `state_snapshot`-equivalent per card play
  (52 cards ⇒ up to 52 events, fewer if a round is a hold-round with fewer
  passes) carrying `{phase, turn, trick, leading_suit, hearts_broken}`.
- **Trick granularity**: one event per resolved trick (13 per round) carrying
  `{trick_winner, trick_points, round_scores[4]}` — this is the natural unit
  for `VsmDivergence` (each trick resolution is a checkpoint where "expected
  winner" vs "observed winner" is meaningful and cheap to assert).
- **Round granularity**: one event at `resolve_round()` carrying
  `{round_scores[4], scores[4], moon_shooter, game_over}` — the natural unit
  for a `replay_checkpoint`.

This 3-tier structure (card-play / trick / round) maps directly onto VSM's
existing `frame` / `change`+`region` / `state_snapshot`+`divergence` layering,
which is a good sign for the chosen seam — nothing about Hearts' rules forces
a shape VSM doesn't already have room for.

---

## 4. API gaps surfaced

Concrete, named — not generic "some friction exists":

1. **`VsmFrameSource` has no notion of driving a semi-live, event-stepped
   source.** `Open/Close/IsReady/ReadFrame` (`FrameSource.h:13-22`) assumes
   frames simply arrive; `CardGameDocumentHost`'s game loop is asynchronous
   (`VmThreadMain`, `QueueVmTask`, `SetTimeCallback`-driven AI steps —
   `uppsrc/ScriptIDE/CardGamePlugin.cpp`). A `VsmHeartsSource` needs a "step
   the game forward by one logical unit (one card play / one AI step), then
   capture" mode that `VsmFrameSource` has no hook for today. `ExecuteSync()`
   exists for headless synchronous execution but is host-specific, not part
   of the `VsmFrameSource` contract.
2. **`VsmGroundTruthComparison::Compare` matches by raw string equality of
   `expected_json`** (`GroundTruth.h:91-98`: "matching key is `expected_json`
   (string equality) within ±5 frames"). `VsmModelStateRef::state_json` is
   documented only as "serialized JSON object" (`Types.h:106-112`) with no
   canonical key ordering/whitespace guarantee. Two structurally-identical
   Hearts state dumps that differ only in JSON serialization formatting would
   silently fail to match — there is no deep/canonical JSON diff helper
   anywhere in `VisualStateModel/`.
3. **No cardinality concept in `VsmRegionAnnotation`.** Hearts' `HAND`/`TRICK`
   zones (`table.form`) hold a *variable number* of individually positioned
   card sprites (0–13, changing every play). `VsmRegionAnnotation`
   (`Annotation.h:34-47`) models exactly one fixed rect per `id`.
   `CardGameDocumentHost` already has the concept it's missing —
   `SetExpectedSpriteCount(zone_id, count)` / `expected_sprite_counts` — but
   nothing in VSM's annotation or ground-truth layer carries a per-frame
   "expected child count for this zone" field.
4. **`VsmOcrRule` expects one static `expectation` per rule**
   (`Types.h:43-54`, `VsmTextExpectation{mode, expected_text}`). Hearts' own
   `.gamestate` `ocr_expected` values (e.g. `"You  T:0  R:+0  C:13"`) are only
   valid for the just-dealt frame — the card count `C:` changes every trick.
   There is no facility anywhere (VSM or the Hearts reference harness) to
   regenerate an `expected_text` dynamically from a live model snapshot mid-
   round; today's static dict only covers frame 0 of a round.
5. **No JSON adapter exists from `CardGameDocumentHost`'s state shapes to
   `VsmSession`/`VsmRegionNode`/`VsmFrameRef`.** Neither `DumpScene()`'s plain-
   text dump nor `GetSprites()`'s `ArrayMap<String,CardGameSprite>` has any
   serializer targeting VSM's JSON schema. This is 100% new code regardless
   of which shape is chosen — worth calling out because it's easy to
   underestimate given how much *looks* reusable at the C++ class level.
6. **No self-consistency check for a controlled source.**
   `VsmGroundTruthComparison::Compare` (`GroundTruth.h:91-98`) is built to
   compare an authored `VsmGroundTruthSession` against divergences "observed
   by the pipeline" — i.e. it assumes ground truth is trustworthy and only
   the vision pipeline can be wrong. A controlled source's whole point is
   that ground truth is generated mechanically from a running game; nothing
   validates that the *generator* itself is internally consistent (e.g. do
   the emitted trick-winner events actually match the emitted round-score
   deltas?) before that ground truth is fed into a pipeline test.

---

## 5. Follow-on task list (numbered from 0066)

`plan/VisualStateModel/0056` through `0065` are already taken by concurrent
work as of this investigation, so new plan files for this exercise should
start at `0066`. Ordered so each task's prerequisites are already done by the
time it starts:

- **0066** — Define and document the `VsmSession` `state_json` schema for
  card-game sources (canonical field set for card-play/trick/round
  granularity from Section 3), plus a canonical-JSON comparison helper to
  close gap #2 (replace `VsmGroundTruthComparison`'s raw string-equality
  match with a normalized structural compare).
- **0067** — Add a "step" driving mode to `VsmFrameSource` (or a sibling
  interface) for semi-live, event-stepped sources, closing gap #1; needed
  before any Hearts-driving source can implement `ReadFrame()` meaningfully.
- **0068** — Write the `CardGameDocumentHost` → `VsmSession` adapter: convert
  `GetSprites()`/`DumpScene()`/captured Python `GameState` globals (via
  `CapturePausedDebugState`/`paused_globals`) into `state_snapshot`/`frame`/
  `region` events per the 0066 schema. This is the "thin C++ adapter" from
  requirement 2(b), aimed at the real export surface instead of `ocr_verify`.
- **0069** — Implement `VsmHeartsSource : VsmFrameSource` wrapping
  `CardGameDocumentHost` loaded with `uppsrc/ScriptIDE/reference/Hearts/game.gamestate`,
  using the 0067 step-driving mode and the 0068 adapter; drive exactly one
  full round (Section 3) per capture run via `ExecuteSync()` +
  `CaptureRecordFrame()`.
- **0070** — Add per-frame dynamic `VsmOcrRule.expected_text` generation from
  a live `state_json` snapshot (closes gap #4), so OCR expectations track
  score/card-count changes across a round instead of only matching the
  initial deal.
- **0071** — Add a zone-cardinality field (`expected_child_count` or similar)
  to `VsmRegionAnnotation`/ground-truth events (closes gap #3), sourced from
  `CardGameDocumentHost::expected_sprite_counts` for `HAND`/`TRICK` zones.
- **0072** — Add a ground-truth self-consistency validator (closes gap #6):
  given a `VsmHeartsSource`-produced session, assert trick-winner events
  reconcile with round-score deltas and card-count invariants (mirroring the
  `assert_state_invariants`/`assert_render_invariants` checks already present
  in `uppsrc/ScriptIDE/reference/Hearts/main.py`, but as a reusable VSM-side
  tool rather than Python assertions specific to this one game).
- **0073** — Wire `VsmHeartsSource` output through VSM's real
  `VsmOcrExecutor`/`VsmOcrEngine` against captured frames (not `ocr_verify`),
  producing genuine `VsmDivergence` records (`observed_json`) to compare
  against the Hearts-generated `expected_json`, closing the loop described in
  Section 2's chosen seam.
- **0074** (deferred, blocked on out-of-scope work) — Once the `CardGame`
  package dependency is restored/vendored and `game/Hearts` builds again,
  revisit option (a): a second, first-party-fidelity `VsmFrameSource`
  driving `game/Hearts::HeartsCtrl` directly, reusing the same
  `VsmSession`/`state_json` schema defined in 0066 so both Hearts sources are
  interchangeable ground-truth producers.
