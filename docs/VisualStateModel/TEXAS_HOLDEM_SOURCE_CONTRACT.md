# TexasHoldem Source Contract

This document defines Milestone 01 for the TexasHoldem + VisualStateModel
source boundary.

## Scope

`game/TexasHoldem` is the deterministic source. VisualStateModel consumes
rendered frames and synchronized ground truth; it must not reverse-engineer
TexasHoldem internals when a controlled source is available.

The first provider/layout identity is `PS_6p`.

## Session Layout

A minimal source-contract session is a directory:

```text
session/
  metadata.json
  groundtruth.jsonl
  frames/
    00000000.png
    00000001.png
    ...
```

`metadata.json` is a session-level object:

```json
{
  "schema": 1,
  "kind": "texas_holdem_source_contract_sample",
  "session_id": "texas-m01-ps6p-sample",
  "provider": "PS_6p",
  "table_width": 1024,
  "table_height": 648,
  "seed": 1,
  "frame_count": 8,
  "frame_format": "png",
  "frame_pattern": "frames/00000000.png",
  "ground_truth": "groundtruth.jsonl"
}
```

## Frame Identity

Every frame and every ground-truth record share:

- `session_id`: stable session identity.
- `frame_id`: zero-based visual frame index.
- `render_step`: deterministic source step that produced the frame.
- `timestamp_ms`: synthetic timeline timestamp. The first M01 sample uses `0`
  for the first frame and 100 ms steps after that.
- `provider`: provider/layout identity, starting with `PS_6p`.
- `table_width` and `table_height`: rendered table size.
- `seed`: deterministic game seed.

For recorded runs, `frame_id` must be monotonic and unique. `render_step` may
match `frame_id` for one-frame-per-step recordings. Duplicate or dropped frames
must still keep unique `frame_id` values and must not reuse ground-truth rows.

## Ground Truth JSONL

`groundtruth.jsonl` contains one JSON object per frame. The minimal M01 record
shape is:

```json
{
  "schema": 1,
  "session_id": "texas-m01-ps6p-sample",
  "frame_id": 0,
  "render_step": 0,
  "timestamp_ms": 0,
  "provider": "PS_6p",
  "table_width": 1024,
  "table_height": 648,
  "seed": 1,
  "game_id": 1,
  "hand_id": 1,
  "street": 0,
  "turn_uid": 4,
  "pot": 0,
  "board_cards": [0, 0, 0, 0, 0],
  "players": [
    {
      "seat": 0,
      "uid": 0,
      "name": "Player",
      "hero": true,
      "active": true,
      "stack": 2000,
      "bet": 0,
      "action": 0,
      "button": 0,
      "hole_cards": [12, 34]
    }
  ]
}
```

Card IDs are TexasHoldem engine card integers. Later parser/model tasks may add
human-readable card labels, but M01 preserves the engine-native value to avoid
lossy conversion.

## CLI Proof

The minimal proof command is:

```sh
bin\TexasHoldem.exe --dump-source-contract-sample --provider PS_6p --frames 8 --out tmp\texas_m01_sample --fastcrash
```

It writes `metadata.json`, `groundtruth.jsonl`, and `frames/00000000.png`
through `frames/00000007.png`. The stdout summary prints the paths and the
shared frame identity range.

Validate the sample with:

```sh
bin\TexasHoldem.exe --validate-source-contract-sample tmp\texas_m01_sample --fastcrash
```

The validator checks metadata/ground-truth identity, monotonic frame IDs, row
count, and frame-file presence. Milestone 02 extends the same contract to
record/replay CLI tooling with real frame progression.

## M02 Record and Replay

Milestone 02 keeps the same session contract and adds agent-friendly command
names for the record/replay loop:

```sh
bin\TexasHoldem.exe --record-session --provider PS_6p --frames 8 --out tmp\texas_m02_session --seed 1 --fastcrash
bin\TexasHoldem.exe --replay-session tmp\texas_m02_session --expect-provider PS_6p --expect-size 1024x648 --expect-frame-ms 100 --fastcrash
```

`--record-session` writes the same `metadata.json`, `groundtruth.jsonl`, and
`frames/%08d.png` layout as the M01 proof command. `--replay-session` validates
the session and prints a deterministic per-frame summary containing frame
identity, render step, timestamp, game/hand identifiers, street, pot, player
count, and image path. Optional replay expectations fail with concise `ERROR:`
diagnostics for provider mismatch, table-size mismatch, backwards timestamps,
or timestamp deltas that differ from `--expect-frame-ms`.

