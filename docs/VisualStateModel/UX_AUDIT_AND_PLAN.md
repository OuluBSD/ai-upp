# VisualStateWorkbench UX Audit — Decisions and Improvement Plan

This document closes out `plan/VisualStateModel/0056_ui_ux_audit_and_improvement_plan.md`.
It makes the explicit judgment calls the audit (`plan/VisualStateModel/diagrams/`)
deliberately left open, and produces the ordered `0057`+ task list that
implements them.

Inputs (read in full before writing this): `diagrams/workflow_catalogue.md`,
`diagrams/session_state_machine.puml`, `diagrams/current_state.txt`,
`diagrams/workbench_components.puml`, `diagrams/user_journey_current.puml`,
`diagrams/ux_gaps.txt`, `ai-upp_handoff_2026-07-02/DESIGN_PRINCIPLES.md`.

---

## 1. Severity Summary decisions (`workflow_catalogue.md`)

### Correctness bugs

| Item | Decision | Reasoning | Task |
|---|---|---|---|
| **BUG-A** (3.1/3.2/4.2/4.3: toolbar Step/Run All and region-click/list-select always act on the sample session, never the opened/imported one) | **Fix now, first** | Data-correctness bug: a user who opened their own session sees no effect from Step/Run All, and region clicks silently look up the wrong session's region list. This is not cosmetic — it makes core controls non-functional against real data. Must be fixed before any panel/menu cosmetics are built on top of "which session is active." | `0057` |
| **BUG-B** (3.3: "Reset" silently reloads the sample session over an opened/imported one, no confirmation) | **Fix now, first** | Data-loss risk: a user can lose an imported session with one accidental click and zero warning. Same root cause as BUG-A (two independent session objects sharing one `session_store_`), so fixed in the same task. | `0057` |
| 6.1/7.1 rule ID collision after remove+add (`OcrRulePanel`/`TemplateRulePanel` assign IDs from post-add row count, not a monotonic counter) | **Fix now** | Real bug (two different rules can end up self-identifying with the same ID string after a remove+add sequence), but low blast radius (only affects users who delete-then-add rules) and independent of session identity — does not need to block or follow `0057`. | `0059` |
| 1.7 `Clear Pipeline Cache` hardcodes `0` in its "entries cleared" log line | **Fix now** | Trivial, one-line, purely informational — bundled with the other tiny logging-correctness bugs below rather than given its own task. | `0059` |
| 3.5–3.8 overlay-toggle log line always reports `ShowRegions()`'s value regardless of which overlay was actually toggled | **Fix now** | Same class as 1.7: trivial, log-only, no functional effect, but still wrong and cheap to fix. Bundled with 1.7 and 6.1/7.1 into one small task so it doesn't get lost as "not worth a task." | `0059` |

### Misleading affordances

