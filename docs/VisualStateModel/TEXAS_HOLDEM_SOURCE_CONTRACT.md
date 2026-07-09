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

## Compatibility Rules

- Readers must reject mismatched `session_id`, `provider`, or table size between
  `metadata.json` and `groundtruth.jsonl`.
- Readers must reject missing frame files for ground-truth rows.
- Unknown fields are allowed for forward compatibility.
- Required M01 fields must remain present until a schema version bump.
- `PS_6p` means the `GameTable_PS_6p.form` provider and `ps-6p` layout profile.

## Future Work

- Store layout identity and form file version/hash when provider artifacts
  stabilize.