The reusable contract implementation lives in
`game/TexasHoldem/TexasHoldemSessionContract.*`. CLI code should route through
that helper rather than duplicating metadata parsing, ground-truth validation,
frame naming, or replay diagnostics.

For real per-frame state progression, add `--step-actions` to the record
command:

```sh
bin\TexasHoldem.exe --record-session --provider PS_6p --frames 5 --step-actions --out tmp\texas_m02_steps --seed 1 --fastcrash
```

The first stepped mode advances one shared game/model operation before each
frame after frame 0 and prints `record_step=... action=next_player ...`
diagnostics. Ground truth is captured after the step from the live game state.

## Compatibility Rules

- Readers must reject mismatched `session_id`, `provider`, or table size between
  `metadata.json` and `groundtruth.jsonl`.
- Readers must reject missing frame files for ground-truth rows.
- Unknown fields are allowed for forward compatibility.
- Required M01 fields must remain present until a schema version bump.
- `PS_6p` means the `GameTable_PS_6p.form` provider and `ps-6p` layout profile.

## M03 Persistent Fixture Session

Starting with Milestone 03, a canonical sample session is maintained in a
gitignored, persistent directory: `var/vsm_fixtures/texas_ps6p_sample/`

This fixture is regenerable on demand (never committed to the repo) and serves
as a stable reference for regression testing the changed-region pipeline and
later analysis milestones.

**Regeneration Command:**

```sh
bin\TexasHoldem.exe --record-session --provider PS_6p --frames 8 --step-actions --out var\vsm_fixtures\texas_ps6p_sample --seed 1 --fastcrash
```

**Details:**
- **Provider:** `PS_6p` (fixed)
- **Seed:** `1` (fixed for reproducibility)
- **Frames:** `8` (fixed frame count)
- **Mode:** `--step-actions` (captures state progression with one action per frame after frame 0)
- **Output:** `var/vsm_fixtures/texas_ps6p_sample/` containing:
  - `metadata.json` (session-level metadata)
  - `groundtruth.jsonl` (per-frame ground truth, one JSON object per line)
  - `frames/00000000.png` through `frames/00000007.png` (8 rendered table states)

The directory is gitignored (never committed) and is always deleted before
regeneration to ensure fresh output. The deterministic seed guarantees
byte-identical output on every run with these exact parameters.

## M03 Changed-Region CLI

`reference/VisualStateRegionDump` runs the existing `VsmDetectChanges` +
`VsmRegionMemory` pipeline (`uppsrc/VisualStateModel`) directly over an
M01/M02 session directory, decoding frames via the PNG bridge
(`VsmLoadM01M02SessionFrame`, `uppsrc/VisualStateModel/PngFrame.h`):

```sh
bin\VisualStateRegionDump.exe var\vsm_fixtures\texas_ps6p_sample
bin\VisualStateRegionDump.exe var\vsm_fixtures\texas_ps6p_sample --frame-start 1 --frame-end 1 --jsonl-out out.jsonl
```

It emits one deterministic JSON record per changed region per frame
transition (`frame_prev`, `frame`, `x`, `y`, `w`, `h`, `score`, `region_id`),
either to stdout or to a file via `--jsonl-out`, and supports `--frame-start`/
`--frame-end` for focused reruns on a frame range. This mode supersedes the
tool's older real-session path (which read the incompatible OLD
`VsmSessionStoreSource`/`.vsm` format); the tool's synthetic-session path
(no session directory argument) is unchanged and still serves as the
`ChangeDetect`/`RegionMemory` reuse smoke test. See
`reference/VisualStateRegionDump/README.md` for full usage.

Note: for the persistent `texas_ps6p_sample` fixture, ground truth
(`groundtruth.jsonl`) is only actually distinct for frames 0, 1, and 2 —
`turn_uid` and player `action` fields are byte-identical from frame 2 through
frame 7 (the `--step-actions` stepped recorder stalls after two steps for
this seed/provider). The changed-region CLI faithfully reports zero regions
for those later transitions; this is a property of the current recorder, not
a defect in the region-detection pipeline, and is a candidate for a future
`game/TexasHoldem` investigation.

## Future Work

- Store layout identity and form file version/hash when provider artifacts
  stabilize.
- Investigate why `--step-actions` recording stops advancing `turn_uid`/player
  `action` state after frame 2 for the `PS_6p` seed=1 fixture (see "M03
  Changed-Region CLI" above) — affects the number of real transitions
  available for changed-region regression testing.