| Item | Decision | Reasoning | Task |
|---|---|---|---|
| 6.3 OCR "Run (Fake)" always passes (seeded with its own expected answer) | **Fix now** | A validation affordance that cannot fail is worse than no affordance — it actively teaches the user false confidence in a rule. Fixing requires wiring the panel to the real active session's current frame, which only becomes well-defined after `0057` unifies "the active session." | `0060` |
| 8.3 Pipeline "Run" only ever runs against a synthetic 32×32 gradient, never the real loaded frame | **Fix now** | Same underlying gap as 6.3 (panel isn't wired to the real frame) — fixed together in one task rather than twice. | `0060` |
| 5.3 ModelStatePanel "Run Sample Events" is a fixed, non-configurable 2-event canned scenario | **Fix later** | Lower priority: unlike 6.3/8.3 this button is honestly a smoke-test convenience (it doesn't claim to validate the user's own rules), and making it configurable is a distinct, larger feature (an event-authoring UI) rather than a one-line real-frame wiring fix. No task number assigned yet; revisit after `0060` ships and panels are wired to real data, since a configurable-events feature would naturally build on the same "operate on the real active session" foundation. |

### Consistency gaps across sibling panels

| Item | Decision | Reasoning | Task |
|---|---|---|---|
| 7.x `TemplateRulePanel` has no Run button, unlike sibling `OcrRulePanel`/`PipelineEditorPanel` | **Fix now** | Folded into the same real-frame-wiring task as 6.3/8.3 — adding a Run button to `TemplateRulePanel` at the same time it's made to test against the real frame closes both the missing-affordance gap and the misleading-affordance gap in one pass. | `0060` |
| 9.1 `AnnotationEditorPanel` blank-state defaults to 100×40 instead of the "—" convention used by `RegionPropsPanel`/`SessionInfoPanel` | **Fix now** | Cheap, self-contained UI-state fix. Bundled with the other `AnnotationEditorPanel` fix below since both touch the same file/class. | `0061` |

### Silent/implicit behavior

| Item | Decision | Reasoning | Task |
|---|---|---|---|
| 9.4 "Apply" firing a full validate+persist cycle even on a no-op (`row<0`) selection; annotation edits auto-`Save()` with no explicit save/discard/undo | **Fix now** | Not data-loss-critical the way BUG-B is (it's autosaving, not discarding), but a typo can silently overwrite the on-disk annotation file with no way to tell "did that just save," and Apply's no-op-yet-logs-success behavior is actively misleading. Fixed alongside 9.1 in the same panel. | `0061` |
| 1.1/1.3/1.4/1.5/1.6 most failure branches (session open/import/pipeline-run failures) log to the Debug tab only, with no dialog/status feedback | **Fix now** | Real usability defect — a user not watching the Debug tab gets no indication anything went wrong. Sequenced right after `0057` because it touches the same `OpenSessionPath()`/`RunFromSource()` call sites `0057` is unifying; doing it after avoids reworking error-surfacing code against a session model that's about to change underneath it. | `0058` |

**Net effect on sequencing:** `0057` (session identity / BUG-A / BUG-B) is
the first task in the follow-on list, exactly as required — every other
correctness task that touches the same call sites (`0058`, `0060`) is
sequenced after it.

---

## 2. `ux_gaps.txt` gap-by-gap decisions

| Gap | Decision | Reasoning | Task |
|---|---|---|---|
| **#1** 13 of 14 CLI tools take zero arguments | **Fix now** | Blocks all scripting/batch use of the feature line today. Convention decided in §3 below; rollout is `0062`. | `0062` |
| **#2** No shared CLI argument convention | **Fix now** | Decided in §3 below — a single convention, not "each tool decides for itself." | `0062` (same task defines and applies it) |
| **#3** `ReplayTimelinePanel` has no real playback | **Already tasked** (`0053`) | Do not duplicate. **Revision needed:** `0053`'s Step/Run All/Reset controls are exactly the controls implicated in BUG-A/BUG-B — as written, `0053` would build real playback (timer, scrub bar, FPS) on top of `replay_` (session A) only, the same object BUG-A shows is disconnected from whatever session is actually on screen. `0053` must be implemented **after** `0057` lands, and its playback timer must drive whichever session is active post-`0057`, not `replay_` specifically. This document does not edit `0053` itself (out of this task's scope), but the ordering requirement is recorded here and in `0057`'s Context section so whoever picks up `0053` sees it. |
| **#4** Session-loading entry points flat/undifferentiated | **Fix now** | Concrete restructuring proposed in §4. | `0063` |
| **#5** No first-run/empty-state guidance | **Fix now** | Concrete content proposed in §4. | `0064` |
| **#6** Developer-facing wording | **Not a defect, decided explicitly** | See §5 — the tool's actual audience is engineers working on VisualStateModel itself, not naive end users. | none |
| **#7** Five equally-weighted LEFT dock panels | **Fix now** | Concrete primary/secondary regrouping proposed in §4. | `0065` |
| **#8** Annotation Save has no destination picker | **Already tasked** (`0050`) | Do not duplicate. **Revision check:** `0050` adds `File → Import/Export Annotations…`, writing/loading through whatever `annotation_path_` the active session last set (workflow_catalogue.md row 9.2 notes this is "A/B, whichever session set it last"). Once `0057` unifies session identity, "whichever session set it last" collapses cleanly into "the one active session," so **`0050`'s scope does not need to change** — but its implementation should be written against (or land after) the post-`0057` single-session model so "current session" is unambiguous when the export dialog opens. No edits made to `0050` itself; noted here as guidance. |

---

## 3. CLI argument convention (gap #1/#2)

Checked against the one tool that already parses arguments,
`reference/VisualStateReplayReport/main.cpp`:

```cpp
const Vector<String>& args = CommandLine();
bool write_html = false;
String out_dir = AppendFileName(GetTempPath(), "vsm_report");
for(const String& arg : args) {
    if(arg == "--html") write_html = true;
    else out_dir = arg;                    // positional
}
```

This already establishes the shape to standardize on: a simple left-to-right
scan where recognized `--flag` tokens set options, and anything else is a
**positional path argument**. The chosen convention, to be applied
identically across every retrofitted tool:

1. **Positional argument = primary path.** `tool.exe [--flags...] [<path> [<path2>]]`.
   The first non-`--` argument overwrites the tool's primary input path
   (a session directory for most tools). No `--session <dir>` flag — the
   existing precedent already treats the bare path as positional
   (`out_dir` in `ReplayReport`), so a new flag would be an inconsistent
   second convention for the same kind of value.
2. **A second positional argument** is allowed only for tools that
   genuinely need two distinct paths in a fixed, documented order:
   `VisualStateSessionDiff <session_a> <session_b>`,
   `VisualStateGroundTruthInit <session_dir> <output_template_path>`.
   No tool needs a third positional argument.
3. **Flags stay flags**, exactly as `--html` does today: boolean
   `--name` tokens, no `=` syntax, no short forms. New tools may add their
   own flags (e.g. a future `--json` on a tool that currently only prints
   to `Cout()`), but the path is never expressed as a flag.
4. **No positional argument supplied → unchanged existing behavior.**
   Every tool keeps building its synthetic self-check session and running
   its current demo/self-test exactly as it does today. This is strictly
   additive: existing zero-argument invocations (CI smoke tests, `bin/build.exe`
   test runs) keep working byte-for-byte the same; the tool only switches to
   processing a real path when one is actually given.
5. **`--help` prints one line of usage** (e.g.
   `VisualStateSessionValidate [--help] [<session_dir>]`) for every
   retrofitted tool, since none of them currently document their own
   arguments (including the pre-existing `--html` on `ReplayReport`, which
   this rollout should also give a `--help` line while touching that file).

**Scope of the rollout — 11 of the 12 tools, not 12:** `VisualStateModelTest`
is excluded. It is a pure in-process unit-test suite
(`reference/VisualStateModelTest/main.cpp` — round-trips types, builds
in-memory fixtures, asserts) with no notion of "a session to point it at";
retrofitting it with a path argument would be inventing a feature it has no
use for, not closing a real gap. The other 11
(`VisualStateEndToEndSample`, `VisualStateRecordSession`,
`VisualStateImportSequence`, `VisualStateSessionValidate`,
`VisualStateBatchReport`, `VisualStateSessionDiff`,
`VisualStateGroundTruthInit`, `VisualStateAnnotationValidate`,
`VisualStateCacheStats`, `VisualStateRegionDump`, `VisualStateMjpegSource`)
all currently build a synthetic session/layer/rule set specifically to
exercise real headless functionality (`VsmSessionValidator`,
`VsmBatchDivergenceReport`, etc.) that already accepts a real directory —
they just never expose that through `argv`. This is wiring existing
headless surface to real CLI input, not inventing new headless API, per the
task's non-goals.

This convention is concrete enough that `0062` (and any later CLI task) can
implement it without re-deciding the shape.

---

## 4. Structural changes (gap #4/#5/#7)

### Gap #4 — group the session-loading entry points

Current: four flat, equal-weight `File` menu items —
"Open Session…", "Import Image Sequence…", "Load Sample Session",
"Load E2E Sample Session" — with no indication of when to use which.

Proposed: collapse these into a single **"File → Open/Import Session…"**
action that opens one dialog with an explicit source-type chooser
(radio/dropdown): *Existing session directory* / *Image sequence
(.vsm/.jpg/.png)* / *Built-in sample* / *Built-in E2E sample*. Selecting a
type reveals only the controls relevant to it (a directory picker for the
first two, nothing for the two sample options besides a "Load" button).
This keeps every current code path (`OpenSessionPath`, `RunVsmImport`,
`RunJpegImport`, `LoadSampleSession`) but gives them one discoverable entry
point with the source distinction made explicit in the UI instead of
inferred from which of four menu items a new user happens to click. Sample
data particularly needs to stop looking equal-weight with "open my real
session" — the dialog's chooser makes the built-in-sample options visibly a
distinct, labeled category ("Built-in sample data") rather than two more
File-menu bullets indistinguishable from the real-session actions.

Sequenced after `0057` (task `0063`) so the dialog is built against the
already-unified single active-session concept, not against the soon-to-change
A/B split.

### Gap #5 — first-run / empty-state guidance

Proposed first-run content for the empty Frame tab (shown only when
`DockInit()` finds no prior session state and no session has been opened yet
this run):

> **No session loaded.**
> Use **File → Open/Import Session…** to open a recorded session, import an
> image sequence, or load the built-in sample data to explore the workbench.

Plain-text placeholder centered in the Frame tab's canvas area (same
control, conditional content — not a new modal or wizard). This is
deliberately minimal: one sentence, one pointer to the (now-single, per gap
#4) entry point, no separate "Getting Started" panel/dock, matching the
tool's engineering-tool tone (see §5) rather than adding an onboarding flow
this tool's actual audience doesn't need. Task `0064`.

### Gap #7 — LEFT dock panel hierarchy

Current: `AnnotationEditorPanel`, `PipelineEditorPanel`, `TemplateRulePanel`,
`OcrRulePanel`, `ModelStatePanel` all `DockLeft()` in sequence, five
equal-weight stacked panels.

Proposed two-tier grouping, reflecting the actual usage order implied by
`DESIGN_PRINCIPLES.md`'s layer list (Annotation Layer before Fine Rule Layer
before Application Model Runtime):

- **Primary (visible by default, top of the LEFT stack):**
  `AnnotationEditorPanel` (defining what to look at is the first thing a new
  session needs) and `ModelStatePanel` (where the payoff — divergences —
  shows up; also the panel `0057`'s and `0058`'s error-surfacing work make
  more load-bearing).
- **Secondary (grouped into one collapsed-by-default tabbed group,
  "Rules & Preprocessing"):** `PipelineEditorPanel`, `TemplateRulePanel`,
  `OcrRulePanel` — all three are rule-authoring panels a user reaches for
  only after they already have a session and annotations, and grouping them
  together (rather than three more independent stacked panels) also
  directly supports `0060`'s consistency fix (matching Run affordances)
  since they become siblings in one visual group instead of three
  independent docks.

This is a real structural/docking change, so it is sketched in a new
diagram: `plan/VisualStateModel/diagrams/workbench_components_proposed.puml`.
Task `0065`.

---

## 5. Wording call (gap #6)

**Decision: not a defect — the current wording is correct for this tool's
actual audience.** No replacement wording is proposed.

Reasoning: `ai-upp_handoff_2026-07-02/DESIGN_PRINCIPLES.md` — the project's
own stated architecture reference — is written entirely in the same register
as the flagged labels: "Region Memory," "Fine Rule Layer," "Application
Model Runtime," and explicitly calls out "Divergence" as a first-class,
named concept ("the worst category because it means the system may proceed
confidently in the wrong world"). `VisualStateWorkbench` is the debugging/
authoring tool for people building and maintaining `VisualStateModel`
pipelines — the same audience that reads and writes `DESIGN_PRINCIPLES.md`,
not a downstream consumer of a finished product. Someone who does not
already know what "ground truth," "pipeline," or "divergence" mean in this
codebase's vocabulary could not productively use `TemplateRulePanel` or
`OcrRulePanel` regardless of what the menu items are called — the tool
requires that vocabulary to operate at all. Relabeling "Compare with Ground
Truth…" or "Clear Pipeline Cache" to something more generic would not make
the tool usable to a broader audience; it would only make it less precise
for the audience it actually has. Closed with no task.

---

## 6. Follow-on task list (`0057`–`0065`)

Execution order matches numeric order. `0057` is first per the requirement
that the session-identity fix precede cosmetic/discoverability work.

1. **`0057`** — Unify session identity; fix BUG-A/BUG-B (toolbar Step/Run
   All/Reset and region click/list-select operate on whichever session is
   actually active; Reset requires confirmation before discarding an
   opened/imported session).
2. **`0058`** — Surface session load/import/pipeline-run failures to the
   user visibly (not log-only), for the failure branches in
   workflow_catalogue.md rows 1.1/1.3/1.4/1.5/1.6.
3. **`0059`** — Fix three small correctness bugs: rule ID collision
   (6.1/7.1), hardcoded cache-cleared count (1.7), wrong overlay flag in
   toggle log (3.5–3.8).
4. **`0060`** — Wire `OcrRulePanel`/`PipelineEditorPanel`/`TemplateRulePanel`
   "Run" actions to the real active session's current frame instead of
   self-seeded/synthetic data; add the missing Run button to
   `TemplateRulePanel`.
5. **`0061`** — `AnnotationEditorPanel` polish: "—" blank-state convention,
   safe no-op Apply, visible autosave indicator.
6. **`0062`** — CLI argument convention rollout (`--flag`s + positional
   path(s), §3) across the 11 in-scope reference CLI tools.
7. **`0063`** — Group session-loading entry points into one
   "Open/Import Session…" dialog with a source-type chooser.
8. **`0064`** — First-run empty-state guidance in the Frame tab.
9. **`0065`** — LEFT dock panel hierarchy: primary
   (`AnnotationEditorPanel`, `ModelStatePanel`) vs. secondary, collapsed
   "Rules & Preprocessing" group (`PipelineEditorPanel`, `TemplateRulePanel`,
   `OcrRulePanel`).

New diagrams produced alongside this plan:

- `plan/VisualStateModel/diagrams/user_journey_proposed.puml` — updated
  versions of both journeys in `user_journey_current.puml`, reflecting the
  unified session model, the grouped Open/Import dialog, first-run
  guidance, and the primary/secondary panel split.
- `plan/VisualStateModel/diagrams/workbench_components_proposed.puml` —
  updated component diagram for the LEFT dock regrouping (gap #7) and the
  single unified active-session component replacing the `replay_`/
  `src_source_` split.
